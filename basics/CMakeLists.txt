cmake_minimum_required(VERSION 3.20)
project(concurrency_in_action C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

# ——— Sanitizers —————————————————————————————————————
# For GCC/Clang, enable AddressSanitizer (heap-use-after-free, OOB, leaks)
# and UBSan (undefined-behavior detection).
#if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
#    message(STATUS "Building with AddressSanitizer and UBSan")
#    add_compile_options(
#            -fsanitize=address,undefined
#            -fno-omit-frame-pointer
#    )
#    add_link_options(
#            -fsanitize=address,undefined
#    )
#endif ()
# ———————————————————————————————————————————————————
include(CheckCXXCompilerFlag)


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
enable_cxx_compiler_flag_if_supported("-O2")

add_subdirectory(test)

add_executable(trivial_deadlock trivial_deadlock.cpp)
add_executable(try_out compiler_dependent_output.cpp)
add_library(containers INTERFACE thread_safe_stack.h thread_safe_list.h lock_free_stack_shared_ptr.h)

target_link_libraries(trivial_deadlock pthread)
target_link_libraries(try_out pthread)
target_link_libraries(containers INTERFACE pthread)

