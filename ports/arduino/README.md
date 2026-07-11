# Arduino port

The Arduino port will adapt `picomesh::Transport` to `Wire`, `Serial`, and compatible CAN libraries.

Planned first example:

- Arduino controller polls two or more nodes
- Each node emits a heartbeat frame
- Controller validates the checksum and updates `NodeRegistry`
- Missing nodes transition offline after a configurable timeout

No AETERNA-specific inference or weighted-voting behavior will be included here.
