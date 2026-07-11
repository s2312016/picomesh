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
- Add tests for every protocol change.
- Update `docs/protocol.md` when the wire format changes.
- Treat protocol version changes as compatibility-sensitive.

## Validation

```sh
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
