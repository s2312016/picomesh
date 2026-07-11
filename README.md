# PicoMesh

[![CI](https://github.com/s2312016/picomesh/actions/workflows/ci.yml/badge.svg)](https://github.com/s2312016/picomesh/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE)

PicoMesh is a lightweight, transport-independent framework for building small distributed systems on resource-constrained microcontrollers.

The repository contains independently implemented, reusable infrastructure only. Research-specific inference models, voting algorithms, datasets, board-production files, and unpublished fault-management logic remain outside this project.

## Current scope

- Fixed-capacity, versioned frames with an 8-bit integrity check
- No heap allocation in the frame codec, stream decoder, or retry queue
- Node heartbeat and timeout tracking
- Sequence wraparound, duplicate, and stale-packet detection
- Acknowledgements and configurable retries
- Transport abstraction for I2C, UART, CAN, USB CDC, or host simulation
- Arduino Wire controller/peripheral adapters and examples
- Raspberry Pi Pico SDK I2C-controller and UART adapters
- Portable C++17 core with host-side tests and simulator

## Host quick start

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/picomesh_host_simulator
```

## Arduino quick start

Install this repository as an Arduino library, then open one of these examples:

- `ArduinoHeartbeatNode`
- `ArduinoHeartbeatController`

The controller and node use I2C address `0x42` at 100 kHz. Keep all Pico-side I2C signals at 3.3 V. Use a bidirectional level shifter when a controller board has 5 V pull-ups.

```cpp
#include <PicoMesh.h>

picomesh::Frame heartbeat;
heartbeat.type = picomesh::MessageType::heartbeat;
heartbeat.node_id = 2;
heartbeat.sequence = 7;

const auto encoded = picomesh::encode_frame(heartbeat);
```

## Pico SDK integration

After `pico_sdk_init()`, add both PicoMesh targets:

```cmake
add_subdirectory(path/to/picomesh picomesh-build)
add_subdirectory(path/to/picomesh/ports/pico-sdk picomesh-pico-build)

target_link_libraries(my_firmware
    PRIVATE PicoMesh::picomesh PicoMesh::pico_transport)
```

See [ports/pico-sdk/README.md](ports/pico-sdk/README.md).

## Architecture

```text
Application / research logic
          |
PicoMesh reliability, liveness, and protocol
          |
Transport interface
          |
I2C / UART / CAN / USB / simulator
```

See [docs/architecture.md](docs/architecture.md), [docs/protocol.md](docs/protocol.md), and [docs/project_boundary.md](docs/project_boundary.md).

## Roadmap

- **v0.1**: portable core, Arduino Wire support, Pico controller/UART adapters, host CI
- **v0.2**: Pico SDK I2C peripheral endpoint and hardware interoperability tests
- **v0.3**: packet-loss simulator, retry statistics, and command routing
- **v0.4**: optional FreeRTOS integration and bounded task mailbox
- **v0.5**: CAN and USB CDC transports
- **v1.0**: stable API and documented compatibility guarantees

## Project boundary

PicoMesh provides only general communication, liveness, framing, retry, and platform-adapter mechanisms. Application-specific models, datasets, evaluation logic, experimental algorithms, and fixed hardware topologies are intentionally excluded.

## Contributing

Contributions are welcome. Read [CONTRIBUTING.md](CONTRIBUTING.md) before opening a pull request.

## License

Apache License 2.0. See [LICENSE](LICENSE).
