#include <PicoMesh.h>

using picomesh::DecodeError;
using picomesh::EncodedFrame;
using picomesh::Frame;
using picomesh::NodeRegistry;
using picomesh::arduino::WireControllerTransport;

constexpr uint8_t kNodeAddress = 0x42;

WireControllerTransport transport(Wire, kNodeAddress);
NodeRegistry registry(2500);
uint32_t last_poll_ms = 0;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(100000);
    Serial.println("PicoMesh Arduino heartbeat controller ready");
}

void loop() {
    const uint32_t now = millis();
    if (now - last_poll_ms < 500) {
        return;
    }
    last_poll_ms = now;

    uint8_t bytes[picomesh::kMaxEncodedFrameSize]{};
    const size_t length = transport.receive(bytes, sizeof(bytes));
    const auto decoded = picomesh::decode_frame(bytes, length);

    if (decoded) {
        const Frame& frame = decoded.frame;
        registry.observe(frame.node_id, frame.sequence, now);
        Serial.print("node=");
        Serial.print(frame.node_id);
        Serial.print(" sequence=");
        Serial.print(frame.sequence);
        Serial.print(" type=0x");
        Serial.println(static_cast<uint8_t>(frame.type), HEX);
    } else {
        Serial.print("No valid frame, bytes=");
        Serial.println(length);
    }

    registry.refresh(now);
    Serial.print("online nodes=");
    Serial.println(registry.online_count());
}
