//
// Created by andreas on 16.04.23.
//
#include <iostream>
#include <thread>
#include <vector>
#include "thread_safe_stack.h" // assuming the implementation is in thread_safe_stack.h

void push_thread(ThreadSafeStack<int>& stack, int start, int end) {
	for (int i = start; i <= end; ++i) {
		stack.push(i);
	}
}

void pop_thread(ThreadSafeStack<int>& stack, int num_to_pop, std::vector<int>& popped_values) {
	for (int i = 0; i < num_to_pop; ++i) {
		int value;
		if (stack.pop(value)) {
			popped_values.push_back(value);
		}
	}
}

int main() {
	ThreadSafeStack<int> stack;

	const int num_push_threads = 4;
	const int num_pop_threads = 2;
	const int num_values = 10000;

	std::vector<std::thread> push_threads;
	std::vector<std::thread> pop_threads;
	std::vector<std::vector<int>> popped_values(num_pop_threads);

	// Create push threads
	for (int i = 0; i < num_push_threads; ++i) {
		int start = i * num_values / num_push_threads + 1;
		int end = (i + 1) * num_values / num_push_threads;
		push_threads.emplace_back(push_thread, std::ref(stack), start, end);
	}

	// Create pop threads
	for (int i = 0; i < num_pop_threads; ++i) {
		pop_threads.emplace_back(pop_thread, std::ref(stack), num_values / num_pop_threads, std::ref(popped_values[i]));
	}

	// Join push threads
	for (auto& t : push_threads) {
		t.join();
	}

	// Join pop threads
	for (auto& t : pop_threads) {
		t.join();
	}

	// Check if all values were popped
	std::vector<int> all_values(num_values);
	std::iota(all_values.begin(), all_values.end(), 1);
	std::vector<int> popped_values_merged;
	for (auto& values : popped_values) {
		popped_values_merged.insert(popped_values_merged.end(), values.begin(), values.end());
	}
	std::sort(popped_values_merged.begin(), popped_values_merged.end());
	std::sort(all_values.begin(), all_values.end());
	if (popped_values_merged == all_values) {
		std::cout << "All values were popped successfully!" << std::endl;
	} else {
		std::cout << "Some values were not popped!" << std::endl;
	}

	return 0;
}
