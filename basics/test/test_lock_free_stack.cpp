//
// Created by andreas on 23.04.23.
//


#include <gtest/gtest.h>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <random>
#include <thread>
#include <vector>
#include "lock_free_stack.h"

TEST(LockFreeStackTest, MultiProducerMultiConsumer)
{
    LockFreeStack<int> stack;
    constexpr int producer_count = 4;
    constexpr int consumer_count = 4;
    constexpr int operations = 1000;
    constexpr int item_count = producer_count * operations;

    // 1) Launch producers
    std::vector<std::thread> producers;
    for (int thread_count = 0; thread_count < producer_count; ++thread_count)
    {
        producers.emplace_back([&, thread_count]
        {
            int base = thread_count * operations;
            for (int i = 0; i < operations; ++i)
            {
                stack.push(base + i);
            }
        });
    }

    // 2) Consumers collect results under a mutex
    std::vector<int> results;
    results.reserve(item_count);
    std::mutex results_mutex;
    std::atomic<int> consumed_count{0};

    std::vector<std::thread> consumers;
    for (int c = 0; c < consumer_count; ++c)
    {
        consumers.emplace_back([&]
        {
            while (true)
            {
                auto shared_ptr = stack.pop();
                if (!shared_ptr)
                {
                    // stack is empty *right now*; check if we're done
                    if (consumed_count.load() >= item_count)
                        break;
                    // otherwise spin/wait a bit
                    std::this_thread::yield();
                    continue;
                }
                int value = *shared_ptr;
                {
                    std::lock_guard<std::mutex> lg(results_mutex);
                    results.push_back(value);
                }
                ++consumed_count;
            }
        });
    }

    // 3) Wait for everyone
    for (auto& thread : producers)
        thread.join();
    for (auto& thread : consumers)
        thread.join();

    // 4) Validate
    EXPECT_EQ(results.size(), item_count);

    std::vector<int> expected;
    expected.reserve(item_count);
    for (int p = 0; p < producer_count; ++p)
        for (int i = 0; i < operations; ++i)
            expected.push_back(p * operations + i);

    std::sort(results.begin(), results.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(results, expected);
}

//–– Single‐threaded correctness of push / top / pop ––
TEST(SharedPtrStackTest, SingleThreadTopPop) {
    LockFreeStack<int> stack;
    // empty → top() == nullptr
    EXPECT_EQ(stack.top(), nullptr);

    // push 1,2 and observe LIFO via top()/pop()
    stack.push(1);
    {
        auto top_ptr = stack.top();
        EXPECT_NE(top_ptr, nullptr);
        EXPECT_EQ(*top_ptr, 1);
    }

    stack.push(2);
    {
        auto top_ptr = stack.top();
        EXPECT_NE(top_ptr, nullptr);
        EXPECT_EQ(*top_ptr, 2);
    }

    auto pop_ptr = stack.pop();
    EXPECT_NE(pop_ptr, nullptr);
    EXPECT_EQ(*pop_ptr, 2);

    {
        auto top_ptr = stack.top();
        EXPECT_NE(top_ptr, nullptr);
        EXPECT_EQ(*top_ptr, 1);
    }

    pop_ptr = stack.pop();
    EXPECT_NE(pop_ptr, nullptr);
    EXPECT_EQ(*pop_ptr, 1);

    // now empty again
    EXPECT_EQ(stack.top(), nullptr);
    EXPECT_EQ(stack.pop(), nullptr);
}
