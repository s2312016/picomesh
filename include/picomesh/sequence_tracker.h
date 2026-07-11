#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "picomesh/node_registry.h"

namespace picomesh {

/** @brief Classification of a sequence observation. */
enum class SequenceState {
    first,        ///< First sequence observed for the node.
    newer,        ///< Forward distance is between 1 and 127.
    duplicate,    ///< Sequence matches the last accepted value.
    stale,        ///< Sequence is older or delayed by 128 to 255 steps.
    invalid_node, ///< Node ID is outside `[0, kMaxNodes)`.
};

/**
 * @brief Fixed-capacity modulo-256 sequence classifier.
 *
 * The half-range rule treats forward distances 1 through 127 as newer and
 * distances 128 through 255 as stale. This accepts normal wraparound from 255
 * to 0 while rejecting delayed packets.
 */
class SequenceTracker {
public:
    /**
     * @brief Classify and, when newer, remember a sequence.
     * @param node_id Logical node identifier.
     * @param sequence Sequence to observe.
     * @return Classification for this observation.
     */
    SequenceState observe(std::uint8_t node_id, std::uint8_t sequence) noexcept;

    /**
     * @brief Forget one node's sequence history.
     * @param node_id Logical node identifier. Invalid IDs are ignored.
     */
    void reset(std::uint8_t node_id) noexcept;

    /** @brief Forget sequence history for every node. */
    void reset_all() noexcept;

private:
    std::array<std::uint8_t, kMaxNodes> last_sequence_{};
    std::array<bool, kMaxNodes> seen_{};
};

}  // namespace picomesh
