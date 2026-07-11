#!/usr/bin/env python3
"""Check that a release source ZIP has the expected public layout."""

from __future__ import annotations

import sys
import zipfile
from pathlib import Path, PurePosixPath

REQUIRED = {
    "README.md",
    "LICENSE",
    "CHANGELOG.md",
    "library.properties",
    "library.json",
    "CITATION.cff",
    "src/PicoMesh.h",
    "include/picomesh/frame.h",
    "examples/ArduinoHeartbeatController/ArduinoHeartbeatController.ino",
    "examples/PicoSdkI2cNode/CMakeLists.txt",
}
FORBIDDEN_PARTS = {".git", "build", "build-pico", "__pycache__"}
FORBIDDEN_SUFFIXES = {".elf", ".uf2", ".bin", ".hex", ".pyc"}


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: check_source_archive.py ARCHIVE.zip", file=sys.stderr)
        return 2

    archive = Path(sys.argv[1])
    if not archive.is_file():
        print(f"archive not found: {archive}", file=sys.stderr)
        return 2

    with zipfile.ZipFile(archive) as package:
        files = [PurePosixPath(name) for name in package.namelist() if not name.endswith("/")]

    if not files:
        print("archive contains no files", file=sys.stderr)
        return 1

    roots = {path.parts[0] for path in files if path.parts}
    if roots != {"PicoMesh"}:
        print(f"archive must contain one PicoMesh/ root, found: {sorted(roots)}", file=sys.stderr)
        return 1

    relative = {PurePosixPath(*path.parts[1:]).as_posix() for path in files}
    missing = sorted(REQUIRED - relative)
    if missing:
        print("archive is missing required files:", file=sys.stderr)
        for path in missing:
            print(f"  {path}", file=sys.stderr)
        return 1

    forbidden: list[str] = []
    for path in files:
        relative_path = PurePosixPath(*path.parts[1:])
        if any(part in FORBIDDEN_PARTS or part.startswith("build-") for part in relative_path.parts):
            forbidden.append(relative_path.as_posix())
        elif relative_path.suffix.lower() in FORBIDDEN_SUFFIXES:
            forbidden.append(relative_path.as_posix())

    if forbidden:
        print("archive contains generated or forbidden files:", file=sys.stderr)
        for path in sorted(forbidden):
            print(f"  {path}", file=sys.stderr)
        return 1

    print(f"Archive layout is valid: {archive} ({len(files)} files)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
