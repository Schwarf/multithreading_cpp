//
// Created by andreas on 08.02.23.
//
#include <iostream>
#include <mutex>
#include <future>
#include <list>
#include <thread>

std::list<int> list;
std::mutex mutex;

void add_to_list(int new_value)
{
	std::lock_guard<std::mutex> lock_guard(mutex);
	list.push_back(new_value);
}

bool does_list_contain(int value_to_find)
{
	std::lock_guard<std::mutex> locak_guard(mutex);
	return std::find(list.begin(), list.end(), value_to_find) != list.end();
}

void print_list()
{
	for (const auto & it: list)
		std::cout << it<< std::endl;

}

int main()
{
	std::packaged_task<bool(int)> task(does_list_contain);
	std::future<bool> f = task.get_future();

	std::thread t1(add_to_list, 1);  // 0-9
	std::thread t2(add_to_list, 2);  // 0-9
	std::thread t3(add_to_list, 3);  // 0-9
	std::thread t4(std::move(task), 1);  // 10-19

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	print_list();
	std::cout << "Result : " << f.get() << std::endl;
	return 0;
}

