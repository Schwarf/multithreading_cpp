#include <benchmark/benchmark.h>
#include <vector>
#include <algorithm>
#include <functional>
#include <random>
#include <iostream>
#include "merge_sort.h"

template <typename T>
requires std::is_arithmetic_v<T>
static void generic_sorting_benchmark(benchmark::State& state, const std::function<void(std::vector<T>&)> & sort_func) {
    std::vector<T> data(state.range(0));
    for (auto _ : state) {
        int seed = static_cast<int>(state.range(0)) * static_cast<int>(state.iterations());
        std::mt19937 generator(seed);
        std::uniform_int_distribution<T> distribution(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max());

        std::generate(data.begin(), data.end(), [&]() { return distribution(generator); });


        sort_func(data); // Call the provided sorting function

        benchmark::ClobberMemory();
    }
}



BENCHMARK_CAPTURE(generic_sorting_benchmark<int>, merge_sort, merge_sort<int>)->RangeMultiplier(10)->Range(1000, 100000);

int main(int argc, char** argv) {
    // Compiler check
#ifdef __clang__
    std::cout << "Compiled with Clang, version: " << __clang_version__ << std::endl;
#else
    std::cout << "Not using Clang!" << std::endl;
#endif

    // Initialize and run benchmarks
    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
    return 0;
}