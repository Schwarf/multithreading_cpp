//
// Created by andreas on 01.05.23.
//

#include <thread>
#include <numeric>
#include <random>
#include "gtest/gtest.h"
#include "thread_safe_queue.h"

TEST(TestThreadSafeQueue, test_push_and_try_pop)
{
	int run_test{100};
	constexpr int number_of_threads{10};
	constexpr int number_of_operations{10000};
	for (int run{}; run < run_test; run++) {
		ThreadSafeQueue<int> queue;
		std::vector<std::thread> threads;
		for (int i = 0; i < number_of_threads; ++i) {
			threads.emplace_back([&queue, number_of_operations]()
								 {
									 for (int j = 0; j < number_of_operations; ++j) {
										 int value;
										 queue.push(j);
										 ASSERT_TRUE(queue.try_pop(value));
									 }
								 });
		}
		for (auto &thread: threads) {
			thread.join();
		}
		int value;
		ASSERT_FALSE(queue.try_pop(value));
	}
}


TEST(ThreadSafeQueue, test_push_and_try_pop_check_values)
{
	int run_test{100};
	constexpr int number_of_threads{10};
	constexpr int number_of_operations{1000};
	for (int run{}; run < run_test; run++) {

		ThreadSafeQueue<int> queue;
		std::vector<int> input(number_of_operations);
		std::iota(input.begin(), input.end(), 0);
		std::vector<int> result_values;
		std::vector<int> expected_values;
		std::vector<std::thread> threads;
		std::mutex result_mutex;
		auto fill_queue_per_thread =
			[&queue, &result_values, &result_mutex, number_of_operations]()
			{
				for (int i{}; i < number_of_operations; ++i) {
					queue.push(i);
					int value;
					bool popped = queue.try_pop(value);
					ASSERT_TRUE(popped);
					std::lock_guard<std::mutex> lock(result_mutex);
					result_values.push_back(value);
				}
			};
		for (int i = 0; i < number_of_threads; ++i) {
			threads.emplace_back(fill_queue_per_thread);
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

TEST(ThreadSafeQueue, test_push_and_wait_and_pop_corrected)
{
	constexpr int number_of_operations{1000};
	ThreadSafeQueue<int> queue;

	std::vector<int> input(number_of_operations);
	std::iota(input.begin(), input.end(), 0);

	std::vector<int> output;
	std::mutex output_mutex;

	auto data_preparation_thread = [&input, &queue]()
	{
		for (int data : input) {
			queue.push(data);
		}
	};

	auto process = [](int data)
	{
		return data * data;
	};

	auto data_processing_thread = [&queue, &process, &output, &output_mutex, number_of_operations]()
	{
		for (int i = 0; i < number_of_operations; ++i) {
			int data;
			queue.wait_and_pop(data);
			auto processed_data = process(data);

			std::lock_guard<std::mutex> lock(output_mutex);
			output.push_back(processed_data);
		}
	};

	std::thread producer(data_preparation_thread);
	std::thread consumer(data_processing_thread);

	producer.join();
	consumer.join();

	// Sort output since thread scheduling is non-deterministic
	std::sort(output.begin(), output.end());

	// Check correctness
	for (int i = 0; i < number_of_operations; ++i) {
		EXPECT_EQ(output[i], i * i);
	}
}
