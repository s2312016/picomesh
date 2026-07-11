# Use cases

PicoMesh targets small distributed embedded systems where a full network stack or dynamic-memory-heavy middleware is unnecessary.

## Multi-board sensing

A controller can poll several low-cost microcontroller nodes that publish heartbeat and sensor-state frames. `NodeRegistry` tracks which nodes are online without coupling the application to a specific bus or board.

## Robotics subsystems

Motor, sensing, power, and user-interface controllers can exchange compact commands and status frames over I2C, UART, CAN, or USB CDC while sharing the same framing and reliability rules.

## Classroom and laboratory platforms

The fixed-size protocol, host simulator, and hardware examples make it possible to demonstrate sequencing, timeouts, retries, duplicate detection, and transport abstraction using inexpensive Arduino and Raspberry Pi Pico boards.

## Fault-aware prototypes

Applications can detect missing heartbeats and provide their own failover policy above PicoMesh. The public core supplies deterministic communication and liveness mechanisms without embedding an application-specific consensus or migration algorithm.

## Mixed-platform systems

The portable C++17 core allows the same frame codec and tests to be used on desktop simulators, Arduino-compatible boards, RP2040/RP2350 firmware, and future RTOS or gateway ports.

## Why a separate framework is useful

Many embedded projects repeatedly implement packet framing, checksums, sequence handling, node timeouts, acknowledgements, and retries in application code. PicoMesh consolidates those mechanisms into a small, testable layer with explicit memory limits and a documented wire format.
