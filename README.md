# PicoMesh

[![CI](https://github.com/s2312016/picomesh/actions/workflows/ci.yml/badge.svg)](https://github.com/s2312016/picomesh/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE)

PicoMesh is a lightweight, transport-independent framework for building small distributed systems on resource-constrained microcontrollers.

It is being extracted from reusable infrastructure developed for the private AETERNA research project. Research-specific inference models, voting algorithms, datasets, and unpublished fault-management logic remain outside this repository.

## Current scope

- Compact versioned frames with an 8-bit integrity check
- Node heartbeat and timeout tracking
- Transport abstraction suitable for I2C, UART, CAN, USB CDC, or host simulation
- Portable C++17 core
- Host-side tests and simulator
- Planned Arduino and Raspberry Pi Pico SDK ports

## Quick start

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/picomesh_host_simulator
```

## Example

```cpp
#include "picomesh/frame.h"

picomesh::Frame frame;
frame.type = picomesh::MessageType::heartbeat;
frame.node_id = 2;
frame.sequence = 7;

auto bytes = picomesh::encode_frame(frame);
auto decoded = picomesh::decode_frame(bytes);
```

## Architecture

```text
Application / research logic
          |
PicoMesh node registry and protocol
          |
Transport interface
          |
I2C / UART / CAN / USB / simulator
```

See [docs/architecture.md](docs/architecture.md) and [docs/protocol.md](docs/protocol.md).

## Roadmap

- **v0.1**: portable frame codec, checksum, heartbeat registry, host CI
- **v0.2**: Arduino Wire controller and node examples
- **v0.3**: Pico SDK I2C/UART adapters and hardware-in-the-loop tests
- **v0.4**: retries, acknowledgements, duplicate detection, and command routing
- **v0.5**: optional FreeRTOS integration
- **v1.0**: stable API and documented compatibility guarantees

## Relationship to AETERNA

PicoMesh may reuse general infrastructure patterns from AETERNA, including compact state packets, sequence numbers, node identity, heartbeat-driven liveness, and platform separation. It intentionally excludes galaxy-classification models, experimental datasets, learned weighting, research evaluation, and AETERNA-specific hardware topology.

## Contributing

Contributions are welcome. Read [CONTRIBUTING.md](CONTRIBUTING.md) before opening a pull request.

## License

Apache License 2.0. See [LICENSE](LICENSE).
