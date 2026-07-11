#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace picomesh {

constexpr std::size_t kMaxNodes = 32;

struct NodeStatus {
    std::uint8_t node_id{0};
    std::uint8_t last_sequence{0};
    std::uint32_t last_seen_ms{0};
    bool online{false};
};

class NodeRegistry {
public:
    explicit NodeRegistry(std::uint32_t timeout_ms = 3000) noexcept;

    bool observe(std::uint8_t node_id, std::uint8_t sequence, std::uint32_t now_ms) noexcept;
    void refresh(std::uint32_t now_ms) noexcept;
    std::optional<NodeStatus> get(std::uint8_t node_id) const noexcept;
    std::size_t online_count() const noexcept;
    std::uint32_t timeout_ms() const noexcept { return timeout_ms_; }

private:
    std::array<NodeStatus, kMaxNodes> nodes_{};
    std::array<bool, kMaxNodes> present_{};
    std::uint32_t timeout_ms_;
};

}  // namespace picomesh
