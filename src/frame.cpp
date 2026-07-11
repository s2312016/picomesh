#include "picomesh/frame.h"

#include <algorithm>

#include "picomesh/checksum.h"

namespace picomesh {

std::vector<std::uint8_t> encode_frame(const Frame& frame) {
    const auto payload_length = static_cast<std::size_t>(
        std::min<std::uint8_t>(frame.payload_length, static_cast<std::uint8_t>(kMaxPayloadSize)));

    std::vector<std::uint8_t> encoded;
    encoded.reserve(kFrameOverhead + payload_length);
    encoded.push_back(kFrameMagic);
    encoded.push_back(kProtocolVersion);
    encoded.push_back(static_cast<std::uint8_t>(frame.type));
    encoded.push_back(frame.flags);
    encoded.push_back(frame.node_id);
    encoded.push_back(frame.sequence);
    encoded.push_back(static_cast<std::uint8_t>(payload_length));
    encoded.insert(encoded.end(), frame.payload.begin(), frame.payload.begin() + payload_length);
    encoded.push_back(checksum8(encoded.data(), encoded.size()));
    return encoded;
}

DecodeResult decode_frame(const std::uint8_t* data, const std::size_t length) noexcept {
    if (data == nullptr || length < kFrameOverhead) {
        return {std::nullopt, DecodeError::too_short};
    }
    if (data[0] != kFrameMagic) {
        return {std::nullopt, DecodeError::bad_magic};
    }
    if (data[1] != kProtocolVersion) {
        return {std::nullopt, DecodeError::unsupported_version};
    }

    const auto payload_length = static_cast<std::size_t>(data[6]);
    if (payload_length > kMaxPayloadSize) {
        return {std::nullopt, DecodeError::payload_too_large};
    }
    if (length != kFrameOverhead + payload_length) {
        return {std::nullopt, DecodeError::length_mismatch};
    }
    if (!verify_checksum8(data, length)) {
        return {std::nullopt, DecodeError::bad_checksum};
    }

    Frame frame;
    frame.type = static_cast<MessageType>(data[2]);
    frame.flags = data[3];
    frame.node_id = data[4];
    frame.sequence = data[5];
    frame.payload_length = static_cast<std::uint8_t>(payload_length);
    std::copy_n(data + kFrameHeaderSize, payload_length, frame.payload.begin());
    return {frame, DecodeError::none};
}

DecodeResult decode_frame(const std::vector<std::uint8_t>& data) noexcept {
    return decode_frame(data.data(), data.size());
}

}  // namespace picomesh
