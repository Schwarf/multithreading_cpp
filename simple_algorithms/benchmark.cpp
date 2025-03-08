#include <benchmark/benchmark.h>
#include <vector>
#include <algorithm>
#include <functional>
#include <random>
#include <iostream>

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