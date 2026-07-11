# Example catalog

Examples are grouped by the environment in which they run. Host examples are compiled and executed by CI. Arduino and Pico SDK examples are compiled by CI; physical behavior still requires a hardware validation record.

## Host examples

| Target | Source | Demonstrates |
|---|---|---|
| `picomesh_frame_codec_example` | `examples/frame_codec_example.cpp` | Constructing, encoding, decoding, and validating a frame |
| `picomesh_reliable_queue_example` | `examples/reliable_queue_example.cpp` | Scheduling a send, waiting for a retry deadline, and resolving an ACK |
| `picomesh_host_simulator` | `examples/host_simulator.cpp` | Frame output and node liveness timeout behavior |
| `picomesh_lossy_link_simulator` | `examples/lossy_link_simulator.cpp` | Deterministic retry behavior across a simulated lossy link |

Build and run them with:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The two minimal examples are intended as copyable starting points. The simulators are intended for behavior exploration and regression tests.

## Arduino examples

### ArduinoHeartbeatNode

Runs as an I2C peripheral and publishes a heartbeat frame. Use it with an I2C controller that requests the encoded frame at the configured address.

### ArduinoHeartbeatController

Polls one I2C peripheral, decodes frames, and updates `NodeRegistry`. Disconnecting the node should eventually reduce the online count after the configured timeout.

### ArduinoMultiNodeController

Polls multiple unique I2C addresses. It demonstrates that the physical I2C address and logical PicoMesh node ID are separate values.

### ArduinoReliableCommandController

Queues a command with the ACK-required flag, sends it when due, retries after a bounded timeout, and removes it after a matching ACK.

## Pico SDK example

### PicoSdkI2cNode

Runs on Pico or Pico 2 as an I2C peripheral endpoint. It publishes heartbeats and responds to ACK-required commands. The reference configuration uses:

- address `0x42`
- GP4 for SDA
- GP5 for SCL
- 100 kHz

Build instructions are in [`getting_started.md`](getting_started.md).

## Choosing an example

- Start with `frame_codec_example.cpp` to learn the data model.
- Use `reliable_queue_example.cpp` before integrating commands and ACKs.
- Use the host simulators to explore timeouts and packet loss.
- Use the Arduino heartbeat pair for the first physical link.
- Move to the reliable command examples only after heartbeat polling is stable.

## Evidence levels

| Evidence | Meaning |
|---|---|
| Host test passes | Portable logic behaves as asserted on the host compiler |
| Arduino/Pico build passes | The source compiles for the selected core or SDK |
| Hardware validation passes | The documented boards, wiring, toolchain, and procedure were physically tested |

Do not describe a compile-only example as hardware-validated.
