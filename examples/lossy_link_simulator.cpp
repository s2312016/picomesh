#include <cstdint>
#include <iostream>

#include "picomesh/frame.h"
#include "picomesh/reliability.h"

int main() {
    picomesh::ReliableQueue queue({200, 3});

    picomesh::Frame command;
    command.type = picomesh::MessageType::command;
    command.node_id = 4;
    command.sequence = 17;
    command.payload_length = 1;
    command.payload[0] = 0x55;

    if (queue.enqueue(command, 0) != picomesh::QueueResult::queued) {
        std::cerr << "Unable to queue command\n";
        return 1;
    }

    std::uint32_t now_ms = 0;
    unsigned simulated_transmissions = 0;

    while (queue.pending_count() != 0) {
        const auto decision = queue.next_due(now_ms);
        if (decision.action == picomesh::TxAction::send) {
            ++simulated_transmissions;
            std::cout << "TX attempt " << static_cast<unsigned>(decision.attempts)
                      << " at " << now_ms << " ms\n";

            // Deterministically drop the first packet and accept the second.
            if (simulated_transmissions == 1) {
                std::cout << "  packet dropped by simulated link\n";
            } else {
                const auto resolution = queue.acknowledge(
                    decision.frame.node_id,
                    decision.frame.sequence,
                    picomesh::AckStatus::accepted);
                std::cout << "  acknowledgement received: "
                          << (resolution == picomesh::AckResolution::accepted ? "accepted" : "error")
                          << '\n';
            }
        } else if (decision.action == picomesh::TxAction::exhausted) {
            std::cout << "Retry budget exhausted\n";
        }
        now_ms = static_cast<std::uint32_t>(now_ms + 50u);
    }

    std::cout << "Completed after " << simulated_transmissions << " transmissions\n";
    return simulated_transmissions == 2 ? 0 : 1;
}
