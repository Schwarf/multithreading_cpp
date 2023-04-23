//
// Created by andreas on 07.02.23.
//
#include <vector>
#include <iostream>
// The order of evaluation of function arguments is unspecified in C++ because the language standard
// does not require a specific order of evaluation.
void foo(int a,int b)
{
	std::cout << a << "," << b<< std::endl;
}
int get_num()
{
	static int i=0;
	return ++i;
}
int main()
{
	foo(get_num(),get_num());
}