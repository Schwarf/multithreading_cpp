//
// Created by andreas on 10.03.26.
//
#include <vector>
#include <thread>

#include "sample_produce_consumer.h"

int main()
{
    std::vector<int> data1;
    std::vector<int> data2;
    data1.resize(1 << 6);
    data2.resize(1 << 6);
    std::binary_semaphore signal_start_processing_data{0};
    std::binary_semaphore signal_start_generating_data{1};
    constexpr int iterations{10000000};
    auto data_generator = [&]()
    {
        for(int i{}; i < iterations; ++i)
        {
            generate_data(data1);
            // Wait until current processing work is done
            signal_start_generating_data.acquire();
            data1.swap(data2);
            signal_start_processing_data.release();
        }
    };
    auto data_processor = [&]()
    {
        for(int i{}; i < iterations; ++i)
        {
            // Wait until the current data generation is done
            signal_start_processing_data.acquire();
            process_data(data2);
            signal_start_generating_data.release();
        }
    };

    std::jthread data_generator_thread(data_generator);
    std::jthread data_processor_thread(data_processor);
    return 0;
}