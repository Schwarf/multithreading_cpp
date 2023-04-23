// Created by andreas on 23.04.23.
//

#include <atomic>
#include <thread>
#include <assert.h>
#include <iostream>

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x()
{
	x.store(true, std::memory_order_relaxed);
}
void write_y()
{
	y.store(true, std::memory_order_relaxed);
}
void read_x_then_y()
{
	while (!x.load(std::memory_order_relaxed));
	if (y.load(std::memory_order_relaxed))
		++z;
}
void read_y_then_x()
{
	while (!y.load(std::memory_order_relaxed));
	if (x.load(std::memory_order_relaxed))
		++z;
}
int main()
{
	int count0{};
	int count1{};
	int count2{};
	for (int i{}; i < 10000; ++i) {
		x = false;
		y = false;
		z = 0;
		std::thread a(write_x);
		std::thread b(write_y);
		std::thread c(read_x_then_y);
		std::thread d(read_y_then_x);
		a.join();
		b.join();
		c.join();
		d.join();//
		//	assert(z.load()!=0);

		if(!z.load())
			count0++;
		if(z.load() == 1)
			count1++;
		if(z.load() == 2)
			count2++;
	}
	std::cout << "0 count: "<< count0 << std::endl;
	std::cout << "1 count: "<< count1 << std::endl;
	std::cout << "2 count: "<< count2 << std::endl;
}
