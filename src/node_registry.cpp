#include "picomesh/node_registry.h"

namespace picomesh {

NodeRegistry::NodeRegistry(const std::uint32_t timeout_ms) noexcept
    : timeout_ms_(timeout_ms) {}

bool NodeRegistry::observe(
    const std::uint8_t node_id,
    const std::uint8_t sequence,
    const std::uint32_t now_ms) noexcept {
    if (node_id >= kMaxNodes) {
        return false;
    }

    auto& node = nodes_[node_id];
    node.node_id = node_id;
    node.last_sequence = sequence;
    node.last_seen_ms = now_ms;
    node.online = true;
    present_[node_id] = true;
    return true;
}

void NodeRegistry::refresh(const std::uint32_t now_ms) noexcept {
    for (std::size_t i = 0; i < kMaxNodes; ++i) {
        if (!present_[i]) {
            continue;
        }

        auto& node = nodes_[i];
        const std::uint32_t elapsed = static_cast<std::uint32_t>(now_ms - node.last_seen_ms);
        node.online = elapsed <= timeout_ms_;
    }
}

std::optional<NodeStatus> NodeRegistry::get(const std::uint8_t node_id) const noexcept {
    if (node_id >= kMaxNodes || !present_[node_id]) {
        return std::nullopt;
    }
    return nodes_[node_id];
}

std::size_t NodeRegistry::online_count() const noexcept {
    std::size_t count = 0;
    for (std::size_t i = 0; i < kMaxNodes; ++i) {
        if (present_[i] && nodes_[i].online) {
            ++count;
        }
    }
    return count;
}

}  // namespace picomesh
