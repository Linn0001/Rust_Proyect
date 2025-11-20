cmake_minimum_required(VERSION 3.10)
project(ProyectoRustBase CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
)

add_executable(proyecto_rust_base main.cpp ${SOURCES})
