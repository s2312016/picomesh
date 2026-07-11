#include "picomesh_pico_transport.h"

#include "hardware/irq.h"
#include "hardware/sync.h"

namespace picomesh::pico {

I2cPeripheralEndpoint* I2cPeripheralEndpoint::endpoints_[2] = {nullptr, nullptr};

I2cControllerTransport::I2cControllerTransport(i2c_inst_t* instance,
                                               const std::uint8_t address) noexcept
    : instance_(instance), address_(address) {}

bool I2cControllerTransport::send(const std::uint8_t* data, const std::size_t length) {
    if (instance_ == nullptr || data == nullptr || length == 0) {
        return false;
    }

    const int written = i2c_write_blocking(instance_, address_, data, length, false);
    return written == static_cast<int>(length);
}

std::size_t I2cControllerTransport::receive(std::uint8_t* destination, const std::size_t capacity) {
    if (instance_ == nullptr || destination == nullptr || capacity == 0) {
        return 0;
    }

    const int received = i2c_read_blocking(instance_, address_, destination, capacity, false);
    if (received <= 0) {
        return 0;
    }

    const std::size_t count = static_cast<std::size_t>(received);
    const std::size_t frame_length = frame_length_from_prefix(destination, count);
    return frame_length != 0 ? frame_length : count;
}

bool UartTransport::send(const std::uint8_t* data, const std::size_t length) {
    if (instance_ == nullptr || data == nullptr || length == 0) {
        return false;
    }

    uart_write_blocking(instance_, data, length);
    return true;
}

std::size_t UartTransport::receive(std::uint8_t* destination, const std::size_t capacity) {
    if (instance_ == nullptr || destination == nullptr || capacity == 0) {
        return 0;
    }

    std::size_t count = 0;
    while (count < capacity && uart_is_readable(instance_)) {
        destination[count++] = static_cast<std::uint8_t>(uart_getc(instance_));
    }
    return count;
}

bool I2cPeripheralEndpoint::begin(i2c_inst_t* instance, const std::uint8_t address) noexcept {
    if (instance == nullptr || address > 0x7f) {
        return false;
    }

    const std::uint8_t index = instance == i2c0 ? 0 : instance == i2c1 ? 1 : 0xff;
    if (index > 1 || (endpoints_[index] != nullptr && endpoints_[index] != this)) {
        return false;
    }

    instance_ = instance;
    instance_index_ = index;
    endpoints_[index] = this;

    i2c_set_slave_mode(instance_, true, address);
    const uint irq = index == 0 ? I2C0_IRQ : I2C1_IRQ;
    irq_set_exclusive_handler(irq, index == 0 ? i2c0_irq_handler : i2c1_irq_handler);
    irq_set_enabled(irq, true);

    i2c_get_hw(instance_)->intr_mask =
        I2C_IC_INTR_MASK_M_RX_FULL_BITS | I2C_IC_INTR_MASK_M_RD_REQ_BITS |
        I2C_IC_INTR_MASK_M_STOP_DET_BITS | I2C_IC_INTR_MASK_M_TX_ABRT_BITS |
        I2C_IC_INTR_MASK_M_RX_OVER_BITS;
    return true;
}

bool I2cPeripheralEndpoint::publish(const Frame& frame) noexcept {
    const auto encoded = encode_frame(frame);
    if (encoded.length == 0 || encoded.length > kMaxEncodedFrameSize) {
        return false;
    }

    const std::uint8_t inactive = static_cast<std::uint8_t>(tx_active_index_ ^ 1u);
    for (std::size_t i = 0; i < encoded.length; ++i) {
        tx_buffers_[inactive][i] = encoded.bytes[i];
    }

    const std::uint32_t interrupt_state = save_and_disable_interrupts();
    if (tx_index_ != 0) {
        restore_interrupts(interrupt_state);
        return false;
    }
    tx_lengths_[inactive] = static_cast<std::uint8_t>(encoded.length);
    __dmb();
    tx_active_index_ = inactive;
    restore_interrupts(interrupt_state);
    return true;
}

bool I2cPeripheralEndpoint::poll(Frame& frame, DecodeError* error) noexcept {
    std::array<std::uint8_t, kMaxEncodedFrameSize> local{};
    std::uint8_t local_length = 0;

    const std::uint32_t interrupt_state = save_and_disable_interrupts();
    if (!rx_ready_) {
        restore_interrupts(interrupt_state);
        return false;
    }

    const std::uint8_t ready_index = rx_ready_index_;
    local_length = rx_lengths_[ready_index];
    for (std::uint8_t i = 0; i < local_length; ++i) {
        local[i] = rx_buffers_[ready_index][i];
    }
    rx_lengths_[ready_index] = 0;
    rx_ready_ = false;
    restore_interrupts(interrupt_state);

    const auto decoded = decode_frame(local.data(), local_length);
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

void I2cPeripheralEndpoint::i2c0_irq_handler() {
    if (endpoints_[0] != nullptr) {
        endpoints_[0]->handle_irq();
    }
}

void I2cPeripheralEndpoint::i2c1_irq_handler() {
    if (endpoints_[1] != nullptr) {
        endpoints_[1]->handle_irq();
    }
}

void I2cPeripheralEndpoint::complete_receive_from_isr() noexcept {
    const std::uint8_t write_index = rx_write_index_;
    if (rx_lengths_[write_index] == 0) {
        return;
    }

    if (rx_ready_) {
        rx_lengths_[write_index] = 0;
        ++dropped_frames_;
        return;
    }

    rx_ready_index_ = write_index;
    rx_ready_ = true;
    rx_write_index_ = static_cast<std::uint8_t>(write_index ^ 1u);
    rx_lengths_[rx_write_index_] = 0;
}

void I2cPeripheralEndpoint::handle_irq() noexcept {
    i2c_hw_t* const hw = i2c_get_hw(instance_);
    const std::uint32_t status = hw->raw_intr_stat;

    if ((status & I2C_IC_RAW_INTR_STAT_RX_FULL_BITS) != 0u) {
        const std::uint8_t byte = static_cast<std::uint8_t>(hw->data_cmd);
        const std::uint8_t write_index = rx_write_index_;
        const std::uint8_t length = rx_lengths_[write_index];
        if (length < kMaxEncodedFrameSize) {
            rx_buffers_[write_index][length] = byte;
            rx_lengths_[write_index] = static_cast<std::uint8_t>(length + 1u);
        } else {
            ++dropped_frames_;
        }
    }

    if ((status & I2C_IC_RAW_INTR_STAT_RD_REQ_BITS) != 0u) {
        (void)hw->clr_rd_req;
        const std::uint8_t active = tx_active_index_;
        const std::uint8_t length = tx_lengths_[active];
        const std::uint8_t value = tx_index_ < length ? tx_buffers_[active][tx_index_++] : 0;
        hw->data_cmd = value;
    }

    if ((status & I2C_IC_RAW_INTR_STAT_STOP_DET_BITS) != 0u) {
        (void)hw->clr_stop_det;
        complete_receive_from_isr();
        tx_index_ = 0;
    }

    if ((status & I2C_IC_RAW_INTR_STAT_TX_ABRT_BITS) != 0u) {
        (void)hw->clr_tx_abrt;
        tx_index_ = 0;
    }

    if ((status & I2C_IC_RAW_INTR_STAT_RX_OVER_BITS) != 0u) {
        (void)hw->clr_rx_over;
        ++dropped_frames_;
    }
}

} // namespace picomesh::pico
