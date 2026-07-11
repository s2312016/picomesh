#include "picomesh/stream_decoder.h"

namespace picomesh {

void StreamDecoder::reset() noexcept {
    size_ = 0;
    expected_length_ = 0;
}

StreamResult StreamDecoder::fail(
    const DecodeError error,
    const std::uint8_t last_byte) noexcept {
    reset();
    if (last_byte == kFrameMagic) {
        buffer_[0] = last_byte;
        size_ = 1;
    }
    return {StreamStatus::frame_error, {}, error};
}

StreamResult StreamDecoder::feed(const std::uint8_t byte) noexcept {
    if (size_ == 0) {
        if (byte != kFrameMagic) {
            return {StreamStatus::discarded, {}, DecodeError::bad_magic};
        }
        buffer_[size_++] = byte;
        return {};
    }

    if (size_ >= buffer_.size()) {
        return fail(DecodeError::length_mismatch, byte);
    }
    buffer_[size_++] = byte;

    if (size_ == 2 && buffer_[1] != kProtocolVersion) {
        return fail(DecodeError::unsupported_version, byte);
    }

    if (size_ == kFrameHeaderSize) {
        const auto payload_length = static_cast<std::size_t>(buffer_[6]);
        if (payload_length > kMaxPayloadSize) {
            return fail(DecodeError::payload_too_large, byte);
        }
        expected_length_ = kFrameOverhead + payload_length;
    }

    if (expected_length_ != 0 && size_ == expected_length_) {
        const auto decoded = decode_frame(buffer_.data(), size_);
        reset();
        if (!decoded) {
            return {StreamStatus::frame_error, {}, decoded.error};
        }
        return {StreamStatus::frame_ready, decoded.frame, DecodeError::none};
    }

    return {};
}

}  // namespace picomesh
