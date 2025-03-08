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

TEST(ThreadSafeQueue, test_push_and_wait_and_pop)
{
	constexpr int number_of_operations{1000};
	std::vector<int> input(number_of_operations);
	std::iota(input.begin(), input.end(), 0);
	ThreadSafeQueue<int> queue;
	std::vector<std::thread> threads;
	auto data_preparation_thread = [&input, &queue]()
	{
		while(!input.empty())
		{
			int const data = input.back();
			input.pop_back();
			queue.push(data);
		}
	};
	auto process = [](int data)
	{
		return data*data;
	};
	std::vector<int> output;
	auto data_processing_thread = [&queue, &process, &output]()
	{
		while(true)
		{
			int data;
			queue.wait_and_pop(data);
			auto output_data = process(data);
			output.push_back(output_data);
			if(data >=999)
				break;
		}
	};
	threads.emplace_back(data_preparation_thread);
	threads.emplace_back(data_processing_thread);
	for (auto &thread: threads) {
		thread.join();
	}
	std::reverse(output.begin(), output.end());
	for(int i{}; i < number_of_operations; ++i)
	{
		std::cout << i <<"  "<< input[i] << "  " << output[i] << std::endl;
		EXPECT_EQ(input[i]*input[i], output[i]);
	}
	EXPECT_TRUE(true);

}