//
// Created by andreas on 24.03.23.
//

#ifndef BINARY_SEARCH_H
#define BINARY_SEARCH_H
#include <thread>
#include <vector>

template<typename T>
int binary_search(const std::vector<T> &input, int left, int right, const T & target)
{
	while (left <= right) {
		int mid = (left + right) / 2;
		if (input[mid] == target)
			return mid;
		if (input[mid] < target)
			left = mid + 1;
		if (input[mid] > target)
			right = mid - 1;
	}
}

template<typename T>
int parallel_binart_search(const std::vector<T> &input, int left, int right, const T & target, int number_of_threads)
{
	int index{-1};
	std::vector<std::thread> threads;
	std::vector<T> results(number_of_threads, {});
	for (int i{}; i < number_of_threads; ++i)
	{

	}
}

#endif //BINARY_SEARCH_H
