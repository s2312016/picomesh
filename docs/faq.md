# Frequently asked questions

## Is PicoMesh a mesh-network routing stack?

Not currently. The name refers to coordinating small groups of microcontroller nodes. The public core provides framing, sequencing, liveness, ACKs, retries, and transport boundaries. Multi-hop routing is not part of v0.1.

## Does PicoMesh allocate memory dynamically?

The frame codec, stream decoder, node registry, sequence tracker, and reliable queue use fixed-capacity storage. Applications and platform libraries may still allocate memory outside those components.

## Which transports are supported?

The portable protocol is transport-independent. The repository currently includes Arduino Wire adapters and Pico SDK I2C/UART adapters. CAN, USB CDC, radio, and host serial links can implement the same transport boundary, but a roadmap entry is not the same as a supported implementation.

## How large is a frame?

The maximum application payload is 32 bytes and the protocol adds eight bytes of overhead, producing a maximum encoded frame size of 40 bytes. Some Arduino Wire implementations have smaller buffers, so applications must check the selected core before using the full payload.

## Are I2C addresses the same as node IDs?

No. An I2C address selects a device on one physical bus. A PicoMesh node ID identifies the logical source or destination represented in frames. They may use matching numbers by convention, but the protocol does not require it.

## Can a Pico be connected directly to a 5 V Arduino?

Only when every SDA/SCL pull-up is verified to be 3.3 V. Pico GPIO is not 5 V tolerant. Use a suitable bidirectional level shifter when the controller side or an attached module may pull the bus to 5 V.

## Why can the same sequence appear more than once?

A controller may read a published frame more frequently than the peripheral refreshes it. The sequence tracker classifies repeated values as duplicates. A retry may also intentionally resend the same command sequence until an ACK is received.

## What happens when sequence 255 is followed by 0?

The tracker uses wrap-aware 8-bit sequence comparisons, so that transition is treated as newer. Large backward jumps are treated as stale.

## Does a successful CI build prove hardware compatibility?

No. CI proves compilation and host-side behavior for the tested configuration. It does not prove voltage safety, pull-up selection, bus capacitance, timing margin, or physical interoperability. Hardware claims require a validation report.

## Can PicoMesh be used with an RTOS?

Yes, the portable core can be called from an RTOS application, but callers must provide appropriate synchronization. Dedicated bounded FreeRTOS adapters are planned rather than claimed as complete.

## How should protocol changes be proposed?

Open an issue before changing the wire format. A protocol change must include compatibility analysis, tests, documentation updates, and a versioning decision. See [`compatibility.md`](compatibility.md) and [`CONTRIBUTING.md`](../CONTRIBUTING.md).

## How do I report a security problem?

Follow [`SECURITY.md`](../SECURITY.md). Do not publish sensitive vulnerability details in a public issue before coordinated disclosure.
