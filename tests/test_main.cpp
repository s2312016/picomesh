#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>

#include "picomesh/checksum.h"
#include "picomesh/frame.h"
#include "picomesh/node_registry.h"
#include "picomesh/reliability.h"
#include "picomesh/sequence_tracker.h"
#include "picomesh/stream_decoder.h"

namespace {

void test_checksum() {
    std::array<std::uint8_t, 5> packet{1, 2, 3, 4, 0};
    packet.back() = picomesh::checksum8(packet.data(), packet.size() - 1);
    assert(picomesh::verify_checksum8(packet.data(), packet.size()));
}

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

void test_frame_round_trip() {
    const auto frame = sample_command();
    const auto encoded = picomesh::encode_frame(frame);
    const auto decoded = picomesh::decode_frame(encoded);
    assert(decoded);
    assert(decoded.frame.node_id == 7);
    assert(decoded.frame.sequence == 42);
    assert(decoded.frame.payload_length == 3);
    assert(decoded.frame.payload[2] == 0x30);
}

void test_bad_checksum() {
    picomesh::Frame frame;
    auto damaged = picomesh::encode_frame(frame);
    damaged.bytes[2] ^= 0x01;
    const auto decoded = picomesh::decode_frame(damaged);
    assert(!decoded);
    assert(decoded.error == picomesh::DecodeError::bad_checksum);
}

void test_sequence_tracker() {
    picomesh::SequenceTracker tracker;
    assert(tracker.observe(1, 254) == picomesh::SequenceState::first);
    assert(tracker.observe(1, 255) == picomesh::SequenceState::newer);
    assert(tracker.observe(1, 0) == picomesh::SequenceState::newer);
    assert(tracker.observe(1, 0) == picomesh::SequenceState::duplicate);
    assert(tracker.observe(1, 250) == picomesh::SequenceState::stale);
    assert(tracker.observe(40, 1) == picomesh::SequenceState::invalid_node);
}

void test_registry_timeout() {
    picomesh::NodeRegistry registry(1000);
    assert(registry.observe(2, 1, 500));
    registry.refresh(1200);
    assert(registry.get(2)->online);
    registry.refresh(1501);
    assert(!registry.get(2)->online);
    assert(registry.online_count() == 0);
}

void test_acknowledgement_payload() {
    const auto acknowledgement = picomesh::make_ack_frame(
        7,
        43,
        42,
        picomesh::AckStatus::accepted);

    std::uint8_t acknowledged_sequence = 0;
    picomesh::AckStatus status = picomesh::AckStatus::malformed;
    assert(picomesh::parse_ack_frame(acknowledgement, acknowledged_sequence, status));
    assert(acknowledged_sequence == 42);
    assert(status == picomesh::AckStatus::accepted);
}

void test_reliable_queue() {
    picomesh::ReliableQueue queue({100, 2});
    const auto command = sample_command();

    assert(queue.enqueue(command, 1000) == picomesh::QueueResult::queued);
    assert(queue.pending_count() == 1);

    auto decision = queue.next_due(1000);
    assert(decision.action == picomesh::TxAction::send);
    assert(decision.attempts == 1);
    assert((decision.frame.flags & picomesh::kFlagAckRequired) != 0);

    assert(queue.next_due(1099).action == picomesh::TxAction::none);
    decision = queue.next_due(1100);
    assert(decision.action == picomesh::TxAction::send);
    assert(decision.attempts == 2);

    decision = queue.next_due(1200);
    assert(decision.action == picomesh::TxAction::exhausted);
    assert(queue.pending_count() == 0);

    assert(queue.enqueue(command, 2000) == picomesh::QueueResult::queued);
    assert(queue.acknowledge(7, 42, picomesh::AckStatus::accepted) ==
           picomesh::AckResolution::accepted);
    assert(queue.pending_count() == 0);
}

void test_stream_decoder() {
    const auto encoded = picomesh::encode_frame(sample_command());
    picomesh::StreamDecoder decoder;

    assert(decoder.feed(0x00).status == picomesh::StreamStatus::discarded);

    picomesh::StreamResult result;
    for (std::size_t i = 0; i < encoded.length; ++i) {
        result = decoder.feed(encoded.bytes[i]);
    }

    assert(result.status == picomesh::StreamStatus::frame_ready);
    assert(result.frame.node_id == 7);
    assert(result.frame.sequence == 42);
    assert(decoder.buffered_size() == 0);
}

}  // namespace

int main() {
    test_checksum();
    test_frame_round_trip();
    test_bad_checksum();
    test_sequence_tracker();
    test_registry_timeout();
    test_acknowledgement_payload();
    test_reliable_queue();
    test_stream_decoder();
    std::cout << "PicoMesh tests passed\n";
    return 0;
}
