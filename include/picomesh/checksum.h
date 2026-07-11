#pragma once

#include <cstddef>
#include <cstdint>

namespace picomesh {

// Returns an 8-bit two's-complement checksum. Appending the returned byte to
// the input makes the unsigned sum of all bytes equal to zero modulo 256.
std::uint8_t checksum8(const std::uint8_t* data, std::size_t length) noexcept;

// Verifies a buffer that already contains its checksum byte.
bool verify_checksum8(const std::uint8_t* data, std::size_t length) noexcept;

}  // namespace picomesh
