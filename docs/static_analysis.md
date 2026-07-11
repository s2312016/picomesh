# Static analysis policy

PicoMesh uses automated analysis to catch portable C++ defects before merge.

## Blocking checks

`cppcheck` runs on the portable core and public headers with warning, performance, and portability diagnostics enabled. New findings fail the workflow and should be fixed or narrowly suppressed with an explanation close to the affected code.

`unusedFunction` is suppressed because PicoMesh is a library and public entry points are intentionally not called from the library itself.

## Advisory checks

During the initial rollout, `cppcheck` style diagnostics and `clang-format` findings are reported without blocking a pull request. This keeps the first enforcement change focused while existing sources are normalized in separate style-only work.

After the repository is normalized, remove `continue-on-error` from both advisory steps. Contributors should then run:

```sh
cppcheck --enable=warning,style,performance,portability --std=c++17 -I include src include
clang-format -i $(git ls-files '*.cpp' '*.h' '*.hpp' '*.ino')
```

before opening a pull request.

## Scope and exceptions

- Generated files and vendored dependencies must not be analyzed as project code.
- Suppressions must be specific; repository-wide suppression of actionable diagnostics is discouraged.
- Static analysis does not replace unit tests, sanitizer runs, protocol compatibility review, or physical hardware validation.
