# PicoMesh wire protocol v1

A frame contains a seven-byte header, zero to 32 payload bytes, and one checksum byte.

| Offset | Field | Size |
|---:|---|---:|
| 0 | Magic (`0xB7`) | 1 |
| 1 | Protocol version (`1`) | 1 |
| 2 | Message type | 1 |
| 3 | Flags | 1 |
| 4 | Node ID | 1 |
| 5 | Sequence | 1 |
| 6 | Payload length | 1 |
| 7 | Payload | 0–32 |
| final | Two's-complement checksum | 1 |

The unsigned sum of every encoded byte, including the checksum, must equal zero modulo 256.

## Standard message types

- `0x01`: heartbeat
- `0x02`: state
- `0x10`: command
- `0x11`: acknowledgement
- `0x80`–`0xFF`: application-defined

## Flags

- Bit 0 (`0x01`): acknowledgement required
- Bits 1–7: reserved for future protocol revisions and must be zero when unused

## Acknowledgement payload

An acknowledgement frame has a two-byte payload:

| Payload offset | Field |
|---:|---|
| 0 | Sequence number being acknowledged |
| 1 | Status code |

Status codes are:

- `0`: accepted
- `1`: rejected
- `2`: temporarily busy
- `3`: unsupported command or feature
- `4`: malformed request

The acknowledgement frame's own sequence field is the sender's normal outgoing sequence number. Matching uses the acknowledgement sender's node ID and payload byte 0.

## Retry behavior

The core `ReliableQueue` provides a fixed capacity of eight pending transmissions. Applications choose the timeout and maximum number of attempts. A transmission is removed when an acknowledgement arrives, is cancelled, or reaches the configured attempt limit.

Commands should be idempotent whenever practical because an acknowledgement may be lost after the receiver has already applied the command. Receivers should combine `SequenceTracker` with application-level command handling to reject duplicate or stale commands.

Sequence comparison uses modulo-256 distance. Distances `1..127` are newer, `0` is a duplicate, and `128..255` are stale. This permits normal wraparound from `255` to `0`.

## Stream transports

UART, USB CDC, and other byte-stream transports should use `StreamDecoder`. It discards noise before the magic byte, determines the frame length after reading the header, enforces the 32-byte payload limit, and validates the checksum without heap allocation.

## Compatibility

Receivers must reject unknown protocol versions unless an adapter explicitly supports them. Application payloads should define their own schema version when long-term compatibility is required.

## Project boundary

The public protocol contains only general framing, identity, sequencing, liveness, and reliability fields. Application-specific inference results, experimental fault commands, datasets, and fixed hardware topology are not part of the protocol.
