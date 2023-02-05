//
// Created by andreas on 05.02.23.
//

#include <future>
#include <thread>
#include <chrono>
#include <random>
#include <iostream>
#include <exception>

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

int wrapper1()
{
	return print_character_at_random_time('!');
}

int wrapper2()
{
	return print_character_at_random_time('?');
}

int main()
{
	std::cout << "Start wrapper1 asynchronously in background"
	<< " and wrapper2 in foreground:" << std::endl;

	std::future<int> result1(std::async(wrapper1));
	int result2 = wrapper2();
	int result = result1.get() + result2;
	std::cout <<" \n Final result = " << result << std::endl;
}