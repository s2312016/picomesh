# Distribution and package readiness

PicoMesh includes metadata for CMake, Arduino, and PlatformIO, but registry publication and a tagged release are separate maintainer actions. Do not describe a package as published until its registry entry or release URL is publicly available and verified.

## Automated release-readiness checks

The `Release readiness` workflow verifies that:

- version declarations match across CMake, Arduino, PlatformIO, citation metadata, and the public version header;
- numeric version macros match the semantic version string;
- the changelog contains a section for the release version;
- a clean `git archive` contains required public files;
- generated firmware, build directories, caches, and repository internals are absent from the source archive.

Run the checks locally:

```sh
python3 scripts/check_release_metadata.py
git archive --format=zip --prefix=PicoMesh/ --output=PicoMesh-source.zip HEAD
python3 scripts/check_source_archive.py PicoMesh-source.zip
```

## Arduino ZIP testing

Before registry submission:

1. Download the workflow-produced source candidate.
2. Install it through **Sketch > Include Library > Add .ZIP Library**.
3. Compile every Arduino example with the documented supported core.
4. Confirm the library name, version, sentence, URL, includes, and examples appear correctly.
5. Record the Arduino IDE or CLI and core versions used.

Arduino Library Manager publication should follow a tagged release and the registry's current submission process.

## PlatformIO testing

The repository contains `library.json`. Test the source candidate in a minimal project before registry publication:

```ini
[env:uno_r4_minima]
platform = renesas-ra
board = uno_r4_minima
framework = arduino
lib_deps = file://../picomesh
```

Compile an example that includes `PicoMesh.h`. Registry publication should only be claimed after the package is discoverable and installable by name and version.

## CMake package testing

CI installs PicoMesh to a temporary prefix and configures a separate consumer with `find_package(PicoMesh CONFIG REQUIRED)`. Maintainers should repeat the consumer test for every release candidate when CMake packaging changes.

## Tagging gate

The first public tag remains blocked until:

- all required CI workflows pass;
- the changelog date and release notes are finalized;
- the versioned hardware validation report contains the required physical evidence;
- known release-blocking failures are resolved or explicitly deferred in public issues;
- source and firmware artifacts are reproduced by the tag-triggered release workflow.

See [`RELEASING.md`](../RELEASING.md) and the active release-preparation issue for the authoritative checklist.
