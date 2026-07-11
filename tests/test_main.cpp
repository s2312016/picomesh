#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "picomesh/checksum.h"
#include "picomesh/frame.h"
#include "picomesh/node_registry.h"

namespace {

void test_checksum() {
    const std::uint8_t data[] = {1, 2, 3, 4};
    std::vector<std::uint8_t> packet(std::begin(data), std::end(data));
    packet.push_back(picomesh::checksum8(packet.data(), packet.size()));
    assert(picomesh::verify_checksum8(packet.data(), packet.size()));
}

void test_frame_round_trip() {
    picomesh::Frame frame;
    frame.type = picomesh::MessageType::state;
    frame.node_id = 7;
    frame.sequence = 42;
    frame.payload_length = 3;
    frame.payload[0] = 0x10;
    frame.payload[1] = 0x20;
    frame.payload[2] = 0x30;

    const auto encoded = picomesh::encode_frame(frame);
    const auto decoded = picomesh::decode_frame(encoded);
    assert(decoded);
    assert(decoded.frame->node_id == 7);
    assert(decoded.frame->sequence == 42);
    assert(decoded.frame->payload_length == 3);
    assert(decoded.frame->payload[2] == 0x30);
}

void test_bad_checksum() {
    picomesh::Frame frame;
    const auto original = picomesh::encode_frame(frame);
    auto damaged = original;
    damaged[2] ^= 0x01;
    const auto decoded = picomesh::decode_frame(damaged);
    assert(!decoded);
    assert(decoded.error == picomesh::DecodeError::bad_checksum);
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

}  // namespace

int main() {
    test_checksum();
    test_frame_round_trip();
    test_bad_checksum();
    test_registry_timeout();
    std::cout << "PicoMesh tests passed\n";
    return 0;
}
