#!/usr/bin/env python3
"""Summarize sanitized PicoMesh Arduino serial logs without certifying hardware."""

from __future__ import annotations

import argparse
import json
import re
import sys
from collections import Counter, defaultdict
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Iterable

HEARTBEAT_RE = re.compile(
    r"node=(?P<node>\d+)\s+sequence=(?P<sequence>\d+)\s+type=0x(?P<type>[0-9A-Fa-f]+)"
)
ONLINE_RE = re.compile(r"online nodes=(?P<count>\d+)")
INVALID_RE = re.compile(r"No valid frame, bytes=(?P<bytes>\d+)")
QUEUED_RE = re.compile(r"queued command sequence=(?P<sequence>\d+)")
SEND_RE = re.compile(
    r"send sequence=(?P<sequence>\d+)\s+attempt=(?P<attempt>\d+)\s+result=(?P<result>\S+)"
)
ACK_RE = re.compile(
    r"ack sequence=(?P<sequence>\d+)\s+status=(?P<status>\d+)\s+matched=(?P<matched>yes|no)"
)
EXHAUSTED_RE = re.compile(r"retry exhausted sequence=(?P<sequence>\d+)")


@dataclass
class NodeSummary:
    frames: int = 0
    first_sequence: int | None = None
    last_sequence: int | None = None
    duplicates: int = 0
    forward_steps: int = 0
    wraparounds: int = 0
    stale_or_backward: int = 0
    message_types: Counter[int] = field(default_factory=Counter)

    def observe(self, sequence: int, message_type: int) -> None:
        sequence &= 0xFF
        self.frames += 1
        self.message_types[message_type] += 1
        if self.first_sequence is None:
            self.first_sequence = sequence
            self.last_sequence = sequence
            return

        assert self.last_sequence is not None
        distance = (sequence - self.last_sequence) & 0xFF
        if distance == 0:
            self.duplicates += 1
        elif distance < 128:
            self.forward_steps += 1
            if sequence < self.last_sequence:
                self.wraparounds += 1
            self.last_sequence = sequence
        else:
            self.stale_or_backward += 1

    def to_dict(self) -> dict[str, object]:
        result = asdict(self)
        result["message_types"] = {
            f"0x{message_type:02X}": count
            for message_type, count in sorted(self.message_types.items())
        }
        return result


@dataclass
class LogSummary:
    total_lines: int = 0
    recognized_lines: int = 0
    nodes: dict[int, NodeSummary] = field(default_factory=dict)
    online_samples: list[int] = field(default_factory=list)
    online_transitions: list[tuple[int, int]] = field(default_factory=list)
    invalid_frame_reads: int = 0
    invalid_frame_bytes: Counter[int] = field(default_factory=Counter)
    queued_commands: Counter[int] = field(default_factory=Counter)
    send_attempts: dict[int, list[dict[str, object]]] = field(
        default_factory=lambda: defaultdict(list)
    )
    acknowledgements: list[dict[str, object]] = field(default_factory=list)
    retry_exhaustions: Counter[int] = field(default_factory=Counter)

    def observe_online_count(self, count: int) -> None:
        if self.online_samples and self.online_samples[-1] != count:
            self.online_transitions.append((self.online_samples[-1], count))
        self.online_samples.append(count)

    def to_dict(self) -> dict[str, object]:
        return {
            "total_lines": self.total_lines,
            "recognized_lines": self.recognized_lines,
            "nodes": {
                str(node_id): summary.to_dict()
                for node_id, summary in sorted(self.nodes.items())
            },
            "online_samples": self.online_samples,
            "online_transitions": [list(item) for item in self.online_transitions],
            "invalid_frame_reads": self.invalid_frame_reads,
            "invalid_frame_bytes": {
                str(byte_count): count
                for byte_count, count in sorted(self.invalid_frame_bytes.items())
            },
            "queued_commands": {
                str(sequence): count
                for sequence, count in sorted(self.queued_commands.items())
            },
            "send_attempts": {
                str(sequence): attempts
                for sequence, attempts in sorted(self.send_attempts.items())
            },
            "acknowledgements": self.acknowledgements,
            "retry_exhaustions": {
                str(sequence): count
                for sequence, count in sorted(self.retry_exhaustions.items())
            },
        }


