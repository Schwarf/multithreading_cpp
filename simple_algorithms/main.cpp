//
// Created by andreas on 25.03.23.
//
#include <ranges>
#include <iostream>
#include "merge_sort.h"
template<typename Func, typename... Args>
void execute_i_times(int i, Func func, Args... args) {
	for (int j = 0; j < i; j++) {
		func(args...);
	}
}
int main()
{
	long long min{-313};
	long long max{215};
	std::vector<long long> simple_input(std::ranges::views::iota(min, max).begin(), std::ranges::views::iota(min, max).end());
	long long target{80913};

	std::vector<long long> simple_input2(std::ranges::views::iota(min, max).begin(), std::ranges::views::iota(min, max).end());
	int detected_number_of_processors = std::thread::hardware_concurrency();
	std::cout << "Number of threads: " << detected_number_of_processors << std::endl;
	std::cout << simple_input.size() << ", " << simple_input2.size() << std::endl;

}