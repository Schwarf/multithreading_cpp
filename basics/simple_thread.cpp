//
// Created by andreas on 04.02.23.
//
#include <iostream>
#include <thread>

void hello()
{
	std::cout << "Hello thread" << std::endl;
}

int main()
{
	std::thread t(hello);
	t.join();
	auto number_of_threads = std::thread::hardware_concurrency();
	std::cout <<"Number of concurrent threads supported by implementation (is only a hint): " << number_of_threads << std::endl;
}