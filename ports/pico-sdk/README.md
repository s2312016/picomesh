# Raspberry Pi Pico SDK port

This port currently provides:

- `I2cControllerTransport` for RP2040/RP2350 controller-mode I2C links.
- `I2cPeripheralEndpoint` with bounded double-buffered RX/TX mailboxes.
- `UartTransport` for non-blocking receive and blocking transmit.
- A CMake target named `PicoMesh::pico_transport`.

Pico and Pico 2 use the same source. Select the target board with `-DPICO_BOARD=pico` or `-DPICO_BOARD=pico2`.

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

The application remains responsible for configuring pins and bus speed:

```cpp
i2c_init(i2c0, 100000);
gpio_set_function(4, GPIO_FUNC_I2C);
gpio_set_function(5, GPIO_FUNC_I2C);

picomesh::pico::I2cPeripheralEndpoint endpoint;
endpoint.begin(i2c0, 0x42);
```

`publish()` prepares the frame returned to the next controller read. Incoming controller writes are captured by the ISR and decoded later by `poll()`. If the application does not call `poll()` before another complete write arrives, `dropped_frames()` increases instead of overwriting the unread frame.

Only one endpoint can own each hardware I2C instance. `begin()` installs an exclusive IRQ handler, so applications that already use the same IRQ must integrate the handlers explicitly.

For UART byte streams, feed received bytes into `picomesh::StreamDecoder` until it returns `StreamStatus::frame_ready`.

## Standalone example

Build the I2C heartbeat node with:

```sh
cmake -S examples/PicoSdkI2cNode -B build-pico -DPICO_BOARD=pico
cmake --build build-pico --parallel
```

Use `-DPICO_BOARD=pico2` for Pico 2. The resulting UF2 publishes a heartbeat at address `0x42` and accepts PicoMesh command frames with optional acknowledgements.

The peripheral mailbox generalizes the bounded IRQ and double-buffering lessons from AETERNA, but it does not include AETERNA's fixed 26-byte response, inference fields, fault commands, or board topology.

Do not vendor the Pico SDK or FreeRTOS into PicoMesh. Consumers should provide those dependencies through their own build environment.
