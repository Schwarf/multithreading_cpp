//
// Created by andreas on 10.03.26.
//

// This file does not contain any buffering but a sequentially write-then-read-buffer.
#include <span>
#include <vector>
#include <random>

void generate_data(std::span<int> data)
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int> dist(0, 100);
    for (int i = 0; i < data.size(); ++i)
    {
        data[i] = dist(gen);
    }
}


void process_data(std::span<int> data)
{
    for (int i{}; i < 5; ++i)
    {
        for (auto& element : data)
        {
            element %= 1 + element;
        }
    }
}


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
