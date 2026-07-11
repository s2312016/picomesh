# Testing and coverage

PicoMesh separates portable logic tests, compile-only platform checks, security analysis, and physical validation. Each layer answers a different question.

## Host tests

The host test executable covers the fixed-capacity protocol core:

- checksum generation and rejection
- frame encode/decode and malformed input
- sequence ordering and wraparound
- node liveness and timer wraparound
- ACK parsing and bounded retries
- stream decoding and recovery from noise

Run locally with:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The CI matrix repeats this on Linux, Windows, and macOS. Sanitizer jobs add AddressSanitizer and UndefinedBehaviorSanitizer where supported.

## Coverage reports

The `Coverage` workflow creates a GCC-instrumented Debug build, runs CTest, and generates XML plus browsable HTML reports with `gcovr`. Download the `picomesh-coverage` artifact from the workflow run and open `index.html`.

Coverage is currently diagnostic rather than a release gate. A numeric threshold should only be introduced after the suite covers intentional error paths and the threshold is documented. Coverage percentage must not replace review of boundary conditions, timer wraparound, fixed-capacity behavior, or compatibility-sensitive branches.

## Platform compile checks

Arduino and Pico SDK workflows prove that supported examples compile with the pinned cores and SDK versions. Compile success does not prove electrical safety or runtime interoperability.

## Static and security analysis

- `cppcheck` blocks warning, performance, and portability findings in the portable core.
- style and formatting checks are being rolled out separately from behavioral changes.
- CodeQL provides an additional security-oriented analysis pass.

## Physical validation

Real boards must be tested with documented models, wiring, voltages, toolchain versions, and serial evidence. Use [`hardware_validation.md`](hardware_validation.md) and the versioned report under `docs/validation/`.

## Adding a regression test

A bug fix should first include a test that fails for the reported behavior and passes after the fix. Keep tests deterministic: do not depend on wall-clock time, random seeds, network services, or attached devices in the host suite.
