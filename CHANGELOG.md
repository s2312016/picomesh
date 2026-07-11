# Changelog

All notable changes to PicoMesh will be documented in this file.

The format is based on Keep a Changelog, and the project intends to follow Semantic Versioning after the public API reaches stability.

## [Unreleased]

### Added

- Portable fixed-capacity PicoMesh v1 frame codec.
- Additive checksum validation and bounded stream decoder.
- Heartbeat-oriented node registry with wrap-safe timeouts.
- Modulo-256 duplicate and stale sequence detection.
- Acknowledgement frames and a fixed-capacity retry queue.
- Arduino Wire controller and peripheral adapters.
- Arduino UNO R4 heartbeat examples and compile CI.
- Raspberry Pi Pico SDK I2C controller, I2C peripheral, and UART adapters.
- Pico and Pico 2 heartbeat example and build CI.
- Host simulators, unit tests, strict warning checks, and install-package tests.
- English and Japanese documentation.

### Security

- Bounded payload, frame, retry, node, RX, and TX storage.
- Malformed lengths, protocol versions, and checksums are rejected before payload use.

## [0.1.0] - TBD

The first release will be tagged after real Arduino-to-Pico interoperability evidence is recorded and all release-blocking CI checks pass.
