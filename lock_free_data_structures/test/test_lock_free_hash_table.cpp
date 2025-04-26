//
// Created by andreas on 26.04.25.
//
#include <random>
#include <thread>

#include "./../lock_free_hash_table.h"
#include "gtest/gtest.h"

using HashTable = LockFreeHashTable<int64_t, -1, -1, 1024>;

TEST(TestLockFreeHashTable, InsertAndLookupSingleThread)
{
    HashTable table;
    EXPECT_EQ(table.lookup(42), HashTable::no_value);
    EXPECT_TRUE(table.insert(42, 100));
    EXPECT_EQ(table.lookup(42), 100);

    EXPECT_TRUE(table.insert(100, 200));
    EXPECT_EQ(table.lookup(100), 200);

    // Update existing key
    EXPECT_TRUE(table.insert(42, 300));
    EXPECT_EQ(table.lookup(42), 300);
}


TEST(LockFreeHashTableTest, InsertSameKeyMultipleTimes)
{
    HashTable table;

    EXPECT_TRUE(table.insert(55, 1));
    EXPECT_TRUE(table.insert(55, 2));
    EXPECT_TRUE(table.insert(55, 3));

    EXPECT_EQ(table.lookup(55), 3); // Last inserted value should persist
}

TEST(LockFreeHashTableTest, FullTableThrowsException)
{
    HashTable table;

    for (int i = 0; i < 1024; ++i)
    {
        EXPECT_TRUE(table.insert(i, i * 10));
    }

    EXPECT_FALSE(table.insert(2048, 9999));
}

TEST(LockFreeHashTableTest, MultiThreadedInsertions)
{
    // repeat 1000 times
    for (int repeat = 0; repeat < 1000; ++repeat)
    {
        HashTable table;
        constexpr int num_threads = 8;
        constexpr int inserts_per_thread = 64;

        std::vector<std::thread> threads;
        for (int thread = 0; thread < num_threads; ++thread)
        {
            threads.emplace_back([&table, thread]()
            {
                for (int i = 0; i < inserts_per_thread; ++i)
                {
                    int key = thread * inserts_per_thread + i;
                    int value = key * 10;
                    table.insert(key, value);
                }
            });
        }

        for (auto& thread : threads)
        {
            thread.join();
        }

        // Now verify all insertions
        for (int thread = 0; thread < num_threads; ++thread)
        {
            for (int i = 0; i < inserts_per_thread; ++i)
            {
                int key = thread * inserts_per_thread + i;
                EXPECT_EQ(table.lookup(key), key * 10);
            }
        }
    }
}

TEST(LockFreeHashTableTest, ChaoticMultiThreadedInsertions) {
    constexpr int num_threads = 8;
    constexpr int inserts_per_thread = 128;

    for (int repeat = 0; repeat < 100; ++repeat) {
        HashTable table;
        std::vector<std::vector<int>> thread_keys(num_threads);
        std::vector<std::thread> threads;
        for (int thread = 0; thread < num_threads; ++thread) {
            threads.emplace_back([&table, thread, &thread_keys]() {
                std::mt19937 rng(std::random_device{}());
                std::uniform_int_distribution<int> sleep_chance(0, 9);

                std::vector<int> keys;
                for (int i = 0; i < inserts_per_thread; ++i) {
                    keys.push_back(thread * inserts_per_thread + i);
                }
                std::shuffle(keys.begin(), keys.end(), rng);
                thread_keys[thread] = keys;

                for (int key : keys) {
                    int value = key * 10;
                    table.insert(key, value);

                    if (sleep_chance(rng) == 0) {
                        std::this_thread::yield();
                        std::this_thread::sleep_for(std::chrono::microseconds(10));
                    }
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        // Verify all insertions
        for (int thread = 0; thread < num_threads; ++thread) {
            for (const auto key : thread_keys[thread]) {
                EXPECT_EQ(table.lookup(key), key * 10);
            }
        }
    }
}

TEST(LockFreeHashTableTest, MultiThreadedLookups) {
    HashTable table;
    constexpr int N = 1000;
    for (int i = 0; i < N; ++i) {
        EXPECT_TRUE(table.insert(i, i*10));
    }

    // Launch threads that all do lookups
    constexpr int num_threads = 8;
    std::vector<std::thread> threads;
    for (int thread = 0; thread < num_threads; ++thread) {
        threads.emplace_back([&](){
            for (int i = 0; i < N; ++i) {
                EXPECT_EQ(table.lookup(i), i*10);
            }
        });
    }
    for (auto &therad: threads)
        therad.join();
}

TEST(LockFreeHashTableTest, ConcurrentInsertsAndLookups) {
    HashTable table;
    constexpr int num_threads = 8;
    constexpr int operations_per_thread = 128;

    std::atomic<bool> ready_insert{false};
    std::atomic<bool> ready_lookup{false};
    std::vector<std::thread> threads;

    for (int thread = 0; thread < num_threads/2; ++thread) {
        threads.emplace_back([&,thread](){
            // wait until
            while(!ready_insert)
                std::this_thread::yield();
            for (int i = 0; i < operations_per_thread; ++i) {
                table.insert(thread*operations_per_thread + i, (thread*operations_per_thread + i)*10);
            }
        });
    }
    ready_insert = true;

    for (int thread = 0; thread < num_threads/2; ++thread) {
        threads.emplace_back([&](){
            while(!ready_lookup)
                std::this_thread::yield();
            // try lookups for the full key range repeatedly
            for (int iter = 0; iter < 10; ++iter) {
                for (int k = 0; k < (num_threads/2)*operations_per_thread; ++k) {
                    // can be NO_VALUE if not yet inserted
                    auto v = table.lookup(k);
                    EXPECT_TRUE(v == HashTable::no_value || v == k*10);
                }
            }
        });
    }

    // start them together
    ready_insert = true;
    // let insret threads run for a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ready_lookup = true;

    for (auto &thread: threads)
        thread.join();

    // finally verify everything was inserted
    for (int key = 0; key < (num_threads/2)*operations_per_thread; ++key) {
        EXPECT_EQ(table.lookup(key), key*10);
    }
}

