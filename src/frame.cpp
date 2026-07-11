#include "picomesh/frame.h"

#include <algorithm>

#include "picomesh/checksum.h"

namespace picomesh {

EncodedFrame encode_frame(const Frame& frame) noexcept {
    const auto payload_length = static_cast<std::size_t>(
        std::min<std::uint8_t>(frame.payload_length, static_cast<std::uint8_t>(kMaxPayloadSize)));

    EncodedFrame encoded;
    encoded.bytes[0] = kFrameMagic;
    encoded.bytes[1] = kProtocolVersion;
    encoded.bytes[2] = static_cast<std::uint8_t>(frame.type);
    encoded.bytes[3] = frame.flags;
    encoded.bytes[4] = frame.node_id;
    encoded.bytes[5] = frame.sequence;
    encoded.bytes[6] = static_cast<std::uint8_t>(payload_length);
    std::copy_n(frame.payload.begin(), payload_length, encoded.bytes.begin() + kFrameHeaderSize);
    encoded.length = kFrameOverhead + payload_length;
    encoded.bytes[encoded.length - 1] = checksum8(encoded.bytes.data(), encoded.length - 1);
    return encoded;
}

DecodeResult decode_frame(const std::uint8_t* data, const std::size_t length) noexcept {
    if (data == nullptr || length < kFrameOverhead) {
        return {{}, DecodeError::too_short};
    }
    if (data[0] != kFrameMagic) {
        return {{}, DecodeError::bad_magic};
    }
    if (data[1] != kProtocolVersion) {
        return {{}, DecodeError::unsupported_version};
    }

    const auto payload_length = static_cast<std::size_t>(data[6]);
    if (payload_length > kMaxPayloadSize) {
        return {{}, DecodeError::payload_too_large};
    }
    if (length != kFrameOverhead + payload_length) {
        return {{}, DecodeError::length_mismatch};
    }
    if (!verify_checksum8(data, length)) {
        return {{}, DecodeError::bad_checksum};
    }

    DecodeResult result;
    result.frame.type = static_cast<MessageType>(data[2]);
    result.frame.flags = data[3];
    result.frame.node_id = data[4];
    result.frame.sequence = data[5];
    result.frame.payload_length = static_cast<std::uint8_t>(payload_length);
    std::copy_n(data + kFrameHeaderSize, payload_length, result.frame.payload.begin());
    return result;
}

DecodeResult decode_frame(const EncodedFrame& data) noexcept {
    return decode_frame(data.bytes.data(), data.length);
}

}  // namespace picomesh
