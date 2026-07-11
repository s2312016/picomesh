#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "picomesh/frame.h"

namespace picomesh {

constexpr std::uint8_t kFlagAckRequired = 0x01;
constexpr std::size_t kMaxPendingTransmissions = 8;

enum class AckStatus : std::uint8_t {
    accepted = 0,
    rejected = 1,
    busy = 2,
    unsupported = 3,
    malformed = 4,
};

Frame make_ack_frame(
    std::uint8_t node_id,
    std::uint8_t response_sequence,
    std::uint8_t acknowledged_sequence,
    AckStatus status) noexcept;

bool parse_ack_frame(
    const Frame& frame,
    std::uint8_t& acknowledged_sequence,
    AckStatus& status) noexcept;

struct RetryPolicy {
    std::uint32_t timeout_ms{250};
    std::uint8_t max_attempts{3};
};

enum class QueueResult {
    queued,
    full,
    invalid,
};

enum class TxAction {
    none,
    send,
    exhausted,
};

struct TxDecision {
    TxAction action{TxAction::none};
    Frame frame{};
    std::uint8_t attempts{0};
};

enum class AckResolution {
    not_found,
    accepted,
    rejected,
};

// Deterministic retry queue for commands that require acknowledgement.
// It contains no heap allocation and keeps at most eight in-flight frames.
// Deadlines use wrap-safe 32-bit millisecond arithmetic.
class ReliableQueue {
public:
    explicit ReliableQueue(RetryPolicy policy = {}) noexcept;

    QueueResult enqueue(const Frame& frame, std::uint32_t now_ms) noexcept;
    TxDecision next_due(std::uint32_t now_ms) noexcept;
    AckResolution acknowledge(
        std::uint8_t node_id,
        std::uint8_t sequence,
        AckStatus status) noexcept;
    bool cancel(std::uint8_t node_id, std::uint8_t sequence) noexcept;
    std::size_t pending_count() const noexcept;
    void clear() noexcept;

private:
    struct Slot {
        Frame frame{};
        std::uint32_t deadline_ms{0};
        std::uint8_t attempts{0};
        bool active{false};
    };

    std::array<Slot, kMaxPendingTransmissions> slots_{};
    RetryPolicy policy_{};
};

}  // namespace picomesh
