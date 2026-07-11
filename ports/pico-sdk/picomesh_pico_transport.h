#pragma once

#include <cstddef>
#include <cstdint>

#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "picomesh/transport.h"

namespace picomesh::pico {

class I2cControllerTransport final : public Transport {
public:
    I2cControllerTransport(i2c_inst_t* instance, std::uint8_t address) noexcept;

    void set_address(std::uint8_t address) noexcept { address_ = address; }
    std::uint8_t address() const noexcept { return address_; }

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

}  // namespace picomesh::pico
