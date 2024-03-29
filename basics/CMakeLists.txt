cmake_minimum_required(VERSION 3.20)
project(concurrency_in_action C CXX)

set(CMAKE_CXX_STANDARD 20)
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
enable_cxx_compiler_flag_if_supported("-O3")

add_subdirectory(test)

add_executable(trivial_deadlock trivial_deadlock.cpp)
add_executable(try_out compiler_dependent_output.cpp)
add_executable(mutex mutex_sample.cpp)
add_executable(basic simple_thread.cpp thread_safe_queue.h)
add_executable(print print_character.cpp)
add_executable(memory_order memory_orders.cpp)
add_executable(memory_order2 memory_orders2.cpp)
add_library(containers INTERFACE thread_safe_stack.h thread_safe_list.h lock_free_stack.h)

target_link_libraries(basic pthread)
target_link_libraries(print pthread)
target_link_libraries(trivial_deadlock pthread)
target_link_libraries(try_out pthread)
target_link_libraries(mutex pthread)
target_link_libraries(memory_order pthread)
target_link_libraries(memory_order2 pthread)
target_link_libraries(containers INTERFACE pthread)

