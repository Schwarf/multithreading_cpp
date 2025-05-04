//
// Created by andreas on 04.05.25.
//
#include "./../lock_free_stack.h"
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <random>

TEST(LockFreeStackTest, EmptyStackThrows)
{
    LockFreeStack<int> stack;
    EXPECT_THROW(stack.topAndPop(), std::out_of_range);
}

TEST(LockFreeStackStressTest, ConcurrentPushPop) {
    constexpr int kNumThreads     = 4;         // number of pushers and number of poppers
    constexpr int kOpsPerThread   = 100000;    // ops per thread
    constexpr int kTotalThreads   = kNumThreads * 2;

    LockFreeStack<int> stack;

    // Barrier for synchronizing thread start
    std::atomic<int> barrier_count{0};
    auto barrier = [&](){
        barrier_count.fetch_add(1, std::memory_order_acq_rel);
        while (barrier_count.load(std::memory_order_acquire) < kTotalThreads) {
            std::this_thread::yield();
        }
    };

    // Counters to verify correctness
    std::atomic<int> push_count{0}, pop_count{0};
    std::atomic<long long> sum_push{0}, sum_pop{0};

    std::vector<std::thread> threads;
    threads.reserve(kTotalThreads);

    // Pusher threads
    for (int thread = 0; thread < kNumThreads; ++thread) {
        threads.emplace_back([&, thread]() {
            barrier();
            for (int j = 0; j < kOpsPerThread; ++j) {
                int val = thread * kOpsPerThread + j;
                stack.push(val);
                sum_push.fetch_add(val, std::memory_order_relaxed);
                push_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // Popper threads
    for (int thread = 0; thread < kNumThreads; ++thread) {
        threads.emplace_back([&]() {
            barrier();
            while (true) {
                // Stop once we've popped everything
                if (pop_count.load(std::memory_order_acquire) >= kNumThreads * kOpsPerThread)
                    break;

                try {
                    int v = stack.topAndPop();
                    sum_pop.fetch_add(v, std::memory_order_relaxed);
                    pop_count.fetch_add(1, std::memory_order_relaxed);
                } catch (const std::out_of_range&) {
                    // Stack empty right now — retry
                    std::this_thread::yield();
                }
            }
        });
    }

    // Wait for all threads to finish
    for (auto& thread : threads) thread.join();

    // Verify that all pushes matched all pops and sums agree
    EXPECT_EQ(push_count.load(), kNumThreads * kOpsPerThread);
    EXPECT_EQ(pop_count.load(),  kNumThreads * kOpsPerThread);
    EXPECT_EQ(sum_push.load(), pop_count.load() ? sum_pop.load() : 0LL);
}

TEST(LockFreeStackTest, PushPopStress) {
    LockFreeStack<int> stack;

    // # of threads: hardware_concurrency()-1, or fallback to 4
    const unsigned N = std::thread::hardware_concurrency()
                       ? std::thread::hardware_concurrency() - 1
                       : 4u;

    // Run for 5 seconds
    const auto end = std::chrono::steady_clock::now() + std::chrono::seconds(5);

    std::atomic<size_t> total_pushes{0};
    std::atomic<size_t> total_pops{0};

    std::vector<std::thread> threads;
    threads.reserve(N);

    for (unsigned thread = 0; thread < N; ++thread) {
        threads.emplace_back([&, thread]() {
            // Seed each thread’s rng differently
            std::mt19937_64 rng(std::random_device{}() + thread);
            while (std::chrono::steady_clock::now() < end) {
                if ((rng() & 1) == 0) {
                    // push a random value
                    int v = int(rng());
                    stack.push(v);
                    total_pushes.fetch_add(1, std::memory_order_relaxed);
                } else {
                    // try to pop
                    try {
                        stack.topAndPop();
                        total_pops.fetch_add(1, std::memory_order_relaxed);
                    } catch (const std::out_of_range&) {
                        // empty right now — ignore and retry
                    }
                }
                // occasional back-off to increase interleavings
                if ((rng() & 0xF) == 0) {
                    std::this_thread::yield();
                }
            }
        });
    }

    // wait for all threads
    for (auto &thread : threads) thread.join();

    // sanity: you can't pop more than you pushed
    EXPECT_LE(total_pops.load(), total_pushes.load());

    // drain remaining items
    size_t remaining = 0;
    while (true) {
        try {
            stack.topAndPop();
            ++remaining;
        } catch (const std::out_of_range&) {
            break;
        }
    }

    // all pushes == pops + remaining
    EXPECT_EQ(total_pushes.load(), total_pops.load() + remaining);
}