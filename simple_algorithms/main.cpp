//
// Created by andreas on 25.03.23.
//
#include "binary_search.h"
#include <ranges>
#include <iostream>
#include <random>
#include "merge_sort.h"
int main()
{
	long long min{-313131389};
	long long max{215292121};
	std::vector<long long> simple_input(std::ranges::views::iota(min, max).begin(), std::ranges::views::iota(min, max).end());
	long long target{80913};
	int detected_number_of_processors = std::thread::hardware_concurrency();
	std::cout << "Number of threads: " << detected_number_of_processors << std::endl;
	std::cout << binary_search(simple_input, 0, simple_input.size()-1, target) << std::endl;
	std::cout << parallel_binary_search(simple_input, 0, simple_input.size()-1, target, detected_number_of_processors) << std::endl;

/*
	int min{0};
	int max{1000};
	std::vector<int> vec(std::ranges::views::iota(min, max).begin(), std::ranges::views::iota(min, max).end());
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(vec.begin(), vec.end(), g);
	int numThreads = 24;

	// Sort the vector using up to numThreads threads
	merge_sort(vec, 0, vec.size() - 1, numThreads);

	// Print the sorted vector
	for (int i = 0; i < vec.size(); i++) {
		std::cout << vec[i] << " ";
	}
	std::cout << std::endl;

	return 0;
*/

}