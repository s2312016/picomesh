# Hardware wiring guide

## Voltage safety

Raspberry Pi Pico and Pico 2 GPIO use 3.3 V logic and are not 5 V tolerant. Connect SDA and SCL directly only when every pull-up on the bus goes to 3.3 V.

Use a bidirectional MOSFET level shifter when an Arduino or expansion board pulls I2C to 5 V. Always connect the grounds of all boards.

## Minimal two-node I2C setup

```text
Controller SDA ----+---- Node SDA
Controller SCL ----+---- Node SCL
Controller GND -------- Node GND
              3.3 V pull-ups
```

The Arduino examples default to:

- I2C address: `0x42`
- Clock: 100 kHz
- Maximum encoded PicoMesh frame: 40 bytes

Some AVR-class Arduino Wire implementations use a 32-byte buffer. On those boards, keep the PicoMesh application payload at 24 bytes or less, because the protocol adds eight bytes of overhead. UNO R4 has a different core, but applications should still check the board's Wire buffer before using the full 32-byte payload.

## Multiple nodes

Assign every node a unique I2C address. A simple convention is:

```text
node 0 -> 0x42
node 1 -> 0x43
node 2 -> 0x44
```

PicoMesh does not require this convention and does not assume AETERNA's three-node cluster or nine-node backplane topology.

## Node identity

The protocol node ID and the physical I2C address are independent:

- I2C address selects a device on one bus.
- PicoMesh node ID identifies the logical node in frames.

Applications may obtain node IDs from GPIO straps, flash configuration, EEPROM, a provisioning command, or compile-time settings.

## Pull-ups and bus length

Start with 4.7 kOhm pull-ups to 3.3 V for short breadboard wiring. Only one effective pair of pull-ups is needed. Many development boards already include pull-ups, so measure or inspect the schematics before adding more.

Use short wires and begin at 100 kHz. Long wiring, high bus capacitance, mixed-voltage modules, or many parallel pull-ups can cause corrupted frames and intermittent timeouts.

## Bring-up sequence

1. Verify the bus voltage with a multimeter before connecting the Pico.
2. Run an I2C scanner and confirm each address appears once.
3. Start with the heartbeat node and controller examples.
4. Confirm checksums and sequence numbers increase normally.
5. Disconnect one node and verify `NodeRegistry` marks it offline after the configured timeout.
6. Add commands and acknowledgements only after heartbeat polling is stable.
