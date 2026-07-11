#include <PicoMesh.h>

using picomesh::AckStatus;
using picomesh::Frame;
using picomesh::MessageType;
using picomesh::ReliableQueue;
using picomesh::TxAction;
using picomesh::arduino::WireControllerTransport;

constexpr uint8_t kNodeAddress = 0x42;
constexpr uint8_t kNodeId = 0;
constexpr uint32_t kCommandIntervalMs = 5000;
constexpr uint32_t kReceivePollMs = 20;

WireControllerTransport transport(Wire, kNodeAddress);
ReliableQueue queue({250, 3});
uint8_t sequence_number = 0;
uint32_t last_command_ms = 0;
uint32_t last_receive_ms = 0;

void enqueueCommand(const uint32_t now) {
    Frame command;
    command.type = MessageType::command;
    command.node_id = kNodeId;
    command.sequence = sequence_number++;
    command.payload_length = 2;
    command.payload[0] = 0x01; // Example application command.
    command.payload[1] = 0x01; // Example value.

    if (queue.enqueue(command, now) == picomesh::QueueResult::queued) {
        Serial.print("queued command sequence=");
        Serial.println(command.sequence);
    } else {
        Serial.println("unable to queue command");
    }
}

void transmitDue(const uint32_t now) {
    const auto decision = queue.next_due(now);
    if (decision.action == TxAction::none) {
        return;
    }
    if (decision.action == TxAction::exhausted) {
        Serial.print("retry exhausted sequence=");
        Serial.println(decision.frame.sequence);
        return;
    }

    const auto encoded = picomesh::encode_frame(decision.frame);
    const bool sent = transport.send(encoded.bytes.data(), encoded.length);
    Serial.print("send sequence=");
    Serial.print(decision.frame.sequence);
    Serial.print(" attempt=");
    Serial.print(decision.attempts);
    Serial.print(" result=");
    Serial.println(sent ? "ok" : "bus-error");
}

void receiveAcknowledgement() {
    uint8_t bytes[picomesh::kMaxEncodedFrameSize]{};
    const size_t length = transport.receive(bytes, sizeof(bytes));
    const auto decoded = picomesh::decode_frame(bytes, length);
    if (!decoded || decoded.frame.type != MessageType::acknowledgement) {
        return;
    }

    uint8_t acknowledged_sequence = 0;
    AckStatus status = AckStatus::malformed;
    if (!picomesh::parse_ack_frame(decoded.frame, acknowledged_sequence, status)) {
        return;
    }

    const auto resolution = queue.acknowledge(decoded.frame.node_id, acknowledged_sequence, status);

    Serial.print("ack sequence=");
    Serial.print(acknowledged_sequence);
    Serial.print(" status=");
    Serial.print(static_cast<uint8_t>(status));
    Serial.print(" matched=");
    Serial.println(resolution == picomesh::AckResolution::not_found ? "no" : "yes");
}

void setup() {
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(100000);
    Serial.println("PicoMesh reliable command controller ready");
    enqueueCommand(millis());
}

void loop() {
    const uint32_t now = millis();

    if (queue.pending_count() == 0 && now - last_command_ms >= kCommandIntervalMs) {
        last_command_ms = now;
        enqueueCommand(now);
    }

    transmitDue(now);

    if (now - last_receive_ms >= kReceivePollMs) {
        last_receive_ms = now;
        receiveAcknowledgement();
    }
}
