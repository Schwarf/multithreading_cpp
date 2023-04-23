//
// Created by andreas on 23.04.23.
//

#include <thread>
#include "gtest/gtest.h"
#include "lock_free_stack.h"


TEST(SetupLockFreeStackTest, test_push_and_pop)
{
	int run_test{100};
	constexpr int number_of_threads{10};
	constexpr int number_of_operations{10000};
	for (int run{}; run < run_test; run++) {
		LockFreeStack<int> stack;
		std::vector<std::thread> threads;
		for (int i = 0; i < number_of_threads; ++i) {
			threads.emplace_back([&stack, number_of_operations]()
								 {
									 for (int j = 0; j < number_of_operations; ++j) {
										 int value;
										 stack.push(j);
										 ASSERT_TRUE(stack.pop());
									 }
								 });
		}
		for (auto &thread: threads) {
			thread.join();
		}
		int value;
		ASSERT_FALSE(stack.pop());
	}
}
