cmake_minimum_required(VERSION 3.16)
project(picomesh VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PICOMESH_BUILD_TESTS "Build PicoMesh tests" ON)
option(PICOMESH_BUILD_EXAMPLES "Build PicoMesh examples" ON)

add_library(picomesh
    src/checksum.cpp
    src/frame.cpp
    src/node_registry.cpp
    src/reliability.cpp
    src/sequence_tracker.cpp
    src/stream_decoder.cpp
)

add_library(PicoMesh::picomesh ALIAS picomesh)

target_include_directories(picomesh
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(picomesh PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(picomesh PRIVATE /W4 /permissive-)
else()
    target_compile_options(picomesh PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

if(PICOMESH_BUILD_EXAMPLES)
    add_executable(picomesh_host_simulator examples/host_simulator.cpp)
    target_link_libraries(picomesh_host_simulator PRIVATE PicoMesh::picomesh)

    add_executable(picomesh_lossy_link_simulator examples/lossy_link_simulator.cpp)
    target_link_libraries(picomesh_lossy_link_simulator PRIVATE PicoMesh::picomesh)
endif()

if(PICOMESH_BUILD_TESTS)
    enable_testing()
    add_executable(picomesh_tests tests/test_main.cpp)
    target_link_libraries(picomesh_tests PRIVATE PicoMesh::picomesh)
    add_test(NAME picomesh_tests COMMAND picomesh_tests)

    if(PICOMESH_BUILD_EXAMPLES)
        add_test(NAME picomesh_lossy_link_simulator COMMAND picomesh_lossy_link_simulator)
    endif()
endif()
