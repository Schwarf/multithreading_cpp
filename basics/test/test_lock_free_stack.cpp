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
            const int base_value = thread_count * operations;
            for (int i = 0; i < operations; ++i)
            {
                stack.push(base_value + i);
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

    std::ranges::sort(results);
    std::ranges::sort(expected);
    EXPECT_EQ(results, expected);
}


//–– Single‐threaded correctness of push / top / pop ––
TEST(LockFreeStackTest, SingleThreadPushPop) {
    LockFreeStack<int> stack;
    // empty → top() == nullptr
    EXPECT_EQ(stack.pop(), nullptr);

    // push 1,2 and observe LIFO via top()/pop()
    stack.push(1);
    stack.push(2);

    auto pop_ptr = stack.pop();
    EXPECT_NE(pop_ptr, nullptr);
    EXPECT_EQ(*pop_ptr, 2);

    pop_ptr = stack.pop();
    EXPECT_NE(pop_ptr, nullptr);
    EXPECT_EQ(*pop_ptr, 1);

    // now empty again
    EXPECT_EQ(stack.pop(), nullptr);
}


//–– Multiple producers -> single‐threaded pop + top check ––
TEST(LockFreeStackTest, MultiProducerPopAndTop) {
    LockFreeStack<int> stack;
    constexpr int producer_count = 4;
    constexpr int operations = 1000;
    constexpr int total_items = producer_count * operations;

    // launch concurrent producers
    std::vector<std::thread> threads;
    for (int thread = 0; thread < producer_count; ++thread) {
        threads.emplace_back([&, thread] {
            const int base_value = thread * operations;
            for (int i = 0; i < operations; ++i) {
                stack.push(base_value + i);
            }
        });
    }
    for (auto &thread : threads) thread.join();

    // now pop everything in a single thread
    std::vector<int> results;
    results.reserve(total_items);
    for (int i = 0; i < total_items; ++i) {
        auto pop_ptr = stack.pop();
        EXPECT_NE(pop_ptr, nullptr);
        results.push_back(*pop_ptr);
    }

    // stack must now be empty
    // EXPECT_EQ(stack.top(), nullptr);
    EXPECT_EQ(stack.pop(), nullptr);

    // verify we got exactly the multiset {0,1,...,TOTAL-1}
    std::ranges::sort(results);
    for (int i = 0; i < total_items; ++i) {
        EXPECT_EQ(results[i], i);
    }
}

