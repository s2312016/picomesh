# AETERNA extraction boundary

This document records which parts of the private AETERNA repository may inform PicoMesh and which parts must remain private research material.

## Reviewed reusable patterns

The initial review covered AETERNA's root build, Pico firmware, Arduino controller, and I2C protocol documentation.

The following concepts are suitable for independent, generalized implementation in PicoMesh:

- Compact versioned packets
- Magic-byte validation
- Sequence numbers
- Additive checksum validation
- Node IDs independent from application payloads
- Heartbeat-driven state publication
- Timeout-based online/offline tracking
- Separation between controller transport and node application logic
- Host-side validation of protocol behavior

PicoMesh implements these as a new public protocol and portable APIs rather than copying the AETERNA-specific packet verbatim.

## Keep in AETERNA

The following remain private unless a separate publication decision is made:

- Galaxy CNN and SNN implementations
- Model weights, labels, generated headers, and datasets
- Accuracy, timing, power, and experiment outputs
- Learned and metadata-weighted voting logic
- Fault-injection scenarios tied to the research evaluation
- Consensus, failover, or task-migration algorithms that contribute to research novelty
- Rev G PCB, backplane, BOM, fabrication, and bring-up packages
- Fixed assumptions about three clusters and nine physical nodes
- Research presentation, thesis, and unpublished design documents

## Third-party code

Do not copy the vendored FreeRTOS kernel or other third-party source trees from AETERNA. PicoMesh ports should consume upstream dependencies through documented external paths, package managers, or submodules with their original licenses preserved.

## Migration procedure

For each future AETERNA component:

1. Identify the reusable behavior, not just the source file.
2. Remove research-specific names, payload fields, topology, and thresholds.
3. Reimplement behind a public PicoMesh API.
4. Add host tests and protocol documentation.
5. Confirm no datasets, weights, secrets, generated artifacts, or third-party code were copied.
6. Link the resulting pull request to the relevant PicoMesh issue.
