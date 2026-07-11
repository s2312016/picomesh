#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "picomesh/frame.h"

namespace picomesh {

enum class StreamStatus {
    need_more,
    frame_ready,
    frame_error,
    discarded,
};

struct StreamResult {
    StreamStatus status{StreamStatus::need_more};
    Frame frame{};
    DecodeError error{DecodeError::none};
};

// Converts UART/USB byte streams into complete PicoMesh frames. Noise before a
// magic byte is discarded and no dynamic memory is used.
class StreamDecoder {
public:
    StreamResult feed(std::uint8_t byte) noexcept;
    void reset() noexcept;
    std::size_t buffered_size() const noexcept { return size_; }

private:
    StreamResult fail(DecodeError error, std::uint8_t last_byte) noexcept;

    std::array<std::uint8_t, kMaxEncodedFrameSize> buffer_{};
    std::size_t size_{0};
    std::size_t expected_length_{0};
};

}  // namespace picomesh
