#include <benchmark/benchmark.h>
#include <vector>
#include <functional>
#include <iostream>
#include "./../helpers/read_write_inputs.h" // Declares read_vectors_from_file
#include "merge_sort.h"                     // Declares merge_sort

// Load the predefined random vectors from disk.
// The file should contain a vector of vectors, each with random inputs.
std::vector<std::vector<int>> global_data = read_vectors_from_file("/mnt/linux_data/projects/cpp/multithreading_cpp/helpers/random_vectors.bin");

// This benchmark function runs the provided sort_func on the first three input vectors.
// The sort_func must match the signature: void(std::vector<int>&).
template <typename T>
requires std::is_arithmetic_v<T>
static void generic_sorting_benchmark(benchmark::State& state,
    const std::function<void(std::vector<T>&)> &sort_func,
    const std::vector<std::vector<int>> &global_data)
{
    for (auto _ : state) {
        // Benchmark only the first three inputs.
        for (int i = 0; i < 3; ++i) {
            // Make a copy so each sort gets unsorted data.
            auto data = global_data[i];
            sort_func(data);
            benchmark::ClobberMemory();
        }
    }
}

// Wrap merge_sort into a function that takes only a vector reference.
// Adjust the indices based on whether merge_sort expects an inclusive or exclusive right bound.
void merge_sort_wrapper(std::vector<int>& data) {
    // If your merge_sort expects [left, right) then call:
    merge_sort(data);
    // Otherwise, if it expects [left, right] then use:
    // merge_sort(data, 0, data.size() - 1);
}

// Register the benchmark. We use BENCHMARK_CAPTURE to pass our merge_sort_wrapper and global_data.
BENCHMARK_CAPTURE(generic_sorting_benchmark<int>, merge_sort, merge_sort_wrapper, global_data);

int main(int argc, char** argv) {
#ifdef __clang__
    std::cout << "Compiled with Clang, version: " << __clang_version__ << std::endl;
#else
    std::cout << "Not using Clang!" << std::endl;
#endif

    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
    return 0;
}
