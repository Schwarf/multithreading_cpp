//
// Created by andreas on 27.04.25.
//
#include "gtest/gtest.h"
#include "./../lock_free_skip_list.h"

#include <thread>
#include <vector>
#include <string>

// Fixture for single-threaded tests
class LockFreeSkipListSingleTest : public ::testing::Test {
protected:
    LockFreeSkipList<int, std::string, 16> skip;
};

TEST_F(LockFreeSkipListSingleTest, InsertSearchRemove) {
    std::string value;
    // Initially empty
    EXPECT_FALSE(skip.search(1, value));

    // Insert elements
    EXPECT_TRUE(skip.insert(1, "one"));
    EXPECT_TRUE(skip.insert(2, "two"));
    EXPECT_TRUE(skip.insert(3, "three"));

    // Duplicate insert returns false
    EXPECT_FALSE(skip.insert(2, "dos"));

    // Search existing
    EXPECT_TRUE(skip.search(1, value));
    EXPECT_EQ(value, "one");
    EXPECT_TRUE(skip.search(2, value));
    EXPECT_EQ(value, "two");
    EXPECT_TRUE(skip.search(3, value));
    EXPECT_EQ(value, "three");

    // Remove existing
    EXPECT_TRUE(skip.remove(2));
    EXPECT_FALSE(skip.search(2, value));
    // Removing again returns false
    EXPECT_FALSE(skip.remove(2));
}

// Fixture for multithreaded tests
template<size_t Threads, size_t OpsPerThread>
void concurrent_insert_search_remove() {
    LockFreeSkipList<int, int, 16> skip;
    std::vector<std::thread> threads;

    // Concurrent inserts
    for (size_t t = 0; t < Threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < (int)OpsPerThread; ++i) {
                int key = t * OpsPerThread + i;
                skip.insert(key, key * 10);
            }
        });
    }
    for (auto& th : threads) th.join();

    // Verify all inserted
    for (size_t t = 0; t < Threads; ++t) {
        for (int i = 0; i < (int)OpsPerThread; ++i) {
            int key = t * OpsPerThread + i;
            int value;
            EXPECT_TRUE(skip.search(key, value));
            EXPECT_EQ(value, key * 10);
        }
    }

    // Concurrent removals
    threads.clear();
    for (size_t t = 0; t < Threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < (int)OpsPerThread; ++i) {
                int key = t * OpsPerThread + i;
                skip.remove(key);
            }
        });
    }
    for (auto& th : threads) th.join();

    // Verify all removed
    for (size_t t = 0; t < Threads; ++t) {
        for (int i = 0; i < (int)OpsPerThread; ++i) {
            int key = t * OpsPerThread + i;
            int value;
            EXPECT_FALSE(skip.search(key, value));
        }
    }
}

// Instantiate multithreaded test
TEST(LockFreeSkipListConcurrentTest, ConcurrentInsertSearchRemove) {
    constexpr size_t threads = 4;
    constexpr size_t ops = 1000;
    concurrent_insert_search_remove<threads, ops>();
}

