cmake_minimum_required(VERSION 3.20)

project(double_buffering LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Release)

add_executable(baseline
        baseline.cpp
)

#add_executable(shared_memory_reader
#        reader.cpp
#)

target_include_directories(baseline PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

#target_include_directories(baseline PRIVATE
#        ${CMAKE_CURRENT_SOURCE_DIR}
#)