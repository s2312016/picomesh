#include "picomesh_pico_transport.h"

namespace picomesh::pico {

I2cControllerTransport::I2cControllerTransport(
    i2c_inst_t* instance,
    const std::uint8_t address) noexcept
    : instance_(instance), address_(address) {}

bool I2cControllerTransport::send(
    const std::uint8_t* data,
    const std::size_t length) {
    if (instance_ == nullptr || data == nullptr || length == 0) {
        return false;
    }

    const int written = i2c_write_blocking(instance_, address_, data, length, false);
    return written == static_cast<int>(length);
}

std::size_t I2cControllerTransport::receive(
    std::uint8_t* destination,
    const std::size_t capacity) {
    if (instance_ == nullptr || destination == nullptr || capacity == 0) {
        return 0;
    }

    const int received = i2c_read_blocking(instance_, address_, destination, capacity, false);
    return received > 0 ? static_cast<std::size_t>(received) : 0;
}

bool UartTransport::send(
    const std::uint8_t* data,
    const std::size_t length) {
    if (instance_ == nullptr || data == nullptr || length == 0) {
        return false;
    }

    uart_write_blocking(instance_, data, length);
    return true;
}

std::size_t UartTransport::receive(
    std::uint8_t* destination,
    const std::size_t capacity) {
    if (instance_ == nullptr || destination == nullptr || capacity == 0) {
        return 0;
    }

    std::size_t count = 0;
    while (count < capacity && uart_is_readable(instance_)) {
        destination[count++] = static_cast<std::uint8_t>(uart_getc(instance_));
    }
    return count;
}

}  // namespace picomesh::pico
