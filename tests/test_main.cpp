#include "picomesh/checksum.h"
#include "picomesh/frame.h"
#include "picomesh/node_registry.h"
#include "picomesh/reliability.h"
#include "picomesh/sequence_tracker.h"
#include "picomesh/stream_decoder.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace {

[[noreturn]] void fail_check(const char* expression, const char* file, const int line) {
    std::cerr << file << ':' << line << ": check failed: " << expression << '\n';
    std::exit(1);
}

#define CHECK(expression)                                                                          \
    do {                                                                                           \
        if (!(expression)) {                                                                       \
            fail_check(#expression, __FILE__, __LINE__);                                           \
        }                                                                                          \
    } while (false)

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

void test_acknowledgement_payload() {
    const auto acknowledgement = picomesh::make_ack_frame(7, 43, 42, picomesh::AckStatus::accepted);

    std::uint8_t acknowledged_sequence = 0;
    picomesh::AckStatus status = picomesh::AckStatus::malformed;
    CHECK(picomesh::parse_ack_frame(acknowledgement, acknowledged_sequence, status));
    CHECK(acknowledged_sequence == 42);
    CHECK(status == picomesh::AckStatus::accepted);

    auto malformed = acknowledgement;
    malformed.type = picomesh::MessageType::command;
    CHECK(!picomesh::parse_ack_frame(malformed, acknowleded_sequence, status));

    malformed = acknowledgement;
    malformed.payload_length = 1;
    CHECK(!picomesh::parse_ack_frame(malformed, acknowledged_sequence, status));

    malformed = acknowledgement;
    malformed.payload[1] = static_cast<std::uint8_t>(picomesh::AckStatus::malformed) + 1;
    CHECK(!picomesh::parse_ack_frame(malformed, acknowledged_sequence, status));
}

void test_reliable_queue() {
    picomesh::ReliableQueue queue({100, 2});
    const auto command = sample_command();

    CHECK(queue.enqueue(command, 1000) == picomesh::QueueResult::queued);
    CHECK(queue.enqueue(command, 1000) == picomesh::QueueResult::invalid);
    CHECK(queue.pending_count() == 1);

    auto decision = queue.next_due(1000);
    CHECK(decision.action == picomesh::TxAction::send);
    CHECK(decision.attempts == 1);
    CHECK((decision.frame.flags & picomesh::kFlagAckRequired) != 0);

    CHECK(queue.next_due(1099).action == picomesh::TxAction::none);
    decision = queue.next_due(1100);
    CHECK(decision.action == picomesh::TxAction::send);
    CHECK(decision.attempts == 2);

    decision = queue.next_due(1200);
    CHECK(decision.action == picomesh::TxAction::exhausted);
    CHECK(queue.pending_count() == 0);

    CHECK(queue.enqueue(command, 2000) == picomesh::QueueResult::queued);
    CHECK(queue.acknowledge(7, 42, picomesh::AckStatus::accepted) ==
          picomesh::AckResolution::accepted);
    CHECK(queue.pending_count() == 0);

    CHECK(queue.acknowledge(7, 42, picomesh::AckStatus::accepted) ==
          picomesh::AckResolution::not_found);
    CHECK(queue.enqueue(command, 3000) == picomesh::QueueResult::queued);
    CHECK(queue.acknowledge(7, 42, picomesh::AckStatus::busy) == picomesh::AckResolution::rejected);
}

void test_reliable_queue_capacity_and_control() {
    picomesh::ReliableQueue queue;
    for (std::size_t i = 0; i < picomesh::kMaxPendingTransmissions; ++i) {
        auto frame = sample_command();
        frame.node_id = static_cast<std::uint8_t>(i);
        frame.sequence = static_cast<std::uint8_t>(i);
        CHECK(queue.enqueue(frame, 0) == picomesh::QueueResult::queued);
    }
    CHECK(queue.pending_count() == picomesh::kMaxPendingTransmissions);

    auto extra = sample_command();
    extra.node_id = static_cast<std::uint8_t>(picomesh::kMaxPendingTransmissions);
    extra.sequence = 99;
    CHECK(queue.enqueue(extra, 0) == picomesh::QueueResult::full);

    CHECK(queue.cancel(3, 3));
    CHECK(!queue.cancel(3, 3));
    CHECK(queue.pending_count() == picomesh::kMaxPendingTransmissions - 1);

    queue.clear();
    CHECK(queue.pending_count() == 0);
    CHECK(queue.next_due(0).action == picomesh::TxAction::none);

    picomesh::ReliableQueue one_attempt({50, 0});
    CHECK(one_attempt.enqueue(sample_command(), 10) == picomesh::QueueResult::queued);
    CHECK(one_attempt.next_due(10).action == picomesh::TxAction::send);
    CHECK(one_attempt.next_due(60).action == picomesh::TxAction::exhausted);
}

void test_retry_timer_wraparound() {
    picomesh::ReliableQueue queue({100, 2});
    const auto command = sample_command();
    constexpr std::uint32_t near_wrap = std::numeric_limits<std::uint32_t>::max() - 50u;

    CHECK(queue.enqueue(command, near_wrap) == picomesh::QueueResult::queued);
    CHECK(queue.next_due(near_wrap).action == picomesh::TxAction::send);
    CHECK(queue.next_due(48).action == picomesh::TxAction::none);
    CHECK(queue.next_due(49).action == picomesh::TxAction::send);
}

void test_stream_decoder() {
    const auto encoded = picomesh::encode_frame(sample_command());
    picomesh::StreamDecoder decoder;

    CHECK(decoder.feed(0x00).status == picomesh::StreamStatus::discarded);

    picomesh::StreamResult result;
    for (std::size_t i = 0; i < encoded.length; ++i) {
        result = decoder.feed(encoded.bytes[i]);
    }

    CHECK(result.status == picomesh::StreamStatus::frame_ready);
    CHECK(result.frame.node_id == 7);
    CHECK(result.frame.sequence == 42);
    CHECK(decoder.buffered_size() == 0);
}

void test_stream_decoder_errors_and_recovery() {
    picomesh::StreamDecoder decoder;
    CHECK(decoder.feed(picomesh::kFrameMagic).status == picomesh::StreamStatus::need_more);
    auto result = decoder.feed(static_cast<std::uint8_t>(picomesh::kProtocolVersion + 1));
    CHECK(result.status == picomesh::StreamStatus::frame_error);
    CHECK(result.error == picomesh::DecodeError::unsupported_version);
    CHECK(decoder.buffered_size() == 0);

    const std::array<std::uint8_t, picomesh::kFrameHeaderSize> oversized_header{
        picomesh::kFrameMagic,
        picomesh::kProtocolVersion,
        static_cast<std::uint8_t>(picomesh::MessageType::state),
        0,
        1,
        2,
        static_cast<std::uint8_t>(picomesh::kMaxPayloadSize + 1),
    };
    for (const auto byte : oversized_header) {
        result = decoder.feed(byte);
    }
    CHECK(result.status == picomesh::StreamStatus::frame_error);
    CHECK(result.error == picomesh::DecodeError::payload_too_large);
    CHECK(decoder.buffered_size() == 0);

    auto damaged = picomesh::encode_frame(sample_command());
    damaged.bytes[damaged.length - 1] ^= 0x01;
    for (std::size_t i = 0; i < damaged.length; ++i) {
        result = decoder.feed(damaged.bytes[i]);
    }
    CHECK(result.status == picomesh::StreamStatus::frame_error);
    CHECK(result.error == picomesh::DecodeError::bad_checksum);
    CHECK(decoder.buffered_size() == 0);

    const auto valid = picomesh::encode_frame(sample_command());
    CHECK(decoder.feed(0xaa).status == picomesh::StreamStatus::discarded);
    for (std::size_t i = 0; i < valid.length; ++i) {
        result = decoder.feed(valid.bytes[i]);
    }
    CHECK(result.status == picomesh::StreamStatus::frame_ready);
    CHECK(result.frame.sequence == sample_command().sequence);

    CHECK(decoder.feed(picomesh::kFrameMagic).status == picomesh::StreamStatus::need_more);
    CHECK(decoder.buffered_size() == 1);
    decoder.reset();
    CHECK(decoder.buffered_size() == 0);
}

} // namespace

int main() {
    test_checksum();
    test_frame_round_trip();
    test_encode_clamps_payload_length();
    test_frame_length_from_padded_read();
    test_decode_errors();
    test_sequence_tracker();
    test_registry_timeout();
    test_registry_timer_wraparound();
    test_acknowledgement_payload();
    test_reliable_queue();
    test_reliable_queue_capacity_and_control();
    test_retry_timer_wraparound();
    test_stream_decoder();
    test_stream_decoder_errors_and_recovery();
    std::cout << "PicoMesh tests passed\n";
    return 0;
}
