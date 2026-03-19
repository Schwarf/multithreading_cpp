//
// Created by andreas on 10.03.26.
//

#ifndef LEARN_MULTITHREADING_COMMON_H
#define LEARN_MULTITHREADING_COMMON_H
#include <span>
#include <random>

// void generate_data(std::span<int> data)
// {
//     std::random_device rd;
//     std::mt19937_64 gen(rd());
//     std::uniform_int_distribution<int> dist(0, 100);
//     for (int i = 0; i < data.size(); ++i)
//     {
//         data[i] = dist(gen);
//     }
// }
//
//
// void process_data(std::span<int> data)
// {
//     for (int i{}; i < 5; ++i)
//     {
//         for (auto& element : data)
//         {
//             element %= 1 + element;
//         }
//     }
// }
constexpr int iterations = 10000000;

void generate_data(std::span<int> data)
{
    static thread_local uint32_t state = 123456789u;

    for (auto &x : data)
    {
        state = state * 1664525u + 1013904223u;
        x = static_cast<int>(state & 0x7fffffff);
    }
}

void process_data(std::span<int> data)
{
    for (int i = 0; i < 5; ++i)
    {
        for (auto &x : data)
        {
            x = (x ^ (x >> 3)) + (x << 1);
        }
    }
}

#endif //LEARN_MULTITHREADING_COMMON_H