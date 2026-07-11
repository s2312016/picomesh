# Static analysis policy

PicoMesh uses automated analysis to catch portable C++ defects and inconsistent source formatting before merge.

## Blocking checks

`cppcheck` runs on the portable core and public headers with warning, style, performance, and portability diagnostics enabled. New findings fail the workflow and should be fixed or narrowly suppressed with an explanation close to the affected code.

`unusedFunction` is suppressed because PicoMesh is a library and public entry points are intentionally not called from the library itself.

`clang-format --dry-run --Werror` checks every tracked C++, header, and Arduino sketch against the repository `.clang-format` file. Formatting differences fail the workflow.

Contributors should run:

```sh
cppcheck --enable=warning,style,performance,portability --std=c++17 -I include src include
clang-format -i $(git ls-files '*.cpp' '*.h' '*.hpp' '*.ino')
```

before opening a pull request.

## Scope and exceptions

- Generated files and vendored dependencies must not be analyzed as project code.
- Suppressions must be specific; repository-wide suppression of actionable diagnostics is discouraged.
- Style-only changes should not be mixed with protocol or behavior changes when that would obscure review.
- Static analysis does not replace unit tests, sanitizer runs, fuzzing, protocol compatibility review, or physical hardware validation.
