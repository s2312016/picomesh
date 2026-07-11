#pragma once

#include <cstddef>
#include <cstdint>

namespace picomesh {

/**
 * @brief Compute the protocol's 8-bit two's-complement checksum.
 *
 * Appending the returned byte to the input makes the unsigned sum of every
 * byte equal to zero modulo 256.
 *
 * @param data Input bytes. A null pointer returns zero.
 * @param length Number of input bytes.
 * @return Checksum byte to append to the input.
 */
std::uint8_t checksum8(const std::uint8_t* data, std::size_t length) noexcept;

/**
 * @brief Verify a buffer that already includes its checksum byte.
 * @param data Complete encoded bytes including the checksum.
 * @param length Number of bytes in the complete buffer.
 * @return `true` when the unsigned byte sum is zero modulo 256.
 */
bool verify_checksum8(const std::uint8_t* data, std::size_t length) noexcept;

} // namespace picomesh
