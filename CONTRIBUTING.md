# Contributing

Thank you for helping build PicoMesh.

## Before opening a pull request

1. Open an issue before substantial API, protocol, or compatibility changes.
2. Keep board-specific code under `ports/`.
3. Keep private datasets, models, board-production assets, and unpublished algorithms out of this repository.
4. Add or update host tests for protocol, reliability, decoder, and registry changes.
5. Update `docs/protocol.md` whenever the wire format or protocol semantics change.
6. Describe any hardware used for validation, including board model, SDK/core version, bus speed, and wiring assumptions.

## Local validation

Run the portable build and tests:

```sh
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Build the API documentation when Doxygen is installed:

```sh
doxygen Doxyfile
```

Format modified C and C++ files with the repository `.clang-format` policy:

```sh
clang-format -i path/to/changed_file.cpp path/to/changed_file.h
```

Do not commit generated build trees, firmware binaries, Doxygen output, credentials, or raw serial logs containing private data.

## Pull-request scope

Use focused commits and keep unrelated refactors separate from behavior changes. Pull requests should explain:

- the problem and proposed behavior;
- compatibility or wire-format impact;
- tests added or updated;
- hardware evidence, when applicable;
- remaining limitations.

By contributing, you agree that your contribution is licensed under Apache-2.0.
