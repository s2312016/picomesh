# Maintainer automation

PicoMesh is designed to support transparent, reviewable maintainer automation without allowing automation to bypass tests or compatibility review.

## High-value workflows

### Pull-request review

Automation may summarize changes, identify compatibility-sensitive files, suggest missing tests, and flag changes to protocol constants or fixed resource limits. A maintainer remains responsible for approval and merge decisions.

### Issue triage

Automation may classify incoming reports by board, transport, compiler, protocol version, and reproducibility. It may request missing logs or wiring details, but should not close valid reports solely from a model decision.

### Release preparation

Automation may draft changelog entries, compare version declarations, summarize merged pull requests, check release prerequisites, and generate first-pass release notes. Published artifacts and notes must be reviewed by a maintainer.

### CI failure analysis

Automation may retrieve failed-job logs, identify likely root causes, and propose focused patches. Generated fixes must pass the same host, Arduino, Pico SDK, sanitizer, and security checks as human-authored changes.

### Security maintenance

Automation may help review parser changes, resource bounds, dependency updates, and security advisories. Vulnerability details must remain in private reporting channels until coordinated disclosure.

## Guardrails

- Never commit credentials or private research material.
- Never silently change the wire protocol.
- Never publish a release without maintainer review.
- Never disable failing checks merely to make a pull request green.
- Keep generated comments concise and link each claim to code, tests, or logs.
- Record material automated changes in normal commits and pull requests.

## Intended API-credit use

API credits would be used for pull-request review assistance, issue triage, CI-log analysis, release-note preparation, compatibility checks, documentation maintenance, and security-focused review of the fixed-memory protocol and hardware transport layers.
