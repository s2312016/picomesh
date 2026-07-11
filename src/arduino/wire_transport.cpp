#ifdef ARDUINO

#include "arduino/wire_transport.h"

namespace picomesh::arduino {

WirePeripheralEndpoint* WirePeripheralEndpoint::active_ = nullptr;

WireControllerTransport::WireControllerTransport(
    TwoWire& wire,
    const std::uint8_t address) noexcept
    : wire_(&wire), address_(address) {}

bool WireControllerTransport::send(
    const std::uint8_t* data,
    const std::size_t length) {
    if (data == nullptr || length == 0 || length > 255) {
        return false;
    }

    wire_->beginTransmission(address_);
    const auto written = wire_->write(data, length);
    return written == length && wire_->endTransmission() == 0;
}

std::size_t WireControllerTransport::receive(
    std::uint8_t* destination,
    const std::size_t capacity) {
    if (destination == nullptr || capacity == 0) {
        return 0;
    }

    const auto requested = static_cast<std::uint8_t>(
        capacity < kMaxEncodedFrameSize ? capacity : kMaxEncodedFrameSize);
    const auto available = wire_->requestFrom(address_, requested);

    std::size_t count = 0;
    while (wire_->available() && count < capacity && count < available) {
        destination[count++] = static_cast<std::uint8_t>(wire_->read());
    }
    while (wire_->available()) {
        (void)wire_->read();
    }
    return count;
}

bool WirePeripheralEndpoint::begin(
    const std::uint8_t address,
    TwoWire& wire) noexcept {
    if (active_ != nullptr && active_ != this) {
        return false;
    }

    wire_ = &wire;
    active_ = this;
    wire_->begin(address);
    wire_->onReceive(receive_callback);
    wire_->onRequest(request_callback);
    return true;
}

bool WirePeripheralEndpoint::publish(const Frame& frame) noexcept {
    const auto encoded = encode_frame(frame);
    if (encoded.length == 0 || encoded.length > kMaxEncodedFrameSize) {
        return false;
    }

    noInterrupts();
    for (std::size_t i = 0; i < encoded.length; ++i) {
        tx_bytes_[i] = encoded.bytes[i];
    }
    tx_length_ = static_cast<std::uint8_t>(encoded.length);
    interrupts();
    return true;
}

bool WirePeripheralEndpoint::poll(Frame& frame, DecodeError* error) noexcept {
    if (!rx_ready_) {
        return false;
    }

    std::uint8_t local[kMaxEncodedFrameSize]{};
    std::uint8_t local_length = 0;

    noInterrupts();
    local_length = rx_length_;
    for (std::uint8_t i = 0; i < local_length; ++i) {
        local[i] = rx_bytes_[i];
    }
    rx_ready_ = false;
    rx_length_ = 0;
    interrupts();

    const auto decoded = decode_frame(local, local_length);
    if (!decoded) {
        if (error != nullptr) {
            *error = decoded.error;
        }
        return false;
    }

    frame = decoded.frame;
    if (error != nullptr) {
        *error = DecodeError::none;
    }
    return true;
}

void WirePeripheralEndpoint::receive_callback(const int count) {
    if (active_ != nullptr) {
        active_->receive_from_isr(count);
    }
}

void WirePeripheralEndpoint::request_callback() {
    if (active_ != nullptr) {
        active_->request_from_isr();
    }
}

void WirePeripheralEndpoint::receive_from_isr(const int count) noexcept {
    std::uint8_t length = 0;
    for (int i = 0; i < count && wire_->available(); ++i) {
        const auto byte = static_cast<std::uint8_t>(wire_->read());
        if (length < kMaxEncodedFrameSize) {
            rx_bytes_[length++] = byte;
        }
    }
    while (wire_->available()) {
        (void)wire_->read();
    }
    rx_length_ = length;
    rx_ready_ = length > 0;
}

void WirePeripheralEndpoint::request_from_isr() noexcept {
    const auto length = tx_length_;
    if (length > 0) {
        wire_->write(tx_bytes_, length);
    }
}

}  // namespace picomesh::arduino

#endif  // ARDUINO
