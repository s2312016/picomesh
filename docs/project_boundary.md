# Project boundary

This document defines which material belongs in the public PicoMesh repository and which material must remain outside it.

## Public reusable scope

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
- Bounded interrupt handling and fixed-capacity mailboxes

PicoMesh implements these concepts through a new public protocol and portable APIs rather than copying private packet structures or application logic.

## Material excluded from this repository

The following remain outside PicoMesh unless they are independently published under a compatible license:

- Application-specific CNN or SNN implementations
- Model weights, labels, generated headers, and datasets
- Accuracy, timing, power, and experiment outputs
- Learned or metadata-weighted voting logic
- Fault-injection scenarios tied to research evaluation
- Consensus, failover, or task-migration algorithms that contribute to research novelty
- Private PCB, backplane, BOM, fabrication, and bring-up packages
- Fixed assumptions about a particular cluster or node topology
- Unpublished presentations, theses, and design documents

## Third-party code

Do not copy vendored FreeRTOS kernels or other third-party source trees from private repositories. PicoMesh ports should consume upstream dependencies through documented external paths, package managers, or submodules with their original licenses preserved.

## Contribution procedure

For each proposed component:

1. Identify the reusable behavior rather than copying a source file.
2. Remove private names, payload fields, topology assumptions, and research thresholds.
3. Implement the behavior behind a public PicoMesh API.
4. Add host tests and protocol documentation.
5. Confirm that no datasets, weights, secrets, generated artifacts, or incompatible third-party code were copied.
6. Link the resulting pull request to the relevant PicoMesh issue.
