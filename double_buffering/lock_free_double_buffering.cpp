#include <atomic>
#include <thread>
#include <vector>

#include "sample_produce_consumer.h"

struct LockFreeDoubleBuffer
{
    std::vector<int> buffers[2];
    std::atomic<int> index{0};
};

int main()
{
    constexpr int iterations = 10000000;

    LockFreeDoubleBuffer db;
    db.buffers[0].resize(1 << 6);
    db.buffers[1].resize(1 << 6);

    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};

    auto producer = [&]()
    {
        while (true)
        {
            int p = produced.fetch_add(1);
            if (p >= iterations)
                break;

            int read = db.index.load(std::memory_order_acquire);
            int write = 1 - read;

            generate_data(db.buffers[write]);

            db.index.store(write, std::memory_order_release);
        }
    };

    auto consumer = [&]()
    {
        int last_seen = -1;

        while (consumed < iterations)
        {
            int idx = db.index.load(std::memory_order_acquire);

            if (idx != last_seen)
            {
                process_data(db.buffers[idx]);
                last_seen = idx;
                consumed++;
            }
        }
    };

    std::jthread p(producer);
    std::jthread c(consumer);
}