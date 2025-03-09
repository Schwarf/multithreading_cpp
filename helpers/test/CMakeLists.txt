project(test_helpers)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})


add_executable(helpers
        test_read_write_inputs.cpp)

include_directories(./../)
target_link_libraries(helpers ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)
