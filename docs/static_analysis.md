# Static analysis policy

PicoMesh uses automated analysis to catch portable C++ defects before merge.

## Blocking checks

`cppcheck` runs on the portable core and public headers with warning, style, performance, and portability diagnostics enabled. New findings fail the workflow and should be fixed or narrowly suppressed with an explanation close to the affected code.

## Formatting rollout

`clang-format` checks all tracked C, C++, header, and Arduino sketch files. During the initial rollout this job is report-only so that existing formatting can be normalized in a focused follow-up change without mixing style-only edits into behavioral work.

After the repository is normalized, remove `continue-on-error` from the formatting job. From that point onward, contributors should run:

```sh
clang-format -i $(git ls-files '*.cpp' '*.h' '*.hpp' '*.ino')
```

before opening a pull request.

## Scope and exceptions

- Generated files and vendored dependencies must not be analyzed as project code.
- Suppressions must be specific; repository-wide suppression of actionable diagnostics is discouraged.
- Static analysis does not replace unit tests, sanitizer runs, protocol compatibility review, or physical hardware validation.
