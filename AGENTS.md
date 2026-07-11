# AGENTS.md

## Project purpose

PicoMesh is a reusable embedded distributed-systems framework built as an independent public project.

## Mandatory boundaries

- Do not copy private datasets, trained model weights, inference implementations, learned voting parameters, unpublished fault-tolerance algorithms, or board-production files.
- Generalize reusable communication and node-management concepts instead of preserving private names or fixed topology assumptions.
- Do not vendor FreeRTOS or other large third-party source trees. Use documented external dependencies.
- Never commit credentials, serial captures containing personal data, generated build outputs, or private research results.

## Engineering rules

- Keep the portable core in `include/picomesh` and `src`.
- Keep platform code in `ports`.
- Maintain C++17 compatibility.
- Avoid exceptions and heap dependence in parsing and liveness-critical paths.
- Preserve bounded storage and deterministic failure behavior.
- Add tests for every protocol, parser, retry, sequence, or liveness change.
- Update `docs/protocol.md` when the wire format or protocol semantics change.
- Treat protocol version changes as compatibility-sensitive.
- Format modified C and C++ files with the repository `.clang-format` policy.
- Add API documentation for new public types, constants, and functions.

## Change discipline

- Keep behavior changes separate from mechanical refactors.
- Do not silently change frame sizes, timeout semantics, retry counts, or node identity rules.
- Include a compatibility note in pull requests that modify public APIs or transport behavior.
- Use synthetic examples and test fixtures; do not import private traces or application-specific identifiers.
- Never claim physical validation unless the exact hardware, toolchain, wiring, and evidence are recorded.

## Validation

```sh
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
doxygen Doxyfile
```

Hardware-dependent changes must also update the relevant validation report under `docs/validation/`.
