//
// Created by andreas on 04.02.23.
//
#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
#include <algorithm>
#include <future>
#include <random>
#include "./../measure/time_measurement.h"

template <typename T>
T sum(const std::vector<T> & input)
{
	return std::accumulate(input.begin(), input.end(), T{});
}

template <typename T>
void sort(std::vector<T> & input)
{
	std::sort(input.begin(), input.end());
}


int main()
{
	long long min{-31313190};
	long long  max{2152921};
	std::vector<long long> simple_input(std::ranges::views::iota(min, max).begin(), std::ranges::views::iota(min, max).end());
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(simple_input.begin(), simple_input.end(), g);

	auto time0 = std::chrono::steady_clock::now();
	sort(simple_input);
	sum(simple_input);
	//auto sort_result = std::async(std::launch::async, sort<long long>, std::ref(simple_input));
	//auto sum_result = std::async(std::launch::async, sum<long long>, std::ref(simple_input));
	//sort_result.wait();
	//sum_result.wait();

	//std::cout << sum_result.get() << std::endl;
	//sort_result.get();
	auto time1 = std::chrono::steady_clock::now();
	std::chrono::nanoseconds duration = time1-time0;
	std::chrono::duration<double, std::milli> d(duration);
	std::cout << d.count() << std::endl;

}