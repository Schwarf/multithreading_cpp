//
// Created by andreas on 10.03.26.
//

// This file does not contain any buffering but a sequentially write-then-read-buffer.
#include <vector>

#include "sample_produce_consumer.h"

int main()
{
    constexpr int iterations = 100;
    std::vector<int> data;
    data.resize(1 << 20);
    for (int i{}; i < iterations; ++i)
    {
        generate_data(data);
        process_data(data);
    }
    return 0;
}