TEST(LockFreeStackTest, PushPopStress) {
    LockFreeStack<int> stack;
    const unsigned N = std::thread::hardware_concurrency() ?
                       std::thread::hardware_concurrency()-10 : 4u;
    const auto end = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    std::cout << N << std::endl;
    std::atomic<int> total_pushes{0};
    std::atomic<int> total_pops{0};
    std::vector<std::thread> threads;
    threads.reserve(N);

    for (unsigned thread = 0; thread < N; ++thread) {
        threads.emplace_back([&, thread]{
            std::mt19937_64 rng(thread);
            while (std::chrono::steady_clock::now() < end) {
                if ((rng() & 1) == 0) {
                    // push a random value
                    stack.push(int(rng()));
                    ++total_pushes;
                } else {
                    // try to pop
                    if (stack.pop()) {
                        ++total_pops;
                    }
                }
                // occasional back-off to increase interleavings
                if ((rng() & 0xF) == 0) {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto &thread : threads) thread.join();
    std::cout << total_pushes.load() << std::endl;
    std::cout << total_pops.load() << std::endl;
    // sanity: you can't pop more than you pushed
    EXPECT_LE(total_pops.load(), total_pushes.load());

    // drain remaining items
    int remaining = 0;
    while (stack.pop()) {
        ++remaining;
    }
    std::cout << remaining << std::endl;
    // all pushes == pops + remaining
    EXPECT_EQ(total_pushes.load(), total_pops.load() + remaining);
}

// TEST(LockFreeStackTest, PushTopStress) {
//     LockFreeStack<int> stack;
//
//     const unsigned N = std::thread::hardware_concurrency() ?
//                        std::thread::hardware_concurrency()-10 : 4u;
//
//     // Run for 3 seconds
//     auto end = std::chrono::steady_clock::now() + std::chrono::seconds(3);
//
//     std::atomic<int> total_pushes{0}, total_tops{0};
//     std::vector<std::thread> threads;
//     threads.reserve(N);
//
//     // Each thread randomly pushes or calls top()
//     for (unsigned int thread = 0; thread < N; ++thread) {
//         threads.emplace_back([&, thread]{
//             std::mt19937_64 rng(thread);
//             while (std::chrono::steady_clock::now() < end) {
//                 if ((rng() & 1) == 0) {
//                     // push a random value
//                     stack.push(int(rng()));
//                     ++total_pushes;
//                 } else {
//                     // peek at the top (non-destructive)
//                     if (stack.top()) {
//                         ++total_tops;
//                     }
//                 }
//                 // occasional back-off
//                 if ((rng() & 0xF) == 0) {
//                     std::this_thread::yield();
//                 }
//             }
//         });
//     }
//
//     // Wait for all threads
//     for (auto &thread : threads) thread.join();
//
//     // We should have done at least one push and one top
//     EXPECT_GT(total_pushes.load(), 0u);
//     EXPECT_GT(total_tops.load(), 0u);
//
//     // Now pop everything and count how many were pushed
//     int remaining = 0;
//     while (stack.pop()) {
//         ++remaining;
//     }
//
//     // Since top() never removed items, the total number popped must equal pushes
//     EXPECT_EQ(remaining, total_pushes.load());
// }

// TEST(LockFreeStackTest, PopTopStress) {
//     LockFreeStack<int> stack;
//     const unsigned N = std::thread::hardware_concurrency() ?
//                        std::thread::hardware_concurrency()-10 : 4u;
//
//     // 1) Single‐threaded push
//     constexpr int item_count = 100000;
//     for (int i = 0; i < item_count; ++i) {
//         stack.push(i);
//     }
//
//     // 2) Now N threads concurrently pop or top
//
//     std::atomic<int> total_pops{0}, total_tops{0};
//     std::vector<std::thread> threads;
//     threads.reserve(N);
//
//     for (unsigned int thread = 0; thread < N; ++thread) {
//         threads.emplace_back([&, thread]{
//             std::mt19937_64 rng(thread);
//             while (true) {
//                 int popped_so_far = total_pops.load(std::memory_order_relaxed);
//                 if (popped_so_far >= item_count)
//                     break;
//
//                 if ((rng() & 1) == 0) {
//                     // try to pop
//                     if (stack.pop()) {
//                         ++total_pops;
//                     }
//                 } else {
//                     // peek at top
//                     if (stack.top()) {
//                         ++total_tops;
//                     }
//                 }
//
//                 // occasional back-off
//                 if ((rng() & 0xF) == 0) {
//                     std::this_thread::yield();
//                 }
//             }
//         });
//     }
//
//     for (auto &thread : threads) thread.join();
//
//     // 3) Validate
//     // We must have popped exactly all pushed items:
//     EXPECT_EQ(total_pops.load(), item_count);
//
//     // And since we did tops under contention, we should have seen some:
//     EXPECT_GT(total_tops.load(), 0);
//
//     // Finally, stack must now be empty:
//     EXPECT_EQ(stack.pop(), nullptr);
// }


// TEST(LockFreeStackTest, MixedPushPopTopStress) {
//     LockFreeStack<int> stack;
//     auto threads_n = std::max(2u, 10u);
//
//     std::atomic<int> pushes{0}, pops{0}, tops{0};
//     auto stop_time = std::chrono::steady_clock::now() + std::chrono::seconds(3);
//
//     std::vector<std::thread> threads;
//     threads.reserve(threads_n);
//
//     for (unsigned i = 0; i < threads_n; ++i) {
//         threads.emplace_back([&, stop_time, i] {
//             std::mt19937_64 rng(i);
//             while (std::chrono::steady_clock::now() < stop_time) {
//                 switch (rng() % 3) {
//                   case 0:
//                       stack.push(int(rng()));
//                       ++pushes;
//                       break;
//                     case 1:
//                         if (stack.pop()) ++pops;
//                         break;
//                       default:
//                           if (stack.top()) ++tops;
//                           break;
//                       }
//                       if ((rng() & 0xF) == 0)
//                           std::this_thread::yield();
//                   }
//               });
//     }
//
//     for (auto& t : threads) t.join();
//
//     EXPECT_LE(pops.load(), pushes.load());
//
//     int remaining = 0;
//     while (stack.pop()) ++remaining;
//     EXPECT_EQ(pushes.load(), pops.load() + remaining);
//
//     // It's vanishingly unlikely to be zero, but if you really need to guarantee:
//     EXPECT_GT(tops.load(), 0);
// }
