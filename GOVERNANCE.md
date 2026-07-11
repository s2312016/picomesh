# Governance

PicoMesh is maintained as an independent open-source project.

## Maintainer roles

The **primary maintainer** is responsible for:

- Reviewing and merging pull requests
- Triaging issues and security reports
- Maintaining protocol compatibility
- Coordinating releases and changelog updates
- Keeping supported toolchains and boards covered by CI
- Protecting the public/private project boundary described in `docs/project_boundary.md`

Additional core maintainers may be added after sustained, constructive contributions and a demonstrated understanding of the protocol and compatibility requirements.

## Decision process

Routine fixes and documentation changes may be merged after CI passes and the change has been reviewed for scope and compatibility.

Substantial API, protocol, security, or support-policy changes should begin with a public issue. The proposal should describe:

1. The user problem
2. Resource and compatibility impact
3. Alternatives considered
4. Migration or versioning requirements
5. Validation evidence

The primary maintainer makes the final decision when consensus cannot be reached. Decisions should be documented in the relevant issue or pull request.

## Releases

Releases follow the checklist in `RELEASING.md`. Before 1.0, breaking changes are allowed but must be documented. Protocol wire-format changes require an explicit protocol-version decision.

## Security

Potential vulnerabilities must be reported privately according to `SECURITY.md`. Public discussion begins only after a mitigation or coordinated disclosure plan is available.

## Changes to governance

Governance changes are reviewed through pull requests and recorded in the changelog when they affect contributors or users.
