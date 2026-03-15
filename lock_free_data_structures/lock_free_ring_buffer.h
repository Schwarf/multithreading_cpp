//
// Created by andreas on 15.03.26.
//

#ifndef LEARN_MULTITHREADING_LOCK_FREE_RING_BUFFER_H
#define LEARN_MULTITHREADING_LOCK_FREE_RING_BUFFER_H
#include <atomic>
#include <vector>

template <typename T>
class RingBuffer
{
public:
    explicit RingBuffer(size_t capacity):
        buffer_(capacity),  capacity_(capacity){}

    bool push(const T& item)
    {
        // Only producer thread can modify or write to head_
        // The consumer thread only reads it.
        size_t head = head_.load(std::memory_order_relaxed);
        // Compute the next write position (circular wrap-around)
        size_t next = (head + 1) % capacity_;
        // If advancing head would collide with tail, the buffer is full
        if (next == tail_.load(std::memory_order_acquire))
            return false;
        // Write the element before publishing the new head position.
        // The release store ensures the consumer sees this write.
        buffer_[head] = item;
        head_.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T& item)
    {
        // Only the consumer thread modifies tail_ (read index)
        // The producer thread only reads it to check whether the buffer is full.
        size_t tail = tail_.load(std::memory_order_relaxed);
        // Load the producer's head index.
        // The acquire ensures we observe the producer's release store to head_,
        // which guarantees that the written element in buffer_ is visible.
        if (tail == head_.load(std::memory_order_acquire))
            return false;
        // Read the element from the current tail position.
        item = buffer_[tail];
        // Advance the read position (circular wrap-around).
        size_t next = (tail + 1) % capacity_;
        // Publish the new tail position so the producer can reuse the slot.
        tail_.store(next, std::memory_order_release);
        return true;
    }

private:
    std::vector<T> buffer_;
    std::atomic<size_t> head_{};
    std::atomic<size_t> tail_{};
    size_t capacity_;
};

#endif //LEARN_MULTITHREADING_LOCK_FREE_RING_BUFFER_H