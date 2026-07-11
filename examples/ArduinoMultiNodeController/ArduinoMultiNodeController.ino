#include <PicoMesh.h>

using picomesh::Frame;
using picomesh::NodeRegistry;
using picomesh::arduino::WireControllerTransport;

constexpr uint8_t kFirstNodeAddress = 0x42;
constexpr uint8_t kSecondNodeAddress = 0x43;
constexpr uint32_t kPollIntervalMs = 500;

WireControllerTransport first_node(Wire, kFirstNodeAddress);
WireControllerTransport second_node(Wire, kSecondNodeAddress);
NodeRegistry registry(2500);
uint32_t last_poll_ms = 0;

void pollNode(WireControllerTransport& transport, const uint8_t address, const uint32_t now) {
    uint8_t bytes[picomesh::kMaxEncodedFrameSize]{};
    const size_t length = transport.receive(bytes, sizeof(bytes));
    const auto decoded = picomesh::decode_frame(bytes, length);

    Serial.print("address=0x");
    Serial.print(address, HEX);

    if (!decoded) {
        Serial.print(" invalid bytes=");
        Serial.println(length);
        return;
    }

    const Frame& frame = decoded.frame;
    registry.observe(frame.node_id, frame.sequence, now);
    Serial.print(" node=");
    Serial.print(frame.node_id);
    Serial.print(" sequence=");
    Serial.print(frame.sequence);
    Serial.print(" type=0x");
    Serial.println(static_cast<uint8_t>(frame.type), HEX);
}

void setup() {
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(100000);
    Serial.println("PicoMesh multi-node controller ready");
}

void loop() {
    const uint32_t now = millis();
    if (now - last_poll_ms < kPollIntervalMs) {
        return;
    }
    last_poll_ms = now;

    pollNode(first_node, kFirstNodeAddress, now);
    pollNode(second_node, kSecondNodeAddress, now);

    registry.refresh(now);
    Serial.print("online nodes=");
    Serial.println(registry.online_count());
}
