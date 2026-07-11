#pragma once

#ifdef ARDUINO

#include "picomesh/frame.h"
#include "picomesh/transport.h"

#include <Arduino.h>
#include <Wire.h>

namespace picomesh::arduino {

class WireControllerTransport final : public Transport {
  public:
    explicit WireControllerTransport(TwoWire& wire = Wire, std::uint8_t address = 0x42) noexcept;

    void set_address(std::uint8_t address) noexcept {
        address_ = address;
    }
    std::uint8_t address() const noexcept {
        return address_;
    }

    bool send(const std::uint8_t* data, std::size_t length) override;
    std::size_t receive(std::uint8_t* destination, std::size_t capacity) override;

  private:
    TwoWire* wire_;
    std::uint8_t address_;
};

// Single-instance I2C peripheral endpoint. Arduino's Wire callbacks do not
// provide a user-data pointer, so only one endpoint can be active per sketch.
class WirePeripheralEndpoint {
  public:
    bool begin(std::uint8_t address, TwoWire& wire = Wire) noexcept;
    bool publish(const Frame& frame) noexcept;
    bool poll(Frame& frame, DecodeError* error = nullptr) noexcept;
    bool has_pending_frame() const noexcept {
        return rx_ready_;
    }

  private:
    static void receive_callback(int count);
    static void request_callback();
    void receive_from_isr(int count) noexcept;
    void request_from_isr() noexcept;

    static WirePeripheralEndpoint* active_;

    TwoWire* wire_{nullptr};
    volatile std::uint8_t rx_bytes_[kMaxEncodedFrameSize]{};
    volatile std::uint8_t rx_length_{0};
    volatile bool rx_ready_{false};
    std::uint8_t tx_bytes_[kMaxEncodedFrameSize]{};
    volatile std::uint8_t tx_length_{0};
};

} // namespace picomesh::arduino

#endif // ARDUINO
