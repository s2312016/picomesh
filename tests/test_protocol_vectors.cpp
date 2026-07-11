#include "picomesh/frame.h"
#include "picomesh/reliability.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>

namespace {

template <std::size_t N>
void expect_bytes(const picomesh::EncodedFrame& encoded,
                  const std::array<std::uint8_t, N>& expected, const char* name) {
    if (encoded.length != expected.size()) {
        std::cerr << name << ": expected length " << expected.size() << ", got " << encoded.length
                  << '\n';
        std::exit(1);
    }

    for (std::size_t i = 0; i < expected.size(); ++i) {
        if (encoded.bytes[i] != expected[i]) {
            std::cerr << name << ": byte " << i << " expected 0x" << std::hex
                      << static_cast<unsigned>(expected[i]) << ", got 0x"
                      << static_cast<unsigned>(encoded.bytes[i]) << std::dec << '\n';
            std::exit(1);
        }
    }

    const auto decoded = picomesh::decode_frame(expected.data(), expected.size());
    if (!decoded) {
        std::cerr << name << ": published vector failed to decode with error "
                  << static_cast<unsigned>(decoded.error) << '\n';
        std::exit(1);
    }
}

void heartbeat_vector() {
    picomesh::Frame frame;
    frame.type = picomesh::MessageType::heartbeat;

    constexpr std::array<std::uint8_t, 8> expected{
        0xB7, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x47,
    };
    expect_bytes(picomesh::encode_frame(frame), expected, "empty heartbeat");
}

void command_vector() {
    picomesh::Frame frame;
    frame.type = picomesh::MessageType::command;
    frame.flags = picomesh::kFlagAckRequired;
    frame.node_id = 7;
    frame.sequence = 42;
    frame.payload_length = 3;
    frame.payload[0] = 0x10;
    frame.payload[1] = 0x20;
    frame.payload[2] = 0x30;

    constexpr std::array<std::uint8_t, 11> expected{
        0xB7, 0x01, 0x10, 0x01, 0x07, 0x2A, 0x03, 0x10, 0x20, 0x30, 0xA3,
    };
    expect_bytes(picomesh::encode_frame(frame), expected, "ACK-required command");
}

void acknowledgement_vector() {
    const auto frame = picomesh::make_ack_frame(7, 43, 42, picomesh::AckStatus::accepted);

    constexpr std::array<std::uint8_t, 10> expected{
        0xB7, 0x01, 0x11, 0x00, 0x07, 0x2B, 0x02, 0x2A, 0x00, 0xD9,
    };
    expect_bytes(picomesh::encode_frame(frame), expected, "accepted acknowledgement");
}

void state_vector() {
    picomesh::Frame frame;
    frame.type = picomesh::MessageType::state;
    frame.node_id = 5;
    frame.sequence = 23;
    frame.payload_length = 2;
    frame.payload[0] = 0x42;
    frame.payload[1] = 0x99;

    constexpr std::array<std::uint8_t, 10> expected{
        0xB7, 0x01, 0x02, 0x00, 0x05, 0x17, 0x02, 0x42, 0x99, 0x4D,
    };
    expect_bytes(picomesh::encode_frame(frame), expected, "state payload");
}

} // namespace

int main() {
    heartbeat_vector();
    command_vector();
    acknowledgement_vector();
    state_vector();
    std::cout << "Protocol conformance vectors passed\n";
    return 0;
}
