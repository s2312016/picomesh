# Architecture

PicoMesh uses four layers:

1. **Application layer** — user commands, telemetry schemas, robotics, sensing, or research logic.
2. **Mesh core** — frame encoding, decoding, sequence handling, liveness, and node registry.
3. **Transport adapter** — I2C, UART, CAN, USB CDC, radio, or an in-memory simulator.
4. **Platform layer** — Arduino, Pico SDK, FreeRTOS, bare metal, or desktop operating systems.

The core must not include AETERNA-specific inference models, datasets, weighted voting, hardware revision names, or fixed nine-node assumptions.

## Design principles

- Small fixed limits suitable for microcontrollers
- Explicit wire versions
- Deterministic parsing
- No exceptions in the decoding path
- Host-testable logic
- Board-specific code isolated in `ports/`
- Research algorithms supplied by the consuming application
