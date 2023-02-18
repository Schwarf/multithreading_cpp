cmake_minimum_required(VERSION 3.20)
project(multithreading C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)
include(CheckCXXCompilerFlag)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.71.0)

if (NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif ()


function(enable_cxx_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_CUDA_FLAGS}" "${flag}" flag_already_set)
    if (flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if (flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
        endif ()
        unset(flag_supported CACHE)
    endif ()
endfunction()

enable_cxx_compiler_flag_if_supported("-Wall")
enable_cxx_compiler_flag_if_supported("-Wextra")
enable_cxx_compiler_flag_if_supported("-pedantic")
enable_cxx_compiler_flag_if_supported("-std=c++17")
enable_cxx_compiler_flag_if_supported("-O0")

include_directories(./../)
add_executable(basic_alarm classic_alarm.cpp)
add_executable(fork_alarm fork_alarm.cpp)
add_executable(thread_alarm thread_alarm.cpp)
add_executable(mutex_alarm mutex_alarm.cpp)

target_link_libraries(thread_alarm pthread)
target_link_libraries(mutex_alarm pthread)
target_link_libraries(fork_alarm pthread)
target_link_libraries(basic_alarm pthread)