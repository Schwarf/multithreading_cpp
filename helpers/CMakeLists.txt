CMAKE_MINIMUM_REQUIRED(VERSION 3.22)
project(inputs)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(test)

add_library(read_write_inputs INTERFACE read_write_inputs.h)

