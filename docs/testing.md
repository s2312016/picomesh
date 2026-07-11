# Testing and coverage

PicoMesh separates portable logic tests, compile-only platform checks, security analysis, fuzzing, and physical validation. Each layer answers a different question.

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

## Decoder fuzzing

The `Fuzz smoke` workflow builds `tests/fuzz_frame_decoder.cpp` with Clang libFuzzer, AddressSanitizer, and UndefinedBehaviorSanitizer. A fixed 25,000-run budget mutates valid and malformed seed inputs against:

- strict frame decoding;
- encoded-length detection;
- decode/encode/decode invariants;
- incremental stream decoding;
- bounded decoder storage and reset behavior.

Crashing inputs are uploaded as workflow artifacts. The fixed budget is a pull-request smoke test, not a claim of exhaustive fuzzing. Longer local or scheduled campaigns should reuse the same target and retain their corpus.

A local Clang invocation is:

```sh
clang++ -std=c++17 -O1 -g -fno-omit-frame-pointer \
  -fsanitize=fuzzer,address,undefined -Iinclude \
  tests/fuzz_frame_decoder.cpp src/checksum.cpp src/frame.cpp src/stream_decoder.cpp \
  -o fuzz-frame-decoder
./fuzz-frame-decoder -runs=25000 -max_len=128 corpus
```

## Platform compile checks

Arduino, PlatformIO, and Pico SDK workflows prove that supported examples compile with the tested cores, platform definitions, and SDK versions. Compile success does not prove electrical safety or runtime interoperability.

## Static and security analysis

- `cppcheck` blocks warning, performance, and portability findings in the portable core.
- style and formatting checks are being rolled out separately from behavioral changes.
- CodeQL provides an additional security-oriented analysis pass.
- libFuzzer exercises parser and stream-decoder invariants under sanitizers.

## Physical validation

Real boards must be tested with documented models, wiring, voltages, toolchain versions, and serial evidence. Use [`hardware_validation.md`](hardware_validation.md) and the versioned report under `docs/validation/`.

## Adding a regression test

A bug fix should first include a test that fails for the reported behavior and passes after the fix. Keep tests deterministic: do not depend on wall-clock time, random seeds, network services, or attached devices in the host suite.
