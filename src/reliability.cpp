#include "picomesh/reliability.h"

namespace picomesh {
namespace {

bool deadline_reached(const std::uint32_t now_ms, const std::uint32_t deadline_ms) noexcept {
    return static_cast<std::int32_t>(now_ms - deadline_ms) >= 0;
}

} // namespace

Frame make_ack_frame(const std::uint8_t node_id, const std::uint8_t response_sequence,
                     const std::uint8_t acknowledged_sequence, const AckStatus status) noexcept {
    Frame frame;
    frame.type = MessageType::acknowledgement;
    frame.node_id = node_id;
    frame.sequence = response_sequence;
    frame.payload_length = 2;
    frame.payload[0] = acknowledged_sequence;
    frame.payload[1] = static_cast<std::uint8_t>(status);
    return frame;
}

bool parse_ack_frame(const Frame& frame, std::uint8_t& acknowledged_sequence,
                     AckStatus& status) noexcept {
    if (frame.type != MessageType::acknowledgement || frame.payload_length != 2) {
        return false;
    }

    const auto raw_status = frame.payload[1];
    if (raw_status > static_cast<std::uint8_t>(AckStatus::malformed)) {
        return false;
    }

    acknowledged_sequence = frame.payload[0];
    status = static_cast<AckStatus>(raw_status);
    return true;
}

ReliableQueue::ReliableQueue(RetryPolicy policy) noexcept : policy_(policy) {
    if (policy_.max_attempts == 0) {
        policy_.max_attempts = 1;
    }
}

QueueResult ReliableQueue::enqueue(const Frame& input, const std::uint32_t now_ms) noexcept {
    if (input.payload_length > kMaxPayloadSize) {
        return QueueResult::invalid;
    }

    for (const auto& slot : slots_) {
        if (slot.active && slot.frame.node_id == input.node_id &&
            slot.frame.sequence == input.sequence) {
            return QueueResult::invalid;
        }
    }

    for (auto& slot : slots_) {
        if (!slot.active) {
            slot.frame = input;
            slot.frame.flags = static_cast<std::uint8_t>(slot.frame.flags | kFlagAckRequired);
            slot.deadline_ms = now_ms;
            slot.attempts = 0;
            slot.active = true;
            return QueueResult::queued;
        }
    }

    return QueueResult::full;
}

TxDecision ReliableQueue::next_due(const std::uint32_t now_ms) noexcept {
    for (auto& slot : slots_) {
        if (!slot.active || !deadline_reached(now_ms, slot.deadline_ms)) {
            continue;
        }

        if (slot.attempts >= policy_.max_attempts) {
            const TxDecision decision{TxAction::exhausted, slot.frame, slot.attempts};
            slot.active = false;
            return decision;
        }

        ++slot.attempts;
        slot.deadline_ms = static_cast<std::uint32_t>(now_ms + policy_.timeout_ms);
        return {TxAction::send, slot.frame, slot.attempts};
    }

    return {};
}

AckResolution ReliableQueue::acknowledge(const std::uint8_t node_id, const std::uint8_t sequence,
                                         const AckStatus status) noexcept {
    for (auto& slot : slots_) {
        if (slot.active && slot.frame.node_id == node_id && slot.frame.sequence == sequence) {
            slot.active = false;
            return status == AckStatus::accepted ? AckResolution::accepted
                                                 : AckResolution::rejected;
        }
    }
    return AckResolution::not_found;
}

bool ReliableQueue::cancel(const std::uint8_t node_id, const std::uint8_t sequence) noexcept {
    for (auto& slot : slots_) {
        if (slot.active && slot.frame.node_id == node_id && slot.frame.sequence == sequence) {
            slot.active = false;
            return true;
        }
    }
    return false;
}

std::size_t ReliableQueue::pending_count() const noexcept {
    std::size_t count = 0;
    for (const auto& slot : slots_) {
        if (slot.active) {
            ++count;
        }
    }
    return count;
}

void ReliableQueue::clear() noexcept {
    for (auto& slot : slots_) {
        slot = Slot{};
    }
}

} // namespace picomesh
