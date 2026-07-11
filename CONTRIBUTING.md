# Contributing

Thank you for helping build PicoMesh.

1. Open an issue for substantial API or protocol changes.
2. Keep board-specific code under `ports/`.
3. Keep research-specific models, datasets, and unpublished algorithms out of this repository.
4. Add or update host tests for protocol and registry changes.
5. Run:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Use focused commits and describe tested hardware in the pull request.

By contributing, you agree that your contribution is licensed under Apache-2.0.
