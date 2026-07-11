#include <PicoMesh.h>

using picomesh::AckStatus;
using picomesh::Frame;
using picomesh::MessageType;
using picomesh::arduino::WirePeripheralEndpoint;

constexpr uint8_t kNodeId = 0;
constexpr uint8_t kI2cAddress = 0x42;

WirePeripheralEndpoint endpoint;
uint8_t sequence_number = 0;
uint32_t last_heartbeat_ms = 0;

void publishHeartbeat() {
  Frame heartbeat;
  heartbeat.type = MessageType::heartbeat;
  heartbeat.node_id = kNodeId;
  heartbeat.sequence = sequence_number++;
  heartbeat.payload_length = 4;

  const uint32_t uptime = millis();
  heartbeat.payload[0] = static_cast<uint8_t>(uptime);
  heartbeat.payload[1] = static_cast<uint8_t>(uptime >> 8);
  heartbeat.payload[2] = static_cast<uint8_t>(uptime >> 16);
  heartbeat.payload[3] = static_cast<uint8_t>(uptime >> 24);
  endpoint.publish(heartbeat);
}

void setup() {
  Serial.begin(115200);
  endpoint.begin(kI2cAddress);
  publishHeartbeat();
  Serial.println("PicoMesh Arduino heartbeat node ready");
}

void loop() {
  const uint32_t now = millis();
  if (now - last_heartbeat_ms >= 1000) {
    last_heartbeat_ms = now;
    publishHeartbeat();
  }

  Frame incoming;
  if (endpoint.poll(incoming)) {
    Serial.print("Received type=0x");
    Serial.print(static_cast<uint8_t>(incoming.type), HEX);
    Serial.print(" sequence=");
    Serial.println(incoming.sequence);

    if ((incoming.flags & picomesh::kFlagAckRequired) != 0) {
      const Frame acknowledgement = picomesh::make_ack_frame(
          kNodeId,
          sequence_number++,
          incoming.sequence,
          AckStatus::accepted);
      endpoint.publish(acknowledgement);
    }
  }
}
