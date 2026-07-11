#include "picomesh/frame.h"
#include "picomesh/reliability.h"
#include "picomesh/stream_decoder.h"
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

void test_acknowledgement_payload() {
    const auto acknowledgement = picomesh::make_ack_frame(7, 43, 42, picomesh::AckStatus::accepted);
    std::uint8_t acknowledged_sequence = 0;
    picomesh::AckStatus status = picomesh::AckStatus::malformed;
    CHECK(picomesh::parse_ack_frame(acknowledgement, acknowledged_sequence, status));
    CHECK(acknowledged_sequence == 42);
    CHECK(status == picomesh::AckStatus::accepted);

    auto malformed = acknowledgement;
    malformed.type = picomesh::MessageType::command;
    CHECK(!picomesh::parse_ack_frame(malformed, acknowledged_sequence, status));
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

} // namespace picomesh_test
