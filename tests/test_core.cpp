#include "picomesh/checksum.h"
#include "picomesh/frame.h"
#include "picomesh/node_registry.h"
#include "picomesh/sequence_tracker.h"
#include "test_support.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace picomesh_test {
namespace {

picomesh::Frame sample_command() {
    picomesh::Frame frame;
    frame.type = picomesh::MessageType::command;
    frame.node_id = 7;
    frame.sequence = 42;
    frame.payload_length = 3;
    frame.payload[0] = 0x10;
    frame.payload[1] = 0x20;
    frame.payload[2] = 0x30;
    return frame;
}

} // namespace

void test_checksum() {
    std::array<std::uint8_t, 5> packet{1, 2, 3, 4, 0};
    packet.back() = picomesh::checksum8(packet.data(), packet.size() - 1);
    CHECK(picomesh::verify_checksum8(packet.data(), packet.size()));

    packet[1] ^= 0x01;
    CHECK(!picomesh::verify_checksum8(packet.data(), packet.size()));
    CHECK(picomesh::checksum8(nullptr, 10) == 0);
    CHECK(!picomesh::verify_checksum8(nullptr, 0));
    CHECK(!picomesh::verify_checksum8(packet.data(), 0));
}

void test_frame_round_trip() {
    const auto frame = sample_command();
    const auto encoded = picomesh::encode_frame(frame);
    const auto decoded = picomesh::decode_frame(encoded);
    CHECK(decoded);
    CHECK(decoded.frame.type == picomesh::MessageType::command);
    CHECK(decoded.frame.node_id == 7);
    CHECK(decoded.frame.sequence == 42);
    CHECK(decoded.frame.payload_length == 3);
    CHECK(decoded.frame.payload[0] == 0x10);
    CHECK(decoded.frame.payload[2] == 0x30);
}

void test_encode_clamps_payload_length() {
    picomesh::Frame frame;
    frame.payload_length = std::numeric_limits<std::uint8_t>::max();
    for (std::size_t i = 0; i < frame.payload.size(); ++i) {
        frame.payload[i] = static_cast<std::uint8_t>(i);
    }

    const auto encoded = picomesh::encode_frame(frame);
    CHECK(encoded.length == picomesh::kMaxEncodedFrameSize);
    CHECK(encoded.bytes[6] == picomesh::kMaxPayloadSize);

    const auto decoded = picomesh::decode_frame(encoded);
    CHECK(decoded);
    CHECK(decoded.frame.payload_length == picomesh::kMaxPayloadSize);
    CHECK(decoded.frame.payload.back() == picomesh::kMaxPayloadSize - 1);
}

void test_frame_length_from_padded_read() {
    const auto encoded = picomesh::encode_frame(sample_command());
    std::array<std::uint8_t, picomesh::kMaxEncodedFrameSize> padded{};
    for (std::size_t i = 0; i < encoded.length; ++i) {
        padded[i] = encoded.bytes[i];
    }

    const auto detected = picomesh::frame_length_from_prefix(padded.data(), padded.size());
    CHECK(detected == encoded.length);
    CHECK(picomesh::decode_frame(padded.data(), detected));
    CHECK(picomesh::frame_length_from_prefix(padded.data(), picomesh::kFrameHeaderSize) == 0);
    CHECK(picomesh::frame_length_from_prefix(nullptr, padded.size()) == 0);

    padded[0] = 0;
    CHECK(picomesh::frame_length_from_prefix(padded.data(), padded.size()) == 0);
    padded[0] = picomesh::kFrameMagic;
    padded[1] = static_cast<std::uint8_t>(picomesh::kProtocolVersion + 1);
    CHECK(picomesh::frame_length_from_prefix(padded.data(), padded.size()) == 0);
    padded[1] = picomesh::kProtocolVersion;
    padded[6] = static_cast<std::uint8_t>(picomesh::kMaxPayloadSize + 1);
    CHECK(picomesh::frame_length_from_prefix(padded.data(), padded.size()) == 0);
}

void test_decode_errors() {
    CHECK(picomesh::decode_frame(nullptr, 0).error == picomesh::DecodeError::too_short);

    auto encoded = picomesh::encode_frame(sample_command());

    auto damaged = encoded;
    damaged.bytes[0] = 0;
    CHECK(picomesh::decode_frame(damaged).error == picomesh::DecodeError::bad_magic);

    damaged = encoded;
    damaged.bytes[1] = static_cast<std::uint8_t>(picomesh::kProtocolVersion + 1);
    CHECK(picomesh::decode_frame(damaged).error == picomesh::DecodeError::unsupported_version);

    damaged = encoded;
    damaged.bytes[6] = static_cast<std::uint8_t>(picomesh::kMaxPayloadSize + 1);
    CHECK(picomesh::decode_frame(damaged).error == picomesh::DecodeError::payload_too_large);

    CHECK(picomesh::decode_frame(encoded.bytes.data(), encoded.length - 1).error ==
          picomesh::DecodeError::length_mismatch);

    damaged = encoded;
    damaged.bytes[2] ^= 0x01;
    CHECK(picomesh::decode_frame(damaged).error == picomesh::DecodeError::bad_checksum);
}

void test_sequence_tracker() {
    picomesh::SequenceTracker tracker;
    CHECK(tracker.observe(1, 254) == picomesh::SequenceState::first);
    CHECK(tracker.observe(1, 255) == picomesh::SequenceState::newer);
    CHECK(tracker.observe(1, 0) == picomesh::SequenceState::newer);
    CHECK(tracker.observe(1, 0) == picomesh::SequenceState::duplicate);
    CHECK(tracker.observe(1, 250) == picomesh::SequenceState::stale);
    CHECK(tracker.observe(40, 1) == picomesh::SequenceState::invalid_node);

    tracker.reset(1);
    CHECK(tracker.observe(1, 17) == picomesh::SequenceState::first);
    CHECK(tracker.observe(1, 144) == picomesh::SequenceState::newer);
    CHECK(tracker.observe(1, 16) == picomesh::SequenceState::stale);

    CHECK(tracker.observe(2, 5) == picomesh::SequenceState::first);
    tracker.reset_all();
    CHECK(tracker.observe(1, 16) == picomesh::SequenceState::first);
    CHECK(tracker.observe(2, 5) == picomesh::SequenceState::first);
}

void test_registry_timeout() {
    picomesh::NodeRegistry registry(1000);
    CHECK(!registry.get(2).has_value());
    CHECK(!registry.observe(static_cast<std::uint8_t>(picomesh::kMaxNodes), 1, 0));
    CHECK(registry.observe(2, 1, 500));
    CHECK(registry.online_count() == 1);

    registry.refresh(1200);
    CHECK(registry.get(2)->online);
    registry.refresh(1501);
    CHECK(!registry.get(2)->online);
    CHECK(registry.online_count() == 0);

    CHECK(registry.observe(2, 2, 1600));
    CHECK(registry.get(2)->online);
    CHECK(registry.get(2)->last_sequence == 2);
    CHECK(registry.online_count() == 1);
}

void test_registry_timer_wraparound() {
    picomesh::NodeRegistry registry(100);
    constexpr std::uint32_t near_wrap = std::numeric_limits<std::uint32_t>::max() - 50u;
    CHECK(registry.observe(3, 9, near_wrap));
    registry.refresh(25);
    CHECK(registry.get(3)->online);
    registry.refresh(60);
    CHECK(!registry.get(3)->online);
}

} // namespace picomesh_test
