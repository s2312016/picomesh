# PicoMesh roadmap

The roadmap describes direction rather than guaranteed dates. Each milestone must preserve deterministic memory use and keep research-specific AETERNA logic outside the public core.

## v0.1 — usable foundation

- Portable fixed-capacity protocol core
- Heartbeats and liveness tracking
- Duplicate and stale-packet detection
- Acknowledgements and bounded retries
- Arduino Wire support
- Pico SDK I2C and UART support
- Host, Arduino, and Pico CI
- First real Arduino-to-Pico interoperability report

## v0.2 — reliable multi-node links

- Reliability statistics and diagnostics
- Command routing primitives
- Multiple-node Arduino controller example
- Explicit application payload schemas
- More complete malformed-frame and packet-loss simulation

## v0.3 — RTOS integration

- Optional FreeRTOS queue/event adapters
- Bounded task mailbox
- Non-blocking transport service examples
- Timing and memory-usage measurements on RP2040 and RP2350

## v0.4 — additional transports

- CAN transport abstraction and example
- USB CDC stream adapter
- Linux/serial gateway example
- Protocol capture and decoding utility

## v1.0 — stable API

- Compatibility policy
- Long-term protocol-version rules
- Stable C++ API and documented resource limits
- Reproducible hardware validation across supported boards
- Release and security maintenance process

## Explicit non-goals

PicoMesh will not publish AETERNA's research models, datasets, experimental voting logic, task-migration policy, fault-injection scenarios, or board-production files. Those may consume PicoMesh through public extension points.
