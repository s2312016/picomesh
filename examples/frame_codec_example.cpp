#include "picomesh/frame.h"

#include <cstdint>
#include <iostream>

int main() {
    picomesh::Frame frame;
    frame.type = picomesh::MessageType::state;
    frame.node_id = 4;
    frame.sequence = 9;
    frame.payload_length = 2;
    frame.payload[0] = 0x2a;
    frame.payload[1] = 0x01;

    const auto encoded = picomesh::encode_frame(frame);
    const auto decoded = picomesh::decode_frame(encoded);

    if (!decoded) {
        std::cerr << "Frame decode failed with error " << static_cast<unsigned>(decoded.error)
                  << '\n';
        return 1;
    }

    if (decoded.frame.node_id != frame.node_id || decoded.frame.sequence != frame.sequence ||
        decoded.frame.payload_length != frame.payload_length) {
        std::cerr << "Decoded frame does not match the source frame\n";
        return 2;
    }

    std::cout << "Decoded node=" << static_cast<unsigned>(decoded.frame.node_id)
              << " sequence=" << static_cast<unsigned>(decoded.frame.sequence)
              << " payload[0]=" << static_cast<unsigned>(decoded.frame.payload[0]) << '\n';
    return 0;
}
