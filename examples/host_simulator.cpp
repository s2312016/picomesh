#include <cstdint>
#include <iomanip>
#include <iostream>

#include "picomesh/frame.h"
#include "picomesh/node_registry.h"

int main() {
    picomesh::Frame heartbeat;
    heartbeat.type = picomesh::MessageType::heartbeat;
    heartbeat.node_id = 3;
    heartbeat.sequence = 1;

    const auto bytes = picomesh::encode_frame(heartbeat);
    std::cout << "Encoded heartbeat:";
    for (const auto byte : bytes) {
        std::cout << ' ' << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<unsigned>(byte);
    }
    std::cout << std::dec << '\n';

    const auto decoded = picomesh::decode_frame(bytes);
    if (!decoded) {
        std::cerr << "Decode failed\n";
        return 1;
    }

    picomesh::NodeRegistry registry(3000);
    registry.observe(decoded.frame->node_id, decoded.frame->sequence, 1000);
    registry.refresh(2500);
    std::cout << "Online nodes: " << registry.online_count() << '\n';
    registry.refresh(4501);
    std::cout << "Online after timeout: " << registry.online_count() << '\n';
    return 0;
}
