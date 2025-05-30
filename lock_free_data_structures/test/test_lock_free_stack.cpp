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
    EXPECT_FALSE(stack.pop().has_value());
}

TEST(LockFreeStackTest, ConcurrentPushPop)
{
    constexpr int kNumThreads = 20; // number of pushers and number of poppers
    constexpr int kOpsPerThread = 100000; // ops per thread
    constexpr int kTotalThreads = kNumThreads * 2;

    LockFreeStack<int> stack;

    // Barrier for synchronizing thread start
    std::atomic<int> barrier_count{0};
    auto barrier = [&]()
    {
        barrier_count.fetch_add(1, std::memory_order_acq_rel);
        while (barrier_count.load(std::memory_order_acquire) < kTotalThreads)
        {
            std::this_thread::yield();
        }
    };

    // Counters to verify correctness
    std::atomic<int> push_count{0}, pop_count{0};
    std::atomic<long long> sum_push{0}, sum_pop{0};

    std::vector<std::thread> threads;
    threads.reserve(kTotalThreads);

    // Pusher threads
    for (int thread = 0; thread < kNumThreads; ++thread)
    {
        threads.emplace_back([&, thread]()
        {
            barrier();
            for (int j = 0; j < kOpsPerThread; ++j)
            {
                int val = thread * kOpsPerThread + j;
                stack.push(val);
                sum_push.fetch_add(val, std::memory_order_relaxed);
                push_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // Popper threads
    for (int thread = 0; thread < kNumThreads; ++thread)
    {
        threads.emplace_back([&]()
        {
            barrier();
            while (true)
            {
                // Stop once we've popped everything
                if (pop_count.load(std::memory_order_acquire) >= kNumThreads * kOpsPerThread)
                    break;

                if (auto v = stack.pop())
                {
                    sum_pop.fetch_add(v.value(), std::memory_order_relaxed);
                    pop_count.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }

    // Wait for all threads to finish
    for (auto& thread : threads) thread.join();

    // Verify that all pushes matched all pops and sums agree
    EXPECT_EQ(push_count.load(), kNumThreads * kOpsPerThread);
    EXPECT_EQ(pop_count.load(), kNumThreads * kOpsPerThread);
    EXPECT_EQ(sum_push.load(), pop_count.load() ? sum_pop.load() : 0LL);
}

TEST(LockFreeStackTest, PushPopStress)
{
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

    for (unsigned thread = 0; thread < N; ++thread)
    {
        threads.emplace_back([&, thread]()
        {
            // Seed each thread’s rng differently
            std::mt19937_64 rng(std::random_device{}() + thread);
            while (std::chrono::steady_clock::now() < end)
            {
                if ((rng() & 1) == 0)
                {
                    // push a random value
                    int v = int(rng());
                    stack.push(v);
                    total_pushes.fetch_add(1, std::memory_order_relaxed);
                }
                else
                {
                    // try to pop
                    if (auto v = stack.pop())
                        total_pops.fetch_add(1, std::memory_order_relaxed);
                }
                // occasional back-off to increase interleavings
                if ((rng() & 0xF) == 0)
                {
                    std::this_thread::yield();
                }
            }
        });
    }

    // wait for all threads
    for (auto& thread : threads) thread.join();

    // sanity: you can't pop more than you pushed
    EXPECT_LE(total_pops.load(), total_pushes.load());

    // drain remaining items
    size_t remaining = 0;
    while (auto v = stack.pop())
    {
        ++remaining;
    }

    // all pushes == pops + remaining
    EXPECT_EQ(total_pushes.load(), total_pops.load() + remaining);
}


TEST(LockFreeStackTest, PopTopStress)
{
    LockFreeStack<int> stack;

    // 1) Single‐threaded push
    constexpr int item_count = 100000;
    for (int i = 0; i < item_count; ++i)
    {
        stack.push(i);
    }

    // 2) Now N threads concurrently pop or top
    const unsigned N = std::thread::hardware_concurrency() > 10
                           ? std::thread::hardware_concurrency()
                           : 4u;

    std::atomic<int> total_pops{0};
    std::atomic<int> total_tops{0};
    std::vector<std::thread> threads;
    threads.reserve(N);

    for (unsigned thread_id = 0; thread_id < N; ++thread_id)
    {
        threads.emplace_back([&, thread_id]
        {
            std::mt19937_64 rng(thread_id);
            while (true)
            {
                int popped_so_far = total_pops.load(std::memory_order_relaxed);
                if (popped_so_far >= item_count)
                    break;

                if ((rng() & 1) == 0)
                {
                    // try to pop
                    if (auto v = stack.pop())
                    {
                        total_pops.fetch_add(1, std::memory_order_relaxed);
                    }
                }
                else
                {
                    // peek at top
                    if (auto v = stack.top())
                    {
                        total_tops.fetch_add(1, std::memory_order_relaxed);
                    }
                }

                // occasional back-off
                if ((rng() & 0xF) == 0)
                {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    // 3) Validate
    // We must have popped exactly all pushed items:
    EXPECT_EQ(total_pops.load(), item_count);
    // And since we did tops under contention, we should have seen some:
    EXPECT_GT(total_tops.load(), 0);
    // Finally, stack must now be empty:
    EXPECT_FALSE(stack.pop().has_value());
}


TEST(LockFreeStackTest, MixedPushPopTopStress)
{
    LockFreeStack<int> stack;

    // fixed thread count of 10 (at least 2)
    const unsigned threads_n = std::max(2u, 10u);

    std::atomic<int> pushes{0}, pops{0}, tops{0};
    auto stop_time = std::chrono::steady_clock::now() + std::chrono::seconds(3);

    std::vector<std::thread> threads;
    threads.reserve(threads_n);

    for (unsigned tid = 0; tid < threads_n; ++tid)
    {
        threads.emplace_back([&, tid]
        {
            std::mt19937_64 rng(tid);
            while (std::chrono::steady_clock::now() < stop_time)
            {
                switch (rng() % 3)
                {
                case 0:
                    {
                        // push
                        stack.push(int(rng()));
                        pushes.fetch_add(1, std::memory_order_relaxed);
                        break;
                    }
                case 1:
                    {
                        // pop (destructive)
                        if (auto v = stack.pop())
                            pops.fetch_add(1, std::memory_order_relaxed);
                    }
                default:
                    {
                        // peek at top (non‐destructive)
                        if (auto v = stack.top())
                            tops.fetch_add(1, std::memory_order_relaxed);
                    }
                }
                // occasional back-off
                if ((rng() & 0xF) == 0)
                {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    // you can't pop more than you pushed
    EXPECT_LE(pops.load(), pushes.load());

    // drain remaining items
    int remaining = 0;
    while (auto v = stack.pop())
    {
        ++remaining;
    }
    EXPECT_EQ(pushes.load(), pops.load() + remaining);

    // we should have seen at least one top()
    EXPECT_GT(tops.load(), 0);
}
