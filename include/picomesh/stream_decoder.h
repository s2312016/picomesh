#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "picomesh/frame.h"

namespace picomesh {

/** @brief Outcome after feeding one byte to @ref StreamDecoder. */
enum class StreamStatus {
    need_more,   ///< A candidate frame is incomplete.
    frame_ready, ///< One complete valid frame is available.
    frame_error, ///< A candidate frame failed strict validation.
    discarded,   ///< Noise was discarded before a magic byte.
};

/** @brief Result returned after one byte is consumed. */
struct StreamResult {
    StreamStatus status{StreamStatus::need_more}; ///< Decoder outcome.
    Frame frame{};                                ///< Valid only for `frame_ready`.
    DecodeError error{DecodeError::none};         ///< Failure reason for `frame_error`.
};

/**
 * @brief Convert a UART- or USB-like byte stream into complete frames.
 *
 * Noise before @ref kFrameMagic is discarded, malformed candidates report a
 * specific error, and storage remains bounded by @ref kMaxEncodedFrameSize.
 * The decoder performs no dynamic allocation.
 */
class StreamDecoder {
public:
    /**
     * @brief Consume one byte from a continuous stream.
     * @param byte Next received byte.
     * @return Current decoder outcome.
     */
    StreamResult feed(std::uint8_t byte) noexcept;

    /** @brief Discard the current candidate frame. */
    void reset() noexcept;

    /** @return Number of bytes retained for the current candidate frame. */
    std::size_t buffered_size() const noexcept { return size_; }

private:
    StreamResult fail(DecodeError error, std::uint8_t last_byte) noexcept;

    std::array<std::uint8_t, kMaxEncodedFrameSize> buffer_{};
    std::size_t size_{0};
    std::size_t expected_length_{0};
};

}  // namespace picomesh
