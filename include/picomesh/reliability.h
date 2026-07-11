#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "picomesh/frame.h"

namespace picomesh {

/** Flag indicating that the receiver should publish an acknowledgement. */
constexpr std::uint8_t kFlagAckRequired = 0x01;
/** Maximum number of simultaneously pending reliable transmissions. */
constexpr std::size_t kMaxPendingTransmissions = 8;

/** @brief Receiver result carried by an acknowledgement frame. */
enum class AckStatus : std::uint8_t {
    accepted = 0,    ///< Command was accepted.
    rejected = 1,    ///< Command was rejected by application policy.
    busy = 2,        ///< Receiver is temporarily unable to process it.
    unsupported = 3, ///< Command or capability is unsupported.
    malformed = 4,   ///< Command payload was malformed.
};

/**
 * @brief Construct a two-byte acknowledgement frame.
 * @param node_id Logical node publishing the acknowledgement.
 * @param response_sequence Sequence assigned to the acknowledgement itself.
 * @param acknowledged_sequence Sequence of the frame being acknowledged.
 * @param status Receiver result.
 * @return Acknowledgement frame ready for encoding.
 */
Frame make_ack_frame(
    std::uint8_t node_id,
    std::uint8_t response_sequence,
    std::uint8_t acknowledged_sequence,
    AckStatus status) noexcept;

/**
 * @brief Parse and validate an acknowledgement payload.
 * @param frame Candidate acknowledgement frame.
 * @param acknowledged_sequence Receives the acknowledged sequence on success.
 * @param status Receives the acknowledgement status on success.
 * @return `true` when type, payload length, and status value are valid.
 */
bool parse_ack_frame(
    const Frame& frame,
    std::uint8_t& acknowledged_sequence,
    AckStatus& status) noexcept;

/** @brief Retry timing and attempt limit. */
struct RetryPolicy {
    std::uint32_t timeout_ms{250};  ///< Delay between transmission attempts.
    std::uint8_t max_attempts{3};   ///< Total sends before exhaustion.
};

/** @brief Result of adding a frame to @ref ReliableQueue. */
enum class QueueResult {
    queued,  ///< Frame was stored.
    full,    ///< Every fixed-capacity slot is active.
    invalid, ///< Frame is invalid or duplicates an active node/sequence pair.
};

/** @brief Action returned by @ref ReliableQueue::next_due. */
enum class TxAction {
    none,      ///< No frame is due.
    send,      ///< Send the returned frame now.
    exhausted, ///< Attempts are exhausted and the slot was removed.
};

/** @brief One scheduler decision from the reliable queue. */
struct TxDecision {
    TxAction action{TxAction::none}; ///< Required action.
    Frame frame{};                   ///< Frame associated with the decision.
    std::uint8_t attempts{0};        ///< Attempts made including this decision.
};

/** @brief Result of resolving a pending frame with an acknowledgement. */
enum class AckResolution {
    not_found, ///< No active frame matched node and sequence.
    accepted,  ///< Matching frame was removed with accepted status.
    rejected,  ///< Matching frame was removed with a non-accepted status.
};

/**
 * @brief Deterministic retry queue for ACK-required frames.
 *
 * The queue performs no heap allocation, keeps at most
 * @ref kMaxPendingTransmissions frames, and uses wrap-safe 32-bit millisecond
 * deadline arithmetic. A frame is keyed by its node ID and sequence.
 */
class ReliableQueue {
public:
    /**
     * @brief Construct a retry queue.
     * @param policy Retry timeout and total attempt limit. A zero attempt limit
     * is normalized to one.
     */
    explicit ReliableQueue(RetryPolicy policy = {}) noexcept;

    /**
     * @brief Add a frame and mark it ACK-required.
     * @param frame Frame to retain until acknowledgement or exhaustion.
     * @param now_ms Current modulo-2^32 millisecond tick.
     * @return Queue result.
     */
    QueueResult enqueue(const Frame& frame, std::uint32_t now_ms) noexcept;

    /**
     * @brief Return the first transmission whose deadline is due.
     * @param now_ms Current modulo-2^32 millisecond tick.
     * @return Send, exhaustion, or no-action decision.
     */
    TxDecision next_due(std::uint32_t now_ms) noexcept;

    /**
     * @brief Resolve and remove a matching pending frame.
     * @param node_id Node ID from the original frame.
     * @param sequence Sequence from the original frame.
     * @param status Receiver result.
     * @return Resolution result.
     */
    AckResolution acknowledge(
        std::uint8_t node_id,
        std::uint8_t sequence,
        AckStatus status) noexcept;

    /**
     * @brief Cancel one matching pending frame.
     * @return `true` when an active frame was removed.
     */
    bool cancel(std::uint8_t node_id, std::uint8_t sequence) noexcept;

    /** @return Number of active fixed-capacity slots. */
    std::size_t pending_count() const noexcept;

    /** @brief Remove every pending frame and reset slot state. */
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
