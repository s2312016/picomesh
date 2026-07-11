#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>

#include "picomesh/frame.h"
#include "picomesh/stream_decoder.h"

namespace {

picomesh::StreamResult feed(
    picomesh::StreamDecoder& decoder,
    const std::uint8_t* bytes,
    const std::size_t length) {
    picomesh::StreamResult result;
    for (std::size_t i = 0; i < length; ++i) {
        result = decoder.feed(bytes[i]);
    }
    return result;
}

bool expect(
    const picomesh::StreamResult& result,
    const picomesh::StreamStatus status,
    const picomesh::DecodeError error,
    const char* scenario) {
    if (result.status == status && result.error == error) {
        std::cout << scenario << ": expected outcome\n";
        return true;
    }

    std::cerr << scenario << ": unexpected status="
              << static_cast<unsigned>(result.status)
              << " error=" << static_cast<unsigned>(result.error) << '\n';
    return false;
}

}  // namespace

int main() {
    picomesh::Frame frame;
    frame.type = picomesh::MessageType::state;
    frame.node_id = 5;
    frame.sequence = 23;
    frame.payload_length = 2;
    frame.payload[0] = 0x42;
    frame.payload[1] = 0x99;

    const auto valid = picomesh::encode_frame(frame);
    picomesh::StreamDecoder decoder;
    unsigned ready_count = 0;
    unsigned error_count = 0;

    const auto noise = decoder.feed(0x00);
    if (!expect(
            noise,
            picomesh::StreamStatus::discarded,
            picomesh::DecodeError::bad_magic,
            "leading noise")) {
        return 1;
    }

    auto result = feed(decoder, valid.bytes.data(), valid.length);
    if (!expect(
            result,
            picomesh::StreamStatus::frame_ready,
            picomesh::DecodeError::none,
            "valid frame after noise")) {
        return 2;
    }
    ++ready_count;

    auto bad_checksum = valid;
    bad_checksum.bytes[bad_checksum.length - 1] ^= 0x01;
    result = feed(decoder, bad_checksum.bytes.data(), bad_checksum.length);
    if (!expect(
            result,
            picomesh::StreamStatus::frame_error,
            picomesh::DecodeError::bad_checksum,
            "checksum corruption")) {
        return 3;
    }
    ++error_count;

    result = feed(decoder, valid.bytes.data(), valid.length);
    if (!expect(
            result,
            picomesh::StreamStatus::frame_ready,
            picomesh::DecodeError::none,
            "recovery after checksum error")) {
        return 4;
    }
    ++ready_count;

    const std::array<std::uint8_t, 2> unsupported_version{
        picomesh::kFrameMagic,
        static_cast<std::uint8_t>(picomesh::kProtocolVersion + 1),
    };
    result = feed(decoder, unsupported_version.data(), unsupported_version.size());
    if (!expect(
            result,
            picomesh::StreamStatus::frame_error,
            picomesh::DecodeError::unsupported_version,
            "unsupported protocol version")) {
        return 5;
    }
    ++error_count;

    result = feed(decoder, valid.bytes.data(), valid.length);
    if (!expect(
            result,
            picomesh::StreamStatus::frame_ready,
            picomesh::DecodeError::none,
            "recovery after version error")) {
        return 6;
    }
    ++ready_count;

    const std::array<std::uint8_t, picomesh::kFrameHeaderSize> oversized_payload{
        picomesh::kFrameMagic,
        picomesh::kProtocolVersion,
        static_cast<std::uint8_t>(picomesh::MessageType::state),
        0,
        5,
        24,
        static_cast<std::uint8_t>(picomesh::kMaxPayloadSize + 1),
    };
    result = feed(decoder, oversized_payload.data(), oversized_payload.size());
    if (!expect(
            result,
            picomesh::StreamStatus::frame_error,
            picomesh::DecodeError::payload_too_large,
            "oversized payload declaration")) {
        return 7;
    }
    ++error_count;

    result = feed(decoder, valid.bytes.data(), valid.length);
    if (!expect(
            result,
            picomesh::StreamStatus::frame_ready,
            picomesh::DecodeError::none,
            "recovery after oversized declaration")) {
        return 8;
    }
    ++ready_count;

    std::cout << "Summary: ready=" << ready_count
              << " errors=" << error_count
              << " discarded=1\n";
    return ready_count == 4 && error_count == 3 ? 0 : 9;
}
