//
// Created by andreas on 19.03.26.
//
#include <condition_variable>
#include <jthread>
#include <mutex>
#include <vector>

#include "sample_produce_consumer.h"
int main()
{
    std::vector<int> data1(1 << 6);
    std::vector<int> data2(1 << 6);

    std::mutex mtx;
    std::condition_variable cv;

    // true  -> processor may process the published buffer
    // false -> generator may generate/swap the next buffer
    bool data_ready = false;

    constexpr int iterations{100000};

    auto data_generator = [&]()
    {
        for (int i = 0; i < iterations; ++i)
        {
            // Generate outside the lock so processing can happen in parallel.
            generate_data(data1);

            std::unique_lock lock(mtx);

            // Wait until the processor has finished the previous round.
            cv.wait(lock, [&]() { return !data_ready; });

            // Publish the newly generated data.
            data1.swap(data2);

            // Signal that fresh data is now ready for processing.
            data_ready = true;

            lock.unlock();
            cv.notify_one();
        }
    };

    auto data_processor = [&]()
    {
        for (int i = 0; i < iterations; ++i)
        {
            std::unique_lock lock(mtx);

            // Wait until the generator has published fresh data.
            cv.wait(lock, [&]() { return data_ready; });

            // At this point, data2 is the published buffer.
            lock.unlock();

            process_data(data2);

            lock.lock();

            // Mark processing as finished so the generator may continue.
            data_ready = false;

            lock.unlock();
            cv.notify_one();
        }
    };

    std::jthread generator_thread(data_generator);
    std::jthread processor_thread(data_processor);

    return 0;
}