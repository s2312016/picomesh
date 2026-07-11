#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace picomesh {

constexpr std::uint8_t kFrameMagic = 0xB7;
constexpr std::uint8_t kProtocolVersion = 1;
constexpr std::size_t kMaxPayloadSize = 32;
constexpr std::size_t kFrameHeaderSize = 7;
constexpr std::size_t kFrameOverhead = kFrameHeaderSize + 1;
constexpr std::size_t kMaxEncodedFrameSize = kFrameOverhead + kMaxPayloadSize;

enum class MessageType : std::uint8_t {
    heartbeat = 0x01,
    state = 0x02,
    command = 0x10,
    acknowledgement = 0x11,
    user = 0x80,
};

struct Frame {
    MessageType type{MessageType::heartbeat};
    std::uint8_t flags{0};
    std::uint8_t node_id{0};
    std::uint8_t sequence{0};
    std::array<std::uint8_t, kMaxPayloadSize> payload{};
    std::uint8_t payload_length{0};
};

struct EncodedFrame {
    std::array<std::uint8_t, kMaxEncodedFrameSize> bytes{};
    std::size_t length{0};
};

enum class DecodeError {
    none,
    too_short,
    bad_magic,
    unsupported_version,
    payload_too_large,
    length_mismatch,
    bad_checksum,
};

struct DecodeResult {
    Frame frame{};
    DecodeError error{DecodeError::none};

    explicit operator bool() const noexcept { return error == DecodeError::none; }
};

// Returns the complete encoded frame length described by a valid header prefix.
// This is useful for transports that return a fixed-size read padded with bytes
// after the frame. A return value of zero means the prefix is incomplete or
// invalid.
std::size_t frame_length_from_prefix(
    const std::uint8_t* data,
    std::size_t available_length) noexcept;

EncodedFrame encode_frame(const Frame& frame) noexcept;
DecodeResult decode_frame(const std::uint8_t* data, std::size_t length) noexcept;
DecodeResult decode_frame(const EncodedFrame& data) noexcept;

}  // namespace picomesh
