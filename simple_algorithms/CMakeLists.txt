CMAKE_MINIMUM_REQUIRED(VERSION 3.22)
project(benchmark)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Default to Release build type if none is specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Add optimization flags for Release builds
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

find_package(GTest REQUIRED)
find_package(benchmark REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(main main.cpp)
add_executable(benchmark benchmark.cpp)

target_link_libraries(main benchmark::benchmark)
target_link_libraries(benchmark benchmark::benchmark)
target_compile_options(main PRIVATE $<$<CONFIG:Release>:-O3>)
target_compile_options(benchmark PRIVATE $<$<CONFIG:Release>:-O3>)
