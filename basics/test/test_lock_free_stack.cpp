//
// Created by andreas on 23.04.23.
//

#include <thread>
#include <numeric>
#include "gtest/gtest.h"
#include "lock_free_stack.h"


TEST(LockFreeStackTest, test_push_and_pop_check_values)
{
	int run_test{1000};
	constexpr int number_of_threads{10};
	constexpr int number_of_operations{100};
	for (int run{}; run < run_test; run++) {

		LockFreeStack<int> stack;
		std::vector<int> input(number_of_operations);
		std::iota(input.begin(), input.end(), 0);
		std::vector<int> result_values;
		std::vector<int> expected_values;
		std::vector<std::thread> threads;
		auto fill_stack_per_thread =
			[&stack, &result_values,  number_of_operations]()
			{
				for (int i{}; i < number_of_operations; ++i) {
					stack.push(i);
					result_values.push_back(*stack.pop());
				}
			};
		for (int i = 0; i < number_of_threads; ++i) {
			threads.emplace_back(fill_stack_per_thread);
			expected_values.insert(expected_values.end(), input.begin(), input.end());
		}

		// Wait for all threads to finish
		for (auto &thread: threads) {
			thread.join();
		}

		// Check that all result_values were pushed and popped

		EXPECT_NE(result_values, expected_values);
		std::sort(result_values.begin(), result_values.end());
		std::sort(expected_values.begin(), expected_values.end());
		EXPECT_EQ(result_values, expected_values);
	}
}
