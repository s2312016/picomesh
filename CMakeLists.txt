cmake_minimum_required(VERSION 3.16)
project(picomesh VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PICOMESH_BUILD_TESTS "Build PicoMesh tests" ON)
option(PICOMESH_BUILD_EXAMPLES "Build PicoMesh examples" ON)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(PICOMESH_INSTALL_DEFAULT ON)
else()
    set(PICOMESH_INSTALL_DEFAULT OFF)
endif()
option(PICOMESH_INSTALL "Generate PicoMesh install rules" ${PICOMESH_INSTALL_DEFAULT})

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

    add_executable(picomesh_malformed_link_simulator examples/malformed_link_simulator.cpp)
    target_link_libraries(picomesh_malformed_link_simulator PRIVATE PicoMesh::picomesh)

    add_executable(picomesh_frame_codec_example examples/frame_codec_example.cpp)
    target_link_libraries(picomesh_frame_codec_example PRIVATE PicoMesh::picomesh)

    add_executable(picomesh_reliable_queue_example examples/reliable_queue_example.cpp)
    target_link_libraries(picomesh_reliable_queue_example PRIVATE PicoMesh::picomesh)
endif()

if(PICOMESH_BUILD_TESTS)
    enable_testing()
    add_executable(picomesh_tests tests/test_main.cpp)
    target_link_libraries(picomesh_tests PRIVATE PicoMesh::picomesh)
    add_test(NAME picomesh_tests COMMAND picomesh_tests)

    if(PICOMESH_BUILD_EXAMPLES)
        add_test(NAME picomesh_lossy_link_simulator COMMAND picomesh_lossy_link_simulator)
        add_test(NAME picomesh_malformed_link_simulator COMMAND picomesh_malformed_link_simulator)
        add_test(NAME picomesh_frame_codec_example COMMAND picomesh_frame_codec_example)
        add_test(NAME picomesh_reliable_queue_example COMMAND picomesh_reliable_queue_example)
    endif()
endif()

if(PICOMESH_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(
        TARGETS picomesh
        EXPORT PicoMeshTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(
        DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    configure_package_config_file(
        cmake/PicoMeshConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/PicoMeshConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PicoMesh
    )
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/PicoMeshConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(
        EXPORT PicoMeshTargets
        FILE PicoMeshTargets.cmake
        NAMESPACE PicoMesh::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PicoMesh
    )
    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/PicoMeshConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/PicoMeshConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PicoMesh
    )
endif()
