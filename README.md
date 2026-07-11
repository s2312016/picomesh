# PicoMesh

[![CI](https://github.com/s2312016/picomesh/actions/workflows/ci.yml/badge.svg)](https://github.com/s2312016/picomesh/actions/workflows/ci.yml)
[![Arduino](https://github.com/s2312016/picomesh/actions/workflows/arduino.yml/badge.svg)](https://github.com/s2312016/picomesh/actions/workflows/arduino.yml)
[![Pico SDK](https://github.com/s2312016/picomesh/actions/workflows/pico-sdk.yml/badge.svg)](https://github.com/s2312016/picomesh/actions/workflows/pico-sdk.yml)
[![Static analysis](https://github.com/s2312016/picomesh/actions/workflows/static-analysis.yml/badge.svg)](https://github.com/s2312016/picomesh/actions/workflows/static-analysis.yml)
[![Documentation](https://github.com/s2312016/picomesh/actions/workflows/docs.yml/badge.svg)](https://github.com/s2312016/picomesh/actions/workflows/docs.yml)
[![CodeQL](https://github.com/s2312016/picomesh/actions/workflows/codeql.yml/badge.svg)](https://github.com/s2312016/picomesh/actions/workflows/codeql.yml)
[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE)

[日本語](README_ja.md)

PicoMesh is a lightweight, transport-independent framework for building small distributed systems on resource-constrained microcontrollers.

It provides deterministic framing, liveness, sequencing, acknowledgements, and retries without requiring dynamic memory in protocol-critical paths. The same portable C++17 core can be exercised on a desktop simulator and used from Arduino or the Raspberry Pi Pico SDK.

> **Project status:** pre-1.0. The portable core and supported build targets are automated, while real-hardware interoperability evidence is being completed for the first release.

## Why PicoMesh

Embedded projects repeatedly reimplement packet framing, checksums, heartbeat tracking, duplicate detection, retries, and platform-specific I/O. PicoMesh places those mechanisms behind a small documented protocol and transport boundary so application code can focus on sensing, control, robotics, education, or research.

## Current capabilities

- Fixed-capacity, versioned frames with an 8-bit integrity check
- No heap allocation in the frame codec, stream decoder, node registry, or retry queue
- Heartbeat and wrap-safe timeout tracking for up to 32 logical nodes
- Sequence wraparound, duplicate, and stale-packet detection
- Acknowledgements and a bounded configurable retry queue
- Transport abstraction for I2C, UART, CAN, USB CDC, radio, or host simulation
- Arduino Wire controller and peripheral adapters
- Raspberry Pi Pico/Pico 2 SDK I2C controller, I2C peripheral, and UART adapters
- Host simulator, lossy-link simulator, unit tests, sanitizers, static analysis, and CodeQL
- CMake install package plus Arduino and PlatformIO metadata

Resource limits are listed in [docs/resource_limits.md](docs/resource_limits.md). New users should start with the [getting started guide](docs/getting_started.md) and [example catalog](docs/examples.md).

## Host quick start

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/picomesh_frame_codec_example
./build/picomesh_reliable_queue_example
./build/picomesh_host_simulator
```

PicoMesh can also be installed as a CMake package and consumed with:

```cmake
find_package(PicoMesh CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE PicoMesh::picomesh)
```

## Arduino quick start

Install this repository as an Arduino library, then open one of these examples:

- `ArduinoHeartbeatNode`
- `ArduinoHeartbeatController`
- `ArduinoMultiNodeController`
- `ArduinoReliableCommandController`

The reference I2C examples use address `0x42` at 100 kHz. Keep Pico-side I2C signals at 3.3 V and read [docs/hardware_wiring.md](docs/hardware_wiring.md) before connecting mixed-voltage boards.

```cpp
#include <PicoMesh.h>

picomesh::Frame heartbeat;
heartbeat.type = picomesh::MessageType::heartbeat;
heartbeat.node_id = 2;
heartbeat.sequence = 7;

const auto encoded = picomesh::encode_frame(heartbeat);
```

## Raspberry Pi Pico SDK

After `pico_sdk_init()`, add both PicoMesh targets:

```cmake
add_subdirectory(path/to/picomesh picomesh-build)
add_subdirectory(path/to/picomesh/ports/pico-sdk picomesh-pico-build)

target_link_libraries(my_firmware
    PRIVATE PicoMesh::picomesh PicoMesh::pico_transport)
```

See [ports/pico-sdk/README.md](ports/pico-sdk/README.md) and the standalone `PicoSdkI2cNode` example.

## Architecture

```text
Application logic
       |
PicoMesh protocol, reliability, and liveness
       |
Transport adapter
       |
I2C / UART / CAN / USB / simulator
```

Key documents:

- [Getting started](docs/getting_started.md)
- [Example catalog](docs/examples.md)
- [Frequently asked questions](docs/faq.md)
- [Architecture](docs/architecture.md)
- [Wire protocol](docs/protocol.md)
- [Compatibility policy](docs/compatibility.md)
- [Resource limits](docs/resource_limits.md)
- [Use cases](docs/use_cases.md)
- [Hardware validation](docs/hardware_validation.md)
- [Static analysis policy](docs/static_analysis.md)
- [Public project boundary](docs/project_boundary.md)

## Roadmap

- **v0.1**: portable core, Arduino Wire support, Pico SDK support, CI, and reference hardware validation
- **v0.2**: reliability statistics, richer malformed-link simulation, and command routing
- **v0.3**: optional FreeRTOS integration and bounded task mailboxes
- **v0.4**: CAN, USB CDC, and gateway tooling
- **v1.0**: stable API and documented long-term compatibility guarantees

See [ROADMAP.md](ROADMAP.md) for details.

## Maintainers and contributions

Contributions are welcome. Start with [CONTRIBUTING.md](CONTRIBUTING.md), then review [GOVERNANCE.md](GOVERNANCE.md) and [MAINTAINERS.md](MAINTAINERS.md). Security reports follow [SECURITY.md](SECURITY.md).

Maintainer automation principles and planned review workflows are documented in [docs/maintainer_automation.md](docs/maintainer_automation.md).

## License

Apache License 2.0. See [LICENSE](LICENSE).
