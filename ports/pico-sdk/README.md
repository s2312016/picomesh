# Raspberry Pi Pico SDK port

This port currently provides:

- `I2cControllerTransport` for RP2040/RP2350 controller-mode I2C links.
- `UartTransport` for non-blocking receive and blocking transmit.
- A CMake target named `PicoMesh::pico_transport`.

Pico and Pico 2 use the same source. Select the target board in the parent Pico SDK project with `-DPICO_BOARD=pico` or `-DPICO_BOARD=pico2`.

## Integration

Initialize the Pico SDK before adding PicoMesh, then include both directories:

```cmake
pico_sdk_init()
add_subdirectory(path/to/picomesh picomesh-build)
add_subdirectory(path/to/picomesh/ports/pico-sdk picomesh-pico-build)

target_link_libraries(my_firmware
    PRIVATE
        PicoMesh::picomesh
        PicoMesh::pico_transport
)
```

The application remains responsible for configuring the pins and baud rate:

```cpp
i2c_init(i2c0, 100000);
gpio_set_function(4, GPIO_FUNC_I2C);
gpio_set_function(5, GPIO_FUNC_I2C);

picomesh::pico::I2cControllerTransport link(i2c0, 0x42);
```

For UART byte streams, feed received bytes into `picomesh::StreamDecoder` until it returns `StreamStatus::frame_ready`.

## Peripheral-mode status

A generic RP2040/RP2350 I2C peripheral endpoint is planned next. The AETERNA raw-register interrupt implementation is not copied directly because it is coupled to the research firmware's fixed state response. PicoMesh will expose a bounded frame mailbox and a minimal ISR instead.

Do not vendor the Pico SDK or FreeRTOS into PicoMesh. Consumers should provide those dependencies through their own build environment.
