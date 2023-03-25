//
// Created by andreas on 25.03.23.
//
#include "binary_search.h"
#include <ranges>
#include <iostream>
int main()
{
	int min{1};
	int max{10000000};
	std::vector<int> simple_input(std::ranges::views::iota(min, max).begin(), std::ranges::views::iota(min, max).end());
//	for(const auto & element: simple_input)
//		std::cout << element << std::endl;
	int target{9099090};
	int detected_number_of_processors = std::thread::hardware_concurrency();
	std::cout << "Number of threads: " << detected_number_of_processors << std::endl;
	std::cout << parallel_binary_search(simple_input, min, max, target, detected_number_of_processors) << std::endl;



}