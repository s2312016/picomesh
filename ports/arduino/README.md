# Arduino port

The Arduino port adapts `picomesh::Transport` to `Wire`, `Serial`, and compatible CAN libraries.

Current examples:

- Arduino controller polls a node
- Each node emits a heartbeat frame
- Controller validates the checksum and updates `NodeRegistry`
- Missing nodes transition offline after a configurable timeout

Private inference, research evaluation, and weighted-voting behavior are intentionally excluded.
