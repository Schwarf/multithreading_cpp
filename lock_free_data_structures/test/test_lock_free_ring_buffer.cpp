//
// Created by andreas on 15.03.26.
//

#include <random>
#include <thread>

#include "./../lock_free_ring_buffer.h"
#include "gtest/gtest.h"

// Basic tests no threading
TEST(RingBufferTest, PushPopSingleElement)
{
    RingBuffer<int> buffer(4);

    EXPECT_TRUE(buffer.push(42));

    int value{};
    EXPECT_TRUE(buffer.pop(value));

    EXPECT_EQ(value, 42);
}

TEST(RingBufferTest, MaintainsFIFOOrder)
{
    RingBuffer<int> buffer(8);

    for (int i = 0; i < 5; ++i)
        EXPECT_TRUE(buffer.push(i));

    for (int i = 0; i < 5; ++i)
    {
        int value{};
        EXPECT_TRUE(buffer.pop(value));
        EXPECT_EQ(value, i);
    }
}

TEST(RingBufferTest, PopEmptyBuffer)
{
    RingBuffer<int> buffer(4);

    int value{};
    EXPECT_FALSE(buffer.pop(value));
}

TEST(RingBufferTest, DetectsFullBuffer)
{
    RingBuffer<int> buffer(4);

    EXPECT_TRUE(buffer.push(1));
    EXPECT_TRUE(buffer.push(2));
    EXPECT_TRUE(buffer.push(3));

    EXPECT_FALSE(buffer.push(4)); // buffer full
}

TEST(RingBufferTest, WrapAroundWorks)
{
    RingBuffer<int> buffer(4);

    EXPECT_TRUE(buffer.push(1));
    EXPECT_TRUE(buffer.push(2));
    EXPECT_TRUE(buffer.push(3));

    int value{};
    EXPECT_TRUE(buffer.pop(value));
    EXPECT_EQ(value, 1);

    EXPECT_TRUE(buffer.push(4));

    EXPECT_TRUE(buffer.pop(value));
    EXPECT_EQ(value, 2);

    EXPECT_TRUE(buffer.pop(value));
    EXPECT_EQ(value, 3);

    EXPECT_TRUE(buffer.pop(value));
    EXPECT_EQ(value, 4);
}

// Real use case with multiple threads

TEST(RingBufferTest, SingleProducerSingleConsumer)
{
    const int N = 100000;

    RingBuffer<int> buffer(1024);

    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};

    std::thread producer([&]()
    {
        for (int i = 0; i < N; ++i)
        {
            while (!buffer.push(i)) {}
            produced++;
        }
    });

    std::thread consumer([&]()
    {
        int value{};
        for (int i = 0; i < N; ++i)
        {
            while (!buffer.pop(value)) {}
            consumed++;
        }
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(produced.load(), N);
    EXPECT_EQ(consumed.load(), N);
}