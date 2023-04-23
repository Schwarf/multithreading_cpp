//
// Created by andreas on 23.04.23.
//
#include <thread>
#include <atomic>
#include <iostream>

std::atomic<int> x(0), y(0), z(0);

std::atomic<bool> go(false);

unsigned const loop_count = 10;

struct ValueStruct
{
	int x, y, z;
};

ValueStruct values1[loop_count];
ValueStruct values2[loop_count];
ValueStruct values3[loop_count];
ValueStruct values4[loop_count];
ValueStruct values5[loop_count];

void increment(std::atomic<int> *variable_to_increment, ValueStruct *values)
{
	while (!go)
		std::this_thread::yield();
	for (unsigned i = 0; i < loop_count; ++i) {
		values[i].x = x.load(std::memory_order_relaxed);
		values[i].y = y.load(std::memory_order_relaxed);
		values[i].z = z.load(std::memory_order_relaxed);
		variable_to_increment->store(i + 1, std::memory_order_relaxed);
		std::this_thread::yield();
	}
}
void read_values(ValueStruct *values)
{

	while (!go)
		std::this_thread::yield();
	for (unsigned i = 0; i < loop_count; ++i) {
		values[i].x = x.load(std::memory_order_relaxed);
		values[i].y = y.load(std::memory_order_relaxed);
		values[i].z = z.load(std::memory_order_relaxed);
		std::this_thread::yield();
	}
}

void print(ValueStruct *v)
{
	for (unsigned i = 0; i < loop_count; ++i) {
		if (i)
			std::cout << ",";
		std::cout << "(" << v[i].x << "," << v[i].y << "," << v[i].z << ")";
	}
	std::cout << std::endl;
}
int main()
{

	std::thread	t1(increment, &x, values1);
	std::thread t2(increment, &y, values2);
	std::thread t3(increment, &z, values3);
	std::thread t4(read_values, values4);
	std::thread t5(read_values, values5);
	go = true;
	t5.join();
	t4.join();
	t3.join();
	t2.join();
	t1.join();
	print(values1);
	print(values2);
	print(values3);
	print(values4);
	print(values5);
}