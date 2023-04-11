//
// Created by andreas on 11.04.23.
//

#ifndef TIME_MEASUREMENT_H
#define TIME_MEASUREMENT_H
#include <chrono>

template<typename Function, typename... FunctionArguments>
std::chrono::duration<double, std::milli> measure_execution_time(int number_of_tries,
																 Function &function,
																 FunctionArguments... arguments)
{
	std::chrono::nanoseconds total_duration{0};
	for (int i{}; i < number_of_tries; ++i) {
		auto time_0 = std::chrono::steady_clock::now();
		function(arguments...);
		auto time_1 = std::chrono::steady_clock::now();
		total_duration += time_1 - time_0;
	}
	total_duration /= number_of_tries;
	return {total_duration};
}

#endif //TIME_MEASUREMENT_H
