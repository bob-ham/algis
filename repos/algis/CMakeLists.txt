cmake_minimum_required(VERSION 3.20)
project(algis)

set(CMAKE_CXX_STANDARD 20)

add_library(algis STATIC
    seperated/impl/algis.cpp
    seperated/external/hooker/process.cpp
    seperated/external/hooker/mmap.cpp
    seperated/internal/dispatch.cpp
)

target_include_directories(algis PRIVATE
    ${CMAKE_SOURCE_DIR}
)


target_compile_definitions(algis PRIVATE ALGIS_EXTERNAL)