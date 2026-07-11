# Hardware validation

This document defines the evidence required before the first stable PicoMesh release.

## Reference setup

- Arduino UNO R4 Minima as I2C controller
- Raspberry Pi Pico or Pico 2 as I2C peripheral
- I2C clock: 100 kHz
- Shared ground
- SDA/SCL pull-ups to 3.3 V, or a bidirectional level shifter when required
- Pico SDK version recorded explicitly
- Arduino board package version recorded explicitly

See `docs/hardware_wiring.md` before connecting the boards.

## Build inputs

Record:

- PicoMesh commit or release tag
- Arduino IDE or Arduino CLI version
- `arduino:renesas_uno` core version
- Pico SDK version
- Selected Pico board (`pico` or `pico2`)
- Exact pin mapping and I2C addresses

## Test matrix

| Test | Expected result | Evidence |
|---|---|---|
| Controller reads heartbeat | Valid frame and increasing sequence | Serial log |
| Checksum rejection | Corrupted frame is rejected | Serial log or test output |
| Protocol-version rejection | Unsupported version is rejected | Serial log or test output |
| Node timeout | Disconnected node becomes offline | Timestamped serial log |
| Sequence wraparound | `255` to `0` is accepted as newer | Test output |
| Duplicate delivery | Duplicate sequence is not applied twice | Test output |
| ACK success | Command is accepted and removed from retry queue | Serial log |
| Lost ACK | Command is retried within configured policy | Serial log |
| Retry exhaustion | Failure is reported after maximum attempts | Serial log |
| Back-to-back writes | Unread frame is preserved or drop count increases | Serial log |
| Concurrent publish/read | No torn or malformed frame is observed | Repeated-run log |
| Pico 2 build and run | Same public protocol behavior as Pico | Serial log |

## Suggested procedure

1. Flash `examples/PicoSdkI2cNode` to the Pico-family board.
2. Upload `examples/ArduinoHeartbeatController` to the UNO R4.
3. Capture at least 60 seconds of normal heartbeat output.
4. Disconnect and reconnect the peripheral while recording timeout behavior.
5. Send a command requiring acknowledgement.
6. Repeat with the first acknowledgement intentionally suppressed.
7. Run at least 1,000 heartbeat reads while repeatedly publishing updated frames.
8. Repeat the reference tests on Pico and Pico 2 when both are available.

## Report format

Create a report under `docs/validation/` named after the release candidate, for example:

```text
docs/validation/v0.1.0-rc1.md
```

Include:

- Date and tester
- Hardware and software versions
- Wiring table
- Pass/fail table
- Relevant serial excerpts
- Known limitations
- Links to any follow-up issues

Do not include credentials, personal device identifiers, unrelated serial output, or private project material.
