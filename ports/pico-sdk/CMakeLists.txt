add_library(picomesh_pico_transport
    picomesh_pico_transport.cpp
)

add_library(PicoMesh::pico_transport ALIAS picomesh_pico_transport)

target_include_directories(picomesh_pico_transport
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(picomesh_pico_transport
    PUBLIC
        PicoMesh::picomesh
        hardware_i2c
        hardware_irq
        hardware_sync
        hardware_uart
)

target_compile_features(picomesh_pico_transport PUBLIC cxx_std_17)
