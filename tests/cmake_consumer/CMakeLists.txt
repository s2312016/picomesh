cmake_minimum_required(VERSION 3.16)
project(picomesh_consumer LANGUAGES CXX)

find_package(PicoMesh 0.1 REQUIRED CONFIG)

add_executable(picomesh_consumer main.cpp)
target_link_libraries(picomesh_consumer PRIVATE PicoMesh::picomesh)
