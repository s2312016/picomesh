#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "picomesh/frame.h"
#include "picomesh/reliability.h"
#include "picomesh_pico_transport.h"

#include <cstdint>
#include <cstdio>

namespace {

constexpr std::uint8_t kNodeId = 0;
constexpr std::uint8_t kI2cAddress = 0x42;
constexpr uint kSdaPin = 4;
constexpr uint kSclPin = 5;

picomesh::pico::I2cPeripheralEndpoint endpoint;
std::uint8_t sequence_number = 0;

void publish_heartbeat() {
    picomesh::Frame heartbeat;
    heartbeat.type = picomesh::MessageType::heartbeat;
    heartbeat.node_id = kNodeId;
    heartbeat.sequence = sequence_number++;
    heartbeat.payload_length = 4;

    const std::uint32_t uptime_ms = static_cast<std::uint32_t>(time_us_64() / 1000u);
    heartbeat.payload[0] = static_cast<std::uint8_t>(uptime_ms);
    heartbeat.payload[1] = static_cast<std::uint8_t>(uptime_ms >> 8);
    heartbeat.payload[2] = static_cast<std::uint8_t>(uptime_ms >> 16);
    heartbeat.payload[3] = static_cast<std::uint8_t>(uptime_ms >> 24);
    endpoint.publish(heartbeat);
}

} // namespace

int main() {
    stdio_init_all();

    i2c_init(i2c0, 100000);
    gpio_set_function(kSdaPin, GPIO_FUNC_I2C);
    gpio_set_function(kSclPin, GPIO_FUNC_I2C);
    gpio_pull_up(kSdaPin);
    gpio_pull_up(kSclPin);

    if (!endpoint.begin(i2c0, kI2cAddress)) {
        std::printf("Unable to start PicoMesh I2C endpoint\n");
        return 1;
    }

    publish_heartbeat();
    std::uint64_t last_heartbeat_ms = 0;

    for (;;) {
        const std::uint64_t now_ms = time_us_64() / 1000u;
        if (now_ms - last_heartbeat_ms >= 1000u) {
            last_heartbeat_ms = now_ms;
            publish_heartbeat();
        }

        picomesh::Frame incoming;
        if (endpoint.poll(incoming)) {
            std::printf("RX node=%u sequence=%u type=0x%02x\n", incoming.node_id, incoming.sequence,
                        static_cast<unsigned>(incoming.type));

            if ((incoming.flags & picomesh::kFlagAckRequired) != 0u) {
                endpoint.publish(picomesh::make_ack_frame(
                    kNodeId, sequence_number++, incoming.sequence, picomesh::AckStatus::accepted));
            }
        }

        sleep_ms(1);
    }
}
