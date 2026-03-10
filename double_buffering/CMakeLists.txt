cmake_minimum_required(VERSION 3.20)

project(shared_memory_chat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(shared_memory_writer
        writer.cpp
)

add_executable(shared_memory_reader
        reader.cpp
)

target_include_directories(shared_memory_writer PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(shared_memory_reader PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)