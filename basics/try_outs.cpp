//
// Created by andreas on 07.02.23.
//
#include <vector>

int compute_dot_product(const std::vector<int> & v1, const std::vector<int>& v2)
{
	int result{};
	for(int i{}; i < v1.size(); ++i)
		result+= v1[i]*v2[i];
	 return result;
}

