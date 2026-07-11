# Releasing PicoMesh

PicoMesh uses semantic versioning for published releases. Before 1.0, breaking API changes are permitted but must be called out clearly.

## Release checklist

1. Confirm all required CI workflows are green:
   - Host CI on Linux, Windows, and macOS
   - Warnings-as-errors build
   - Arduino UNO R4 example compilation
   - Raspberry Pi Pico and Pico 2 SDK builds
   - CodeQL analysis
2. Complete or update the hardware validation report for the release.
3. Review `docs/protocol.md` for wire-format accuracy.
4. Confirm version numbers match in:
   - `CMakeLists.txt`
   - `include/picomesh/version.h`
   - `library.properties`
   - `library.json`
   - `CITATION.cff`
5. Move relevant entries from `Unreleased` to a dated version in `CHANGELOG.md`.
6. Build and test from a clean checkout.
7. Create an annotated tag named `vX.Y.Z`.
8. Verify the release workflow publishes source, Arduino-library, and Pico firmware artifacts.
9. Check generated release notes for accuracy and remove unrelated internal details.
10. Announce the release and invite feedback through GitHub Discussions or Issues.

## Compatibility review

Any change to frame layout, checksum behavior, message-type allocation, flags, acknowledgement payloads, or sequence semantics requires an explicit compatibility review. A wire-incompatible change must not silently reuse the existing protocol version.

## Emergency security release

Security releases may use an abbreviated process, but must still include targeted tests, a changelog entry, fixed-version guidance, and coordinated disclosure through GitHub Security Advisories.
