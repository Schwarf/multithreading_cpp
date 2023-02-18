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


add_executable(main main.cpp)
add_executable(basic basics/simple_thread.cpp posix_threads/thread_error_example.cpp posix_threads/lifecycle.cpp)
add_executable(print basics/print_character.cpp)
add_executable(trivial_deadlock basics/trivial_deadlock.cpp)
add_executable(try_out basics/try_outs.cpp)
add_executable(mutex basics/mutex_sample.cpp)
add_executable(stack_example basics/stack_example.cpp)

add_executable(basic_alarm posix_threads/alarm/classic_alarm.cpp)
add_executable(fork_alarm posix_threads/alarm/fork_alarm.cpp)
add_executable(thread_alarm posix_threads/alarm/thread_alarm.cpp)
add_executable(mutex_alarm posix_threads/alarm/mutex_alarm.cpp)

add_executable(thread_error posix_threads/thread_error_example.cpp)
add_executable(lifecycle posix_threads/lifecycle.cpp)

add_library(containers INTERFACE basics/threadsafe_stack.h)
add_library(posix_misc INTERFACE posix_threads/errors.h)

include_directories(posix_threads)

target_link_libraries(main)
target_link_libraries(basic pthread)
target_link_libraries(print pthread)
target_link_libraries(trivial_deadlock pthread)
target_link_libraries(try_out pthread)
target_link_libraries(mutex pthread)
target_link_libraries(stack_example pthread)
target_link_libraries(thread_alarm pthread)
target_link_libraries(thread_error pthread)
target_link_libraries(lifecycle pthread)
target_link_libraries(mutex_alarm pthread)

#target_link_libraries(alarm_basic posix_misc pthreads)