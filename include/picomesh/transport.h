#pragma once

#include <cstddef>
#include <cstdint>

namespace picomesh {

/**
 * @brief Platform-neutral byte transport interface.
 *
 * Arduino Wire, Pico SDK I2C/UART, CAN, USB CDC, radio, and host simulators can
 * implement this boundary without changing the protocol or node-management
 * layers. Implementations must not write beyond the supplied receive capacity.
 */
class Transport {
public:
    virtual ~Transport() = default;

    /**
     * @brief Send one encoded byte sequence.
     * @param data Bytes to transmit.
     * @param length Number of bytes to transmit.
     * @return `true` when the transport accepted the complete sequence.
     */
    virtual bool send(const std::uint8_t* data, std::size_t length) = 0;

    /**
     * @brief Receive available bytes into a caller-owned buffer.
     * @param destination Output buffer.
     * @param capacity Maximum number of bytes that may be written.
     * @return Number of bytes written to @p destination.
     */
    virtual std::size_t receive(std::uint8_t* destination, std::size_t capacity) = 0;
};

}  // namespace picomesh
