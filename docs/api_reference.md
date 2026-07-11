# Public API reference

PicoMesh keeps its portable public API under `include/picomesh`. The generated Doxygen documentation uses these headers as the source of truth.

## Core headers

| Header | Main API | Purpose |
|---|---|---|
| `checksum.h` | `checksum8`, `verify_checksum8` | Protocol integrity byte |
| `frame.h` | `Frame`, `EncodedFrame`, `encode_frame`, `decode_frame` | Fixed-capacity wire representation |
| `node_registry.h` | `NodeRegistry`, `NodeStatus` | Heartbeat-based liveness state |
| `sequence_tracker.h` | `SequenceTracker`, `SequenceState` | Duplicate, newer, stale, and wraparound classification |
| `reliability.h` | `ReliableQueue`, ACK helpers | Bounded acknowledgement and retry behavior |
| `stream_decoder.h` | `StreamDecoder`, `StreamResult` | Incremental UART/USB-style stream decoding |
| `transport.h` | `Transport` | Platform-neutral byte I/O boundary |
| `version.h` | version macros and constants | Compile-time library version |

Arduino users normally include `PicoMesh.h`, which aggregates the portable headers and exposes the Arduino transport when `ARDUINO` is defined. Pico SDK users include the portable headers plus the adapter header from `ports/pico-sdk`.

## Ownership and lifetime

- Frames and encoded buffers own their storage.
- The portable core does not retain caller-owned pointers.
- `Transport::receive` writes only into the caller-supplied buffer and must respect its capacity.
- `NodeRegistry`, `SequenceTracker`, `StreamDecoder`, and `ReliableQueue` store bounded state internally.
- Public functions marked `noexcept` are intended for embedded builds without exception handling.

## Time values

Liveness and retry APIs accept unsigned 32-bit millisecond ticks. Callers may use `millis()`, a Pico SDK timer converted to milliseconds, or another monotonic tick source. Calculations are designed to tolerate normal modulo-2^32 wraparound, provided configured intervals remain within the unambiguous half-range.

## Node IDs and addresses

A logical node ID is not a transport address. For example, an I2C address selects one peripheral on a bus while `Frame::node_id` identifies the logical node represented by the message. Applications may use the same numeric convention for convenience, but PicoMesh does not require it.

## Error handling

`decode_frame` returns a `DecodeResult` containing a specific `DecodeError`. It performs strict validation in this order:

1. minimum length;
2. magic byte;
3. protocol version;
4. payload bound;
5. exact encoded length;
6. checksum.

`StreamDecoder` additionally reports discarded noise and can recover for the next candidate frame without allocating memory.

## Reliability model

`ReliableQueue` stores at most `kMaxPendingTransmissions` in-flight frames. Enqueueing sets the ACK-required flag. The caller asks `next_due(now_ms)` for work, sends frames returned with `TxAction::send`, and passes received ACK fields to `acknowledge`. Exhaustion removes the frame and is reported once.

The queue schedules retries; it does not perform I/O or define application command semantics.

## Compatibility

The wire format and public API are pre-1.0. Protocol changes require tests, documentation, and a compatibility decision. See [`compatibility.md`](compatibility.md) before depending on undocumented layout or behavior.

## Generate documentation locally

Install Doxygen, then run:

```sh
doxygen Doxyfile
```

Open `build-docs/html/index.html`. Pull requests also publish the generated HTML as a workflow artifact.
