# Protocol conformance vectors

These version-1 vectors fix the exact byte order, flag placement, payload length, and checksum behavior of the public wire format. Implementations in other languages or transports can use them as interoperability fixtures.

All values below are hexadecimal. The final byte is the two's-complement checksum that makes the unsigned sum of the complete frame equal to zero modulo 256.

## Empty heartbeat

Fields:

- type: heartbeat (`01`)
- flags: `00`
- node ID: `00`
- sequence: `00`
- payload length: `00`

```text
B7 01 01 00 00 00 00 47
```

## ACK-required command

Fields:

- type: command (`10`)
- flags: ACK-required (`01`)
- node ID: `07`
- sequence: `2A`
- payload: `10 20 30`

```text
B7 01 10 01 07 2A 03 10 20 30 A3
```

## Accepted acknowledgement

Fields:

- type: acknowledgement (`11`)
- flags: `00`
- publishing node ID: `07`
- acknowledgement sequence: `2B`
- acknowledged sequence: `2A`
- status: accepted (`00`)

```text
B7 01 11 00 07 2B 02 2A 00 D9
```

## State payload

Fields:

- type: state (`02`)
- flags: `00`
- node ID: `05`
- sequence: `17`
- payload: `42 99`

```text
B7 01 02 00 05 17 02 42 99 4D
```

## Automated enforcement

`tests/test_protocol_vectors.cpp` constructs each logical frame, compares every encoded byte with the published vector, and decodes the published bytes again. The test runs on Linux, Windows, and macOS through CTest.

Changing any vector is a compatibility-sensitive protocol change. A pull request must explain migration behavior, update `docs/protocol.md`, add compatibility tests, and make an explicit versioning decision.
