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
}