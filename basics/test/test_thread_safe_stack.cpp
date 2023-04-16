//
// Created by andreas on 16.04.23.
//

#include <thread>
#include <numeric>
#include <random>
#include "gtest/gtest.h"
#include "thread_safe_stack.h"


TEST(SetupThreadSafeStackTest, test_push_and_pop)
{
	int run_test{100};
	constexpr int number_of_threads{10};
	constexpr int number_of_operations{10000};
	for (int run{}; run < run_test; run++) {
		ThreadSafeStack<int> stack;
		std::vector<std::thread> threads;
		for (int i = 0; i < number_of_threads; ++i) {
			threads.emplace_back([&stack, number_of_operations]()
								 {
									 for (int j = 0; j < number_of_operations; ++j) {
										 int value;
										 stack.push(j);
										 ASSERT_TRUE(stack.pop(value));
									 }
								 });
		}
		for (auto &thread: threads) {
			thread.join();
		}
		int value;
		ASSERT_FALSE(stack.pop(value));
	}
}

TEST(ThreadSafeStackTest, test_push_and_pop_check_values)
{
	int run_test{100};
	constexpr int number_of_threads{10};
	constexpr int number_of_operations{1000};
	for (int run{}; run < run_test; run++) {

		ThreadSafeStack<int> stack;
		std::vector<int> input(number_of_operations);
		std::iota(input.begin(), input.end(), 0);
		std::vector<int> result_values;
		std::vector<int> expected_values;
		std::vector<std::thread> threads;
		std::mutex result_mutex;
		auto fill_stack_per_thread =
			[&stack, &result_values, &result_mutex, number_of_operations]()
			{
				for (int i{}; i < number_of_operations; ++i) {
					stack.push(i);
					int value;
					bool popped = stack.pop(value);
					ASSERT_TRUE(popped);
					std::lock_guard<std::mutex> lock(result_mutex);
					result_values.push_back(value);
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

		ASSERT_NE(result_values, expected_values);
		std::sort(result_values.begin(), result_values.end());
		std::sort(expected_values.begin(), expected_values.end());
		ASSERT_EQ(result_values, expected_values);
	}
}


TEST(SetupThreadSafeStackTest, test_push_and_size)
{
	std::random_device random_device;
	std::default_random_engine random_engine(random_device());
	std::uniform_int_distribution<int> random_distribution(0, 100);
	constexpr int number_of_threads{4};
	constexpr int number_of_operations{100};

	int run_test{1000};
	for (int run{}; run < run_test; run++) {
		ThreadSafeStack<int> stack;
		std::vector<std::thread> threads;

		for (int i = 0; i < number_of_threads; ++i) {
			threads.emplace_back([&stack, number_of_operations]()
								 {
									 for (int j = 0; j < number_of_operations; ++j)
										 stack.push(j);
								 });
		}
		for (auto &thread: threads) {
			thread.join();
		}
		threads.clear();
		ASSERT_EQ(stack.size(), number_of_operations * number_of_threads);
		int random_number_of_pops = random_distribution(random_engine);
		for (int i = 0; i < number_of_threads; ++i) {
			threads.emplace_back([&stack, random_number_of_pops]()
								 {
									 int value;
									 for (int j = 0; j < random_number_of_pops; ++j)
										 stack.pop(value);
								 });
		}
		for (auto &thread: threads) {
			thread.join();
		}
		ASSERT_EQ(stack.size(), number_of_operations * number_of_threads - number_of_threads * random_number_of_pops);
		if (!stack.size())
			ASSERT_TRUE(stack.empty());
		else
			ASSERT_TRUE(!stack.empty());
	}
}


