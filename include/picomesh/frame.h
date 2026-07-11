#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace picomesh {

/** Protocol start byte used to locate a frame. */
constexpr std::uint8_t kFrameMagic = 0xB7;
/** Current wire-protocol version encoded in every frame. */
constexpr std::uint8_t kProtocolVersion = 1;
/** Maximum number of application payload bytes in one frame. */
constexpr std::size_t kMaxPayloadSize = 32;
/** Number of bytes before the application payload. */
constexpr std::size_t kFrameHeaderSize = 7;
/** Header plus trailing checksum size. */
constexpr std::size_t kFrameOverhead = kFrameHeaderSize + 1;
/** Largest possible encoded frame size. */
constexpr std::size_t kMaxEncodedFrameSize = kFrameOverhead + kMaxPayloadSize;

/** @brief Message category encoded in a frame. */
enum class MessageType : std::uint8_t {
    heartbeat = 0x01,        ///< Periodic node-liveness announcement.
    state = 0x02,            ///< Application state publication.
    command = 0x10,          ///< Application command.
    acknowledgement = 0x11,  ///< Response to an ACK-required frame.
    user = 0x80,             ///< First value reserved for application-defined types.
};

/**
 * @brief Decoded, fixed-capacity PicoMesh frame.
 *
 * Only the first `payload_length` bytes of `payload` are part of the message.
 * Encoding clamps an oversized payload length to `kMaxPayloadSize`.
 */
struct Frame {
    MessageType type{MessageType::heartbeat};  ///< Message category.
    std::uint8_t flags{0};                     ///< Protocol flags such as ACK-required.
    std::uint8_t node_id{0};                   ///< Logical node identifier.
    std::uint8_t sequence{0};                  ///< Modulo-256 message sequence.
    std::array<std::uint8_t, kMaxPayloadSize> payload{};  ///< Fixed payload storage.
    std::uint8_t payload_length{0};             ///< Number of active payload bytes.
};

/** @brief Encoded frame bytes and their active length. */
struct EncodedFrame {
    std::array<std::uint8_t, kMaxEncodedFrameSize> bytes{};  ///< Encoded storage.
    std::size_t length{0};  ///< Number of valid encoded bytes.
};

/** @brief Reason strict frame decoding failed. */
enum class DecodeError {
    none,                 ///< No error.
    too_short,            ///< Buffer cannot contain the minimum frame.
    bad_magic,            ///< Start byte does not match `kFrameMagic`.
    unsupported_version,  ///< Wire version is not supported.
    payload_too_large,    ///< Header declares more than `kMaxPayloadSize` bytes.
    length_mismatch,      ///< Buffer length does not match the header.
    bad_checksum,         ///< Integrity check failed.
};

/** @brief Result of strict frame decoding. */
struct DecodeResult {
    Frame frame{};                          ///< Decoded frame when successful.
    DecodeError error{DecodeError::none};  ///< Error code or `DecodeError::none`.

    /** @return `true` when decoding succeeded. */
    explicit operator bool() const noexcept { return error == DecodeError::none; }
};

/**
 * @brief Determine a complete encoded length from a valid header prefix.
 *
 * This is useful for fixed-size transports that pad a short frame with bytes
 * after the checksum.
 *
 * @param data Candidate frame prefix.
 * @param available_length Number of available bytes.
 * @return Complete encoded length, or zero for an incomplete or invalid prefix.
 */
std::size_t frame_length_from_prefix(
    const std::uint8_t* data,
    std::size_t available_length) noexcept;

/**
 * @brief Encode a frame into fixed-capacity storage.
 * @param frame Frame to encode.
 * @return Encoded bytes and active length.
 */
EncodedFrame encode_frame(const Frame& frame) noexcept;

/**
 * @brief Strictly decode one exact frame buffer.
 * @param data Encoded bytes.
 * @param length Exact encoded length, including checksum.
 * @return Decoded frame or a specific error.
 */
DecodeResult decode_frame(const std::uint8_t* data, std::size_t length) noexcept;

/**
 * @brief Strictly decode an EncodedFrame.
 * @param data Encoded frame object.
 * @return Decoded frame or a specific error.
 */
DecodeResult decode_frame(const EncodedFrame& data) noexcept;

}  // namespace picomesh
