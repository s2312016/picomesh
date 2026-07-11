#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace picomesh {

/** Maximum logical node ID count tracked by @ref NodeRegistry. */
constexpr std::size_t kMaxNodes = 32;

/** @brief Last observed liveness state for one logical node. */
struct NodeStatus {
    std::uint8_t node_id{0};       ///< Logical node identifier.
    std::uint8_t last_sequence{0}; ///< Most recently observed sequence.
    std::uint32_t last_seen_ms{0}; ///< Modulo-2^32 observation time.
    bool online{false};            ///< Whether the node is within the timeout.
};

/**
 * @brief Fixed-capacity liveness registry for logical nodes.
 *
 * Observations are indexed by node ID and require no dynamic allocation.
 * Timeout calculations use unsigned wrap-safe elapsed-time arithmetic.
 */
class NodeRegistry {
  public:
    /**
     * @brief Construct a registry.
     * @param timeout_ms Maximum elapsed time before a node becomes offline.
     */
    explicit NodeRegistry(std::uint32_t timeout_ms = 3000) noexcept;

    /**
     * @brief Record a frame observation and mark the node online.
     * @param node_id Logical node identifier in `[0, kMaxNodes)`.
     * @param sequence Most recently observed sequence.
     * @param now_ms Current modulo-2^32 millisecond tick.
     * @return `false` when @p node_id is outside the fixed registry.
     */
    bool observe(std::uint8_t node_id, std::uint8_t sequence, std::uint32_t now_ms) noexcept;

    /**
     * @brief Recalculate online state for every observed node.
     * @param now_ms Current modulo-2^32 millisecond tick.
     */
    void refresh(std::uint32_t now_ms) noexcept;

    /**
     * @brief Read one node's status.
     * @return Status when the node has been observed, otherwise `std::nullopt`.
     */
    std::optional<NodeStatus> get(std::uint8_t node_id) const noexcept;

    /** @return Number of currently online observed nodes. */
    std::size_t online_count() const noexcept;

    /** @return Configured liveness timeout in milliseconds. */
    std::uint32_t timeout_ms() const noexcept {
        return timeout_ms_;
    }

  private:
    std::array<NodeStatus, kMaxNodes> nodes_{};
    std::array<bool, kMaxNodes> present_{};
    std::uint32_t timeout_ms_;
};

} // namespace picomesh
