project(test_fun_with_algos)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(./../)

add_executable(test_lock_free_hash_table
                test_lock_free_hash_table.cpp)

target_link_libraries(test_lock_free_hash_table GTest::GTest GTest::Main pthread)
add_test(NAME TestLockFreeHashTable COMMAND test_lock_free_hash_table)
