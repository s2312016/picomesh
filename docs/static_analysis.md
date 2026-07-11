# Static analysis policy

PicoMesh uses automated analysis to catch portable C++ defects and inconsistent source formatting before merge.

## Blocking checks

`cppcheck` runs on the portable core and public headers with warning, style, performance, and portability diagnostics enabled. New findings fail the workflow and should be fixed or narrowly suppressed with an explanation close to the affected code.

`unusedFunction` is suppressed because PicoMesh is a library and public entry points are intentionally not called from the library itself.

`useStlAlgorithm` is suppressed because the fixed-capacity embedded paths intentionally retain explicit loops. The loops make slot activation, early returns, mutation, and interrupt-sensitive control flow visible without introducing iterator or lambda abstractions. Warning, performance, portability, and all other style diagnostics remain blocking.

`clang-format` rewrites every tracked C++, header, and Arduino sketch using the repository `.clang-format` file and fails when the resulting patch is non-empty. The patch is uploaded as a workflow artifact so contributors can apply the exact required formatting.

Contributors should run:

```sh
cppcheck --enable=warning,style,performance,portability \
  --suppress=unusedFunction --suppress=useStlAlgorithm \
  --std=c++17 -I include src include
clang-format -i $(git ls-files '*.cpp' '*.h' '*.hpp' '*.ino')
```

before opening a pull request.

## Scope and exceptions

- Generated files and vendored dependencies must not be analyzed as project code.
- Suppressions must be specific and documented; repository-wide suppression of actionable diagnostics is discouraged.
- Style-only changes should not be mixed with protocol or behavior changes when that would obscure review.
- Static analysis does not replace unit tests, sanitizer runs, fuzzing, protocol compatibility review, or physical hardware validation.
