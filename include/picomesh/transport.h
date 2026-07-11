#pragma once

#include <cstddef>
#include <cstdint>

namespace picomesh {

// Platform-neutral byte transport interface. Arduino Wire, Pico SDK I2C/UART,
// CAN, USB CDC, and host simulators can implement this without changing the
// protocol or node-management layers.
class Transport {
public:
    virtual ~Transport() = default;

    virtual bool send(const std::uint8_t* data, std::size_t length) = 0;
    virtual std::size_t receive(std::uint8_t* destination, std::size_t capacity) = 0;
};

}  // namespace picomesh
