#include "picomesh/sequence_tracker.h"

namespace picomesh {

SequenceState SequenceTracker::observe(const std::uint8_t node_id,
                                       const std::uint8_t sequence) noexcept {
    if (node_id >= kMaxNodes) {
        return SequenceState::invalid_node;
    }

    if (!seen_[node_id]) {
        seen_[node_id] = true;
        last_sequence_[node_id] = sequence;
        return SequenceState::first;
    }

    const std::uint8_t delta = static_cast<std::uint8_t>(sequence - last_sequence_[node_id]);
    if (delta == 0) {
        return SequenceState::duplicate;
    }

    // Distances 1..127 are newer and 128..255 are stale. This permits
    // normal wraparound from 255 to 0 while rejecting delayed packets.
    if (delta < 128) {
        last_sequence_[node_id] = sequence;
        return SequenceState::newer;
    }

    return SequenceState::stale;
}

void SequenceTracker::reset(const std::uint8_t node_id) noexcept {
    if (node_id < kMaxNodes) {
        seen_[node_id] = false;
        last_sequence_[node_id] = 0;
    }
}

void SequenceTracker::reset_all() noexcept {
    seen_.fill(false);
    last_sequence_.fill(0);
}

} // namespace picomesh
