cmake_minimum_required(VERSION 3.20)

project(double_buffering LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Release)

add_executable(baseline baseline.cpp)
add_executable(synchronized_double_buffering synchronized_double_buffering.cpp)
add_executable(lock_free_double_buffering synchronized_double_buffering.cpp)

find_package(Threads REQUIRED)

target_include_directories(baseline PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(synchronized_double_buffering PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(lock_free_double_buffering PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(synchronized_double_buffering PRIVATE Threads::Threads)
target_link_libraries(lock_free_double_buffering PRIVATE Threads::Threads)
