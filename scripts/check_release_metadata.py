#!/usr/bin/env python3
"""Validate that public release metadata uses one semantic version."""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SEMVER = re.compile(r"^[0-9]+\.[0-9]+\.[0-9]+$")


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def capture(path: str, pattern: str) -> str:
    match = re.search(pattern, read(path), flags=re.MULTILINE)
    if match is None:
        raise ValueError(f"Unable to read version from {path}")
    return match.group(1)


def main() -> int:
    try:
        versions = {
            "CMakeLists.txt": capture(
                "CMakeLists.txt",
                r"project\(picomesh VERSION ([0-9]+\.[0-9]+\.[0-9]+)",
            ),
            "library.properties": capture(
                "library.properties",
                r"^version=([^\s]+)$",
            ),
            "library.json": json.loads(read("library.json"))["version"],
            "CITATION.cff": capture(
                "CITATION.cff",
                r'^version:\s*["\']?([^"\'\s]+)',
            ),
            "include/picomesh/version.h": capture(
                "include/picomesh/version.h",
                r'^#define PICOMESH_VERSION_STRING "([^"]+)"$',
            ),
        }
    except (OSError, KeyError, ValueError, json.JSONDecodeError) as error:
        print(f"release metadata error: {error}", file=sys.stderr)
        return 1

    expected = versions["CMakeLists.txt"]
    if not SEMVER.fullmatch(expected):
        print(f"CMake project version is not MAJOR.MINOR.PATCH: {expected}", file=sys.stderr)
        return 1

    mismatches = {path: version for path, version in versions.items() if version != expected}
    if mismatches:
        print(f"Expected version {expected} in every metadata file", file=sys.stderr)
        for path, version in mismatches.items():
            print(f"  {path}: {version}", file=sys.stderr)
        return 1

    version_header = read("include/picomesh/version.h")
    major, minor, patch = expected.split(".")
    required_macros = {
        "PICOMESH_VERSION_MAJOR": major,
        "PICOMESH_VERSION_MINOR": minor,
        "PICOMESH_VERSION_PATCH": patch,
    }
    for macro, value in required_macros.items():
        pattern = rf"^#define {macro} {re.escape(value)}$"
        if re.search(pattern, version_header, flags=re.MULTILINE) is None:
            print(f"{macro} does not match {expected}", file=sys.stderr)
            return 1

    if f"## [{expected}]" not in read("CHANGELOG.md"):
        print(f"CHANGELOG.md has no section for [{expected}]", file=sys.stderr)
        return 1

    for path, version in versions.items():
        print(f"{path}: {version}")
    print(f"Release metadata is consistent for {expected}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
