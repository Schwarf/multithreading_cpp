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
        // Only producer thread can modify or write to head
        size_t head = head_.load(std::memory_order_relaxed);
        // determeine where the next index points to
        size_t next = (head + 1) % capacity_;
        // check if buffer is full
        if (next == tail_.load(std::memory_order_acquire))
            return false;
        buffer_[head] = item;
        head_.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T& item)
    {
        size_t tail = tail_.load(std::memory_order_relaxed);
        // check if buffer is empty
        if (tail == head_.load(std::memory_order_acquire))
            return false;
        item = buffer_[tail];
        size_t next = (tail + 1) % capacity_;
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