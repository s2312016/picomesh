import json
import unittest

from scripts.summarize_serial_log import parse_lines, render_text


class SerialLogSummaryTests(unittest.TestCase):
    def test_tracks_sequence_progress_duplicates_wrap_and_stale_values(self) -> None:
        summary = parse_lines(
            [
                "node=0 sequence=254 type=0x1",
                "node=0 sequence=255 type=0x1",
                "node=0 sequence=0 type=0x1",
                "node=0 sequence=0 type=0x1",
                "node=0 sequence=250 type=0x1",
            ]
        )

        node = summary.nodes[0]
        self.assertEqual(node.frames, 5)
        self.assertEqual(node.first_sequence, 254)
        self.assertEqual(node.last_sequence, 0)
        self.assertEqual(node.forward_steps, 2)
        self.assertEqual(node.wraparounds, 1)
        self.assertEqual(node.duplicates, 1)
        self.assertEqual(node.stale_or_backward, 1)
        self.assertEqual(node.message_types[0x01], 5)

    def test_tracks_timeout_and_recovery_transitions(self) -> None:
        summary = parse_lines(
            [
                "online nodes=1",
                "online nodes=1",
                "No valid frame, bytes=0",
                "online nodes=0",
                "online nodes=0",
                "online nodes=1",
            ]
        )

        self.assertEqual(summary.online_samples, [1, 1, 0, 0, 1])
        self.assertEqual(summary.online_transitions, [(1, 0), (0, 1)])
        self.assertEqual(summary.invalid_frame_reads, 1)
        self.assertEqual(summary.invalid_frame_bytes[0], 1)

    def test_tracks_reliable_command_events(self) -> None:
        summary = parse_lines(
            [
                "queued command sequence=3",
                "send sequence=3 attempt=1 result=ok",
                "send sequence=3 attempt=2 result=ok",
                "ack sequence=3 status=0 matched=yes",
                "queued command sequence=4",
                "retry exhausted sequence=4",
            ]
        )

        self.assertEqual(summary.queued_commands[3], 1)
        self.assertEqual(summary.queued_commands[4], 1)
        self.assertEqual(
            summary.send_attempts[3],
            [
                {"attempt": 1, "result": "ok"},
                {"attempt": 2, "result": "ok"},
            ],
        )
        self.assertEqual(
            summary.acknowledgements,
            [{"sequence": 3, "status": 0, "matched": True}],
        )
        self.assertEqual(summary.retry_exhaustions[4], 1)

    def test_ignores_unrelated_lines_and_serializes_to_json(self) -> None:
        summary = parse_lines(
            [
                "PicoMesh Arduino heartbeat controller ready",
                "unrelated text",
                "node=2 sequence=7 type=0x10",
            ]
        )

        self.assertEqual(summary.total_lines, 3)
        self.assertEqual(summary.recognized_lines, 1)
        encoded = json.dumps(summary.to_dict(), sort_keys=True)
        self.assertIn('"2"', encoded)
        self.assertIn('"0x10"', encoded)

    def test_text_output_contains_evidence_boundary(self) -> None:
        summary = parse_lines(["online nodes=1"])
        output = render_text(summary)
        self.assertIn("online samples: 1", output)
        self.assertIn("does not certify physical validation", output)


if __name__ == "__main__":
    unittest.main()
