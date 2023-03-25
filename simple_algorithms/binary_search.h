//
// Created by andreas on 24.03.23.
//

#ifndef BINARY_SEARCH_H
#define BINARY_SEARCH_H
#include <thread>
#include <vector>

template<typename T>
int binary_search(const std::vector<T> &input, int left, int right, const T &target) requires std::is_integral_v<T>
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
	return input[right] == target? right : -1;
}

template<typename T>
int parallel_binary_search(const std::vector<T> &input,
						   int left,
						   int right,
						   const T &target,
						   int number_of_threads) requires std::is_integral_v<T>
{
	int index{-1};
	std::vector<std::thread> threads;
	std::vector<T> results(number_of_threads, {});
	for (int i{}; i < number_of_threads; ++i) {
		threads.emplace_back([&, i]() {
			int start = left + i * ((right - left+ 1) / number_of_threads);
			int end = i == number_of_threads - 1 ? right : start + ((right - left + 1) / number_of_threads) - 1;
			results[i] = binary_search(input, start, end, target);
		});

	}
	for (auto& t : threads) {
		t.join(); // Wait for all threads to finish
	}

	// Check results to find the first index where the key is found
	for (int i = 0; i < number_of_threads; i++) {
		if (results[i] != -1) {
			index = results[i];
			break;
		}
	}

	return index;

}

#endif //BINARY_SEARCH_H
