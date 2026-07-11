# Compatibility policy

PicoMesh has two compatibility surfaces: the C++ API and the encoded wire protocol.

## Before 1.0

The C++ API may change between minor releases while the project gathers hardware feedback. Breaking changes must be documented in `CHANGELOG.md` and release notes.

The wire protocol is treated more conservatively. A released frame format must not be changed incompatibly while keeping the same protocol version.

## Wire protocol rules

A protocol-version change is required when a receiver implementing the previous version could incorrectly parse or apply a new frame. Examples include:

- Reordering or resizing header fields
- Changing checksum semantics
- Reinterpreting existing flag bits
- Changing acknowledgement payload meaning
- Altering sequence comparison rules

A protocol-version change is not normally required for:

- New application-defined message types in the documented extension range
- New payload schemas that carry their own schema version
- New transport adapters that preserve encoded bytes
- New diagnostics outside the wire frame

Receivers reject unsupported protocol versions by default.

## Stable 1.x policy

After 1.0, the project intends to preserve source compatibility within a major version where practical and wire compatibility for every documented protocol version. Deprecations will be documented before removal.

## Platform support

A platform is considered supported only when it is documented and covered by automated compilation or a repeatable hardware validation report. Experimental ports may be merged behind clear documentation without being listed as stable.
