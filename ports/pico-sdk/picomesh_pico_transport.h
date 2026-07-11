#pragma once

#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "picomesh/frame.h"
#include "picomesh/transport.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace picomesh::pico {

class I2cControllerTransport final : public Transport {
  public:
    I2cControllerTransport(i2c_inst_t* instance, std::uint8_t address) noexcept;

    void set_address(std::uint8_t address) noexcept {
        address_ = address;
    }
    std::uint8_t address() const noexcept {
        return address_;
    }

    bool send(const std::uint8_t* data, std::size_t length) override;
    std::size_t receive(std::uint8_t* destination, std::size_t capacity) override;

  private:
    i2c_inst_t* instance_;
    std::uint8_t address_;
};

class UartTransport final : public Transport {
  public:
    explicit UartTransport(uart_inst_t* instance) noexcept : instance_(instance) {}

    bool send(const std::uint8_t* data, std::size_t length) override;
    std::size_t receive(std::uint8_t* destination, std::size_t capacity) override;

  private:
    uart_inst_t* instance_;
};

// Bounded I2C peripheral mailbox for Pico/Pico 2. The application initializes
// the I2C instance and GPIO pins before calling begin(). One endpoint may be
// attached to each hardware I2C controller.
class I2cPeripheralEndpoint {
  public:
    bool begin(i2c_inst_t* instance, std::uint8_t address) noexcept;
    bool publish(const Frame& frame) noexcept;
    bool poll(Frame& frame, DecodeError* error = nullptr) noexcept;
    bool has_pending_frame() const noexcept {
        return rx_ready_;
    }
    std::uint32_t dropped_frames() const noexcept {
        return dropped_frames_;
    }

  private:
    static void i2c0_irq_handler();
    static void i2c1_irq_handler();
    void handle_irq() noexcept;
    void complete_receive_from_isr() noexcept;

    static I2cPeripheralEndpoint* endpoints_[2];

    i2c_inst_t* instance_{nullptr};
    std::uint8_t instance_index_{0};

    std::array<std::array<std::uint8_t, kMaxEncodedFrameSize>, 2> rx_buffers_{};
    volatile std::uint8_t rx_lengths_[2]{0, 0};
    volatile std::uint8_t rx_write_index_{0};
    volatile std::uint8_t rx_ready_index_{0};
    volatile bool rx_ready_{false};

    std::array<std::array<std::uint8_t, kMaxEncodedFrameSize>, 2> tx_buffers_{};
    volatile std::uint8_t tx_lengths_[2]{0, 0};
    volatile std::uint8_t tx_active_index_{0};
    volatile std::uint8_t tx_index_{0};

    volatile std::uint32_t dropped_frames_{0};
};

} // namespace picomesh::pico
