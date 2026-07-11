# Resource limits

PicoMesh keeps protocol and reliability storage bounded so applications can account for memory use on small microcontrollers.

## Portable core limits

| Resource | Current limit | Definition |
|---|---:|---|
| Application payload | 32 bytes | `kMaxPayloadSize` |
| Encoded frame | 40 bytes | `kMaxEncodedFrameSize` |
| Tracked logical nodes | 32 | `kMaxNodes` |
| Pending reliable transmissions | 8 | `kMaxPendingTransmissions` |
| Sequence space | 256 values | 8-bit sequence field |

The frame codec, stream decoder, sequence tracker, node registry, and reliable queue do not allocate from the heap.

## Frame overhead

Each frame uses:

- 7-byte header
- 0–32-byte application payload
- 1-byte checksum

Applications using a transport with a 32-byte packet buffer should limit application payloads to 24 bytes unless the platform adapter documents fragmentation or a larger buffer.

## Pico SDK I2C endpoint

The endpoint uses two bounded receive buffers and two bounded transmit buffers. Each buffer can hold one maximum-size encoded frame. Incoming frames are decoded outside the interrupt handler. When a completed receive frame has not been consumed before another frame completes, the endpoint increments `dropped_frames()` instead of overwriting unread data.

## Arduino Wire endpoint

The Arduino endpoint stores one bounded receive frame and one bounded transmit frame. Actual transferable length can be lower than PicoMesh's 40-byte maximum when the selected Arduino core uses a smaller internal Wire buffer.

## Compiled firmware reports

The Pico SDK workflow builds the reference I2C node for Pico and Pico 2, runs `arm-none-eabi-size` on each ELF, and uploads the ELF, UF2, and text size report as per-board artifacts. These reports make toolchain and example growth visible during review.

A reference-example ELF includes the Pico SDK startup, standard library, USB stdio, I2C driver, and application code. Its reported flash and RAM values are not the standalone cost of the portable PicoMesh core and must not be advertised as such. Compare reports produced by the same workflow and toolchain configuration.

## Timing limits

Node liveness and retry deadlines use unsigned 32-bit millisecond arithmetic with wrap-safe elapsed-time comparisons. Applications should keep individual timeout intervals below half of the 32-bit timer range.

## Changing limits

Limit changes affect RAM usage and may affect transport compatibility. Pull requests changing a public limit must include:

- The reason for the change
- Memory impact on supported boards
- Updated tests and documentation
- Compatibility analysis for existing users
