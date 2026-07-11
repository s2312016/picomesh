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

## Compatibility

Receivers must reject unknown protocol versions unless an adapter explicitly supports them. Application payloads should define their own schema version when long-term compatibility is required.

## Origin and separation

The compact packet, sequence, and checksum pattern was generalized from AETERNA's internal I2C state exchange. AETERNA's 26-byte galaxy-inference response and fault-injection commands are intentionally not part of the public protocol.

## Sequence handling

Sequence numbers use modulo-256 arithmetic. A receiver may classify a packet as newer when the unsigned distance from the last accepted sequence is `1..127`, duplicate when it is `0`, and stale when it is `128..255`. This permits normal wraparound from `255` to `0` while rejecting delayed duplicates.
