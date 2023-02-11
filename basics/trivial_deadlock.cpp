//
// Created by andreas on 06.02.23.
//
#include <future>
#include <thread>
#include <chrono>
#include <random>
#include <iostream>
#include <error.h>

int print_character_at_random_time(char character)
{
	std::default_random_engine random_engine(character);
	std::uniform_int_distribution<int> random_time_milliseconds(10, 1000);
	for(int i{}; i < 10; ++i)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(random_time_milliseconds(random_engine)));
		std::cout.put(character).flush();
	}
	return character;
}

int main()
{
	auto async_thread_random1 = std::async([]{ print_character_at_random_time('.');});
	auto async_thread_random2 = std::async([]{ print_character_at_random_time('@');});
	if(async_thread_random1.wait_for(std::chrono::seconds(0)) != std::future_status::deferred ||
	async_thread_random2.wait_for(std::chrono::seconds(0)) != std::future_status::deferred)
	{
		while(async_thread_random1.wait_for(std::chrono::seconds(0)) != std::future_status::ready &&
			async_thread_random2.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
		{
			std::this_thread::yield();
		}
	}
	std::cout.put('\n').flush();

	try{
		async_thread_random1.get();
		async_thread_random2.get();
	}
	catch(const std::exception & exception)
	{
		std::cout<< "\n EXCEPTION: " << exception.what() << std::endl;
	}
}
