project(test_basics)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})


add_executable(basics
        test_thread_safe_stack.cpp
        test_lock_free_stack_shared_ptr.cpp
        test_thread_safe_queue.cpp)

include_directories(./../)
target_link_libraries(basics ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)
