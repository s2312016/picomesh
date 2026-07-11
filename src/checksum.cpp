#include "picomesh/checksum.h"

namespace picomesh {

std::uint8_t checksum8(const std::uint8_t* data, const std::size_t length) noexcept {
    std::uint8_t sum = 0;
    if (data == nullptr) {
        return sum;
    }

    for (std::size_t i = 0; i < length; ++i) {
        sum = static_cast<std::uint8_t>(sum + data[i]);
    }
    return static_cast<std::uint8_t>(0u - sum);
}

bool verify_checksum8(const std::uint8_t* data, const std::size_t length) noexcept {
    if (data == nullptr || length == 0) {
        return false;
    }

    std::uint8_t sum = 0;
    for (std::size_t i = 0; i < length; ++i) {
        sum = static_cast<std::uint8_t>(sum + data[i]);
    }
    return sum == 0;
}

}  // namespace picomesh
