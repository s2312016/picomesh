#include <cstdint>
#include <iostream>

#include "picomesh/reliability.h"

int main() {
    picomesh::ReliableQueue queue({100, 3});

    picomesh::Frame command;
    command.type = picomesh::MessageType::command;
    command.node_id = 2;
    command.sequence = 11;
    command.payload_length = 1;
    command.payload[0] = 0x55;

    if (queue.enqueue(command, 0) != picomesh::QueueResult::queued) {
        std::cerr << "Unable to queue command\n";
        return 1;
    }

    const auto first = queue.next_due(0);
    if (first.action != picomesh::TxAction::send || first.attempts != 1) {
        std::cerr << "First transmission was not scheduled\n";
        return 2;
    }

    if (queue.next_due(99).action != picomesh::TxAction::none) {
        std::cerr << "Retry was scheduled too early\n";
        return 3;
    }

    const auto retry = queue.next_due(100);
    if (retry.action != picomesh::TxAction::send || retry.attempts != 2) {
        std::cerr << "Retry was not scheduled\n";
        return 4;
    }

    const auto resolution = queue.acknowledge(
        command.node_id,
        command.sequence,
        picomesh::AckStatus::accepted);
    if (resolution != picomesh::AckResolution::accepted || queue.pending_count() != 0) {
        std::cerr << "Acknowledgement did not clear the queue\n";
        return 5;
    }

    std::cout << "Command sequence=" << static_cast<unsigned>(command.sequence)
              << " acknowledged after " << static_cast<unsigned>(retry.attempts)
              << " attempts\n";
    return 0;
}