def parse_lines(lines: Iterable[str]) -> LogSummary:
    summary = LogSummary()

    for raw_line in lines:
        summary.total_lines += 1
        line = raw_line.strip()

        match = HEARTBEAT_RE.search(line)
        if match:
            node_id = int(match.group("node"))
            sequence = int(match.group("sequence"))
            message_type = int(match.group("type"), 16)
            summary.nodes.setdefault(node_id, NodeSummary()).observe(
                sequence, message_type
            )
            summary.recognized_lines += 1
            continue

        match = ONLINE_RE.search(line)
        if match:
            summary.observe_online_count(int(match.group("count")))
            summary.recognized_lines += 1
            continue

        match = INVALID_RE.search(line)
        if match:
            byte_count = int(match.group("bytes"))
            summary.invalid_frame_reads += 1
            summary.invalid_frame_bytes[byte_count] += 1
            summary.recognized_lines += 1
            continue

        match = QUEUED_RE.search(line)
        if match:
            summary.queued_commands[int(match.group("sequence"))] += 1
            summary.recognized_lines += 1
            continue

        match = SEND_RE.search(line)
        if match:
            sequence = int(match.group("sequence"))
            summary.send_attempts[sequence].append(
                {
                    "attempt": int(match.group("attempt")),
                    "result": match.group("result"),
                }
            )
            summary.recognized_lines += 1
            continue

        match = ACK_RE.search(line)
        if match:
            summary.acknowledgements.append(
                {
                    "sequence": int(match.group("sequence")),
                    "status": int(match.group("status")),
                    "matched": match.group("matched") == "yes",
                }
            )
            summary.recognized_lines += 1
            continue

        match = EXHAUSTED_RE.search(line)
        if match:
            summary.retry_exhaustions[int(match.group("sequence"))] += 1
            summary.recognized_lines += 1

    return summary


def render_text(summary: LogSummary) -> str:
    lines = [
        "PicoMesh serial log summary",
        f"lines: {summary.total_lines}",
        f"recognized: {summary.recognized_lines}",
    ]

    if summary.nodes:
        lines.append("nodes:")
        for node_id, node in sorted(summary.nodes.items()):
            types = ", ".join(
                f"0x{message_type:02X}={count}"
                for message_type, count in sorted(node.message_types.items())
            )
            lines.append(
                "  "
                f"node {node_id}: frames={node.frames}, "
                f"first={node.first_sequence}, last={node.last_sequence}, "
                f"duplicates={node.duplicates}, forward={node.forward_steps}, "
                f"wraparounds={node.wraparounds}, "
                f"stale_or_backward={node.stale_or_backward}, types=[{types}]"
            )

    if summary.online_samples:
        transitions = ", ".join(
            f"{before}->{after}" for before, after in summary.online_transitions
        )
        lines.append(
            f"online samples: {len(summary.online_samples)}; "
            f"min={min(summary.online_samples)}; max={max(summary.online_samples)}"
        )
        lines.append(f"online transitions: {transitions or 'none'}")

    lines.append(f"invalid frame reads: {summary.invalid_frame_reads}")
    lines.append(
        f"queued command events: {sum(summary.queued_commands.values())}"
    )
    lines.append(
        f"send attempts: {sum(len(items) for items in summary.send_attempts.values())}"
    )
    lines.append(f"acknowledgements: {len(summary.acknowledgements)}")
    lines.append(
        f"retry exhaustion events: {sum(summary.retry_exhaustions.values())}"
    )
    lines.append(
        "This summary describes recognized log events; it does not certify physical validation."
    )
    return "\n".join(lines)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Summarize sanitized PicoMesh Arduino serial output."
    )
    parser.add_argument("log", type=Path, help="UTF-8 serial log file")
    parser.add_argument(
        "--json",
        action="store_true",
        help="emit machine-readable JSON instead of text",
    )
    return parser


def main() -> int:
    args = build_parser().parse_args()
    try:
        lines = args.log.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError as error:
        print(f"unable to read {args.log}: {error}", file=sys.stderr)
        return 2

    summary = parse_lines(lines)
    if summary.recognized_lines == 0:
        print("no PicoMesh serial events were recognized", file=sys.stderr)
        return 1

    if args.json:
        print(json.dumps(summary.to_dict(), indent=2, sort_keys=True))
    else:
        print(render_text(summary))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
