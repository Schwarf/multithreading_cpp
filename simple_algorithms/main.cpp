//
// Created by andreas on 25.03.23.
//
#include "binary_search.h"
#include <ranges>
#include <iostream>
int main()
{
	long long min{-3131313};
	long long max{2152921};
	std::vector<long long> simple_input(std::ranges::views::iota(min, max).begin(), std::ranges::views::iota(min, max).end());
	long long target{80913};
	int detected_number_of_processors = std::thread::hardware_concurrency();
	std::cout << "Number of threads: " << detected_number_of_processors << std::endl;
	std::cout << binary_search(simple_input, 0, simple_input.size()-1, target) << std::endl;
	std::cout << parallel_binary_search(simple_input, 0, simple_input.size()-1, target, detected_number_of_processors) << std::endl;



}