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
- Frame-length detection for fixed-size transports that pad short reads.
- Arduino Wire controller and peripheral adapters.
- Arduino heartbeat, multi-node controller, and reliable-command examples.
- Arduino UNO R4 compile CI.
- Raspberry Pi Pico SDK I2C controller, I2C peripheral, and UART adapters.
- Pico and Pico 2 heartbeat example and build CI.
- Host simulators, unit tests, strict warning checks, sanitizer checks, and install-package tests.
- CodeQL security analysis and Dependabot configuration.
- Tagged-release workflow with source archives, checksums, and Pico/Pico 2 firmware artifacts.
- Governance, maintainer, release, compatibility, resource-limit, validation, and automation documentation.
- English and Japanese project documentation.

### Fixed

- CMake package validation now configures release builds correctly on single-config Linux and macOS generators.
- I2C controller adapters trim padded reads to the encoded frame length before strict decoding.

### Security

- Bounded payload, frame, retry, node, RX, and TX storage.
- Malformed lengths, protocol versions, and checksums are rejected before payload use.
- Host tests run under AddressSanitizer and UndefinedBehaviorSanitizer.

## [0.1.0] - TBD

The first release will be tagged after real Arduino-to-Pico interoperability evidence is recorded and all release-blocking CI checks pass.
