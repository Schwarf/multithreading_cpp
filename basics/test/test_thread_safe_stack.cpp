//
// Created by andreas on 16.04.23.
//

#include <thread>
#include "gtest/gtest.h"
#include "thread_safe_stack.h"


TEST(SetupThreadSafeStackTest, test_push_and_pop)
{
	int run_test {100};
	for(int run{}; run < run_test; run++) {
		ThreadSafeStack<int> stack;
		constexpr int number_of_threads{10};
		constexpr int number_of_operations{10000};

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