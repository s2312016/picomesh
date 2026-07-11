#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "picomesh/node_registry.h"

namespace picomesh {

enum class SequenceState {
    first,
    newer,
    duplicate,
    stale,
    invalid_node,
};

class SequenceTracker {
public:
    SequenceState observe(std::uint8_t node_id, std::uint8_t sequence) noexcept;
    void reset(std::uint8_t node_id) noexcept;
    void reset_all() noexcept;

private:
    std::array<std::uint8_t, kMaxNodes> last_sequence_{};
    std::array<bool, kMaxNodes> seen_{};
};

}  // namespace picomesh
