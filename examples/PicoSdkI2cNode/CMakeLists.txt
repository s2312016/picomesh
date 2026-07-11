cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED PICO_SDK_PATH AND DEFINED ENV{PICO_SDK_PATH})
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
endif()

if(NOT DEFINED PICO_SDK_PATH)
    message(FATAL_ERROR "Set PICO_SDK_PATH before configuring this example")
endif()

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(picomesh_pico_i2c_node C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../.. picomesh-build)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../ports/pico-sdk picomesh-pico-build)

add_executable(picomesh_pico_i2c_node main.cpp)

target_link_libraries(picomesh_pico_i2c_node
    PRIVATE
        PicoMesh::picomesh
        PicoMesh::pico_transport
        pico_stdlib
        hardware_i2c
)

pico_enable_stdio_usb(picomesh_pico_i2c_node 1)
pico_enable_stdio_uart(picomesh_pico_i2c_node 0)
pico_add_extra_outputs(picomesh_pico_i2c_node)
