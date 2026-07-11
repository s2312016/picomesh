# Getting started

PicoMesh has one portable C++17 core and platform adapters for host builds, Arduino, and the Raspberry Pi Pico SDK. Start with the host build even when the final target is a microcontroller; it verifies the codec, reliability queue, and test suite without requiring hardware.

## Choose a path

| Goal | Start here |
|---|---|
| Evaluate the protocol on a PC | [Host build](#host-build) |
| Use Arduino Wire | [Arduino](#arduino) |
| Build RP2040 or RP2350 firmware | [Raspberry Pi Pico SDK](#raspberry-pi-pico-sdk) |
| Connect mixed-voltage boards | [Hardware safety](#hardware-safety) |

## Host build

Requirements:

- CMake 3.16 or newer
- A C++17 compiler
- Ninja, Make, or a supported IDE generator

```sh
git clone https://github.com/s2312016/picomesh.git
cd picomesh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Run the examples:

```sh
./build/picomesh_frame_codec_example
./build/picomesh_reliable_queue_example
./build/picomesh_host_simulator
./build/picomesh_lossy_link_simulator
```

On a multi-config generator such as Visual Studio, add `--config Release` to the build and test commands.

### Consume an installed CMake package

```sh
cmake --install build --prefix ./install
```

In another project:

```cmake
find_package(PicoMesh CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE PicoMesh::picomesh)
```

Configure the consumer with `CMAKE_PREFIX_PATH` pointing to the installation prefix.

## Arduino

Until the library is available through a registry, install a source archive with **Sketch > Include Library > Add .ZIP Library**, or place the repository in the Arduino libraries directory.

Open one of these examples:

- `ArduinoHeartbeatNode`: publishes heartbeat frames as an I2C peripheral.
- `ArduinoHeartbeatController`: reads one node and maintains liveness state.
- `ArduinoMultiNodeController`: polls multiple I2C addresses.
- `ArduinoReliableCommandController`: sends commands and resolves ACKs with bounded retries.

Select the exact board and port, compile, upload, and open the serial monitor at the baud rate used by the sketch.

## Raspberry Pi Pico SDK

The reference standalone example uses Pico SDK 2.3.0, I2C address `0x42`, SDA on GP4, SCL on GP5, and a 100 kHz bus.

```sh
export PICO_SDK_PATH=/path/to/pico-sdk
cmake -S examples/PicoSdkI2cNode -B build-pico -DPICO_BOARD=pico
cmake --build build-pico --parallel
```

For Pico 2:

```sh
cmake -S examples/PicoSdkI2cNode -B build-pico2 -DPICO_BOARD=pico2
cmake --build build-pico2 --parallel
```

Copy the generated `.uf2` file to the board while it is in BOOTSEL mode. See [`ports/pico-sdk/README.md`](../ports/pico-sdk/README.md) for integration into another firmware project.

## Hardware safety

Do not assume that an I2C development board uses safe pull-up voltages. Pico and Pico 2 GPIO are 3.3 V and are not 5 V tolerant. Use a suitable bidirectional level shifter when the controller side may pull SDA or SCL to 5 V, connect grounds, and verify idle voltages before connecting GPIO.

Read [`hardware_wiring.md`](hardware_wiring.md) before physical testing and record results with [`hardware_validation.md`](hardware_validation.md).

## What automated validation proves

CI verifies host builds and tests on supported operating systems, Arduino compilation, Pico/Pico 2 SDK compilation, sanitizers, static analysis, CodeQL, package installation, and documentation generation.

Automated builds do not prove electrical compatibility, bus integrity, timing margin, or interoperability on physical boards. Hardware claims remain pending until accompanied by a reproducible validation report.
