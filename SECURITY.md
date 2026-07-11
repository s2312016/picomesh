# Security policy

Please do not disclose suspected vulnerabilities in public issues, pull requests, discussions, or social media.

## Supported versions

PicoMesh is currently pre-1.0. Until a stable support policy is announced, security fixes are provided for the latest released version and the current `main` branch when practical.

| Version | Security fixes |
|---|---|
| Latest release | Supported |
| `main` | Supported for upcoming fixes |
| Older pre-1.0 releases | Upgrade recommended |

## Reporting a vulnerability

Use GitHub private vulnerability reporting for this repository. Include:

- Affected commit or release
- Impact and realistic attack conditions
- Minimal reproduction steps or proof of concept
- Affected board, transport, and toolchain
- Any suggested mitigation
- Whether public disclosure has already occurred

The maintainer aims to acknowledge a complete report within seven days and provide an initial triage within fourteen days. These are response goals rather than contractual guarantees.

## Security-relevant scope

Reports are especially useful for:

- Out-of-bounds reads or writes
- Integer overflow affecting lengths or deadlines
- Malformed-frame handling
- Memory corruption in interrupt-driven transports
- Denial of service through queue or mailbox exhaustion
- Protocol confusion or acknowledgement spoofing
- Build or release workflow compromise
- Incompatible third-party dependency use

## Disclosure process

After confirming a vulnerability, the maintainer will coordinate a fix, targeted tests, affected-version guidance, and an advisory. Public disclosure should occur after a corrected release is available whenever possible.

## Safety statement

PicoMesh is not certified for medical, automotive, aerospace, industrial-safety, or other life-critical use. Users are responsible for system-level risk assessment, redundancy, electrical safety, and validation.
