//
// Created by andreas on 25.03.23.
//

#ifndef MERGE_SORT_H
#define MERGE_SORT_H
#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>



// Function to merge two sorted std::vectors
void merge(std::vector<int>& vec, int low, int mid, int high)
{
	std::vector<int> left(mid - low + 1);
	std::vector<int> right(high - mid);

	// Copy the elements into the left and right std::vectors
	for (int i = 0; i < left.size(); i++) {
		left[i] = vec[low + i];
	}
	for (int i = 0; i < right.size(); i++) {
		right[i] = vec[mid + 1 + i];
	}

	int i = 0, j = 0, k = low;

	// Merge the left and right std::vectors back into the original std::vector
	while (i < left.size() && j < right.size()) {
		if (left[i] <= right[j]) {
			vec[k++] = left[i++];
		}
		else {
			vec[k++] = right[j++];
		}
	}

	// Copy any remaining elements from the left std::vector
	while (i < left.size()) {
		vec[k++] = left[i++];
	}

	// Copy any remaining elements from the right std::vector
	while (j < right.size()) {
		vec[k++] = right[j++];
	}
}


void merge_sort(std::vector<int>& vec, int low, int high, int numThreads)
{
	if (low < high) {
		int mid = low + (high - low) / 2;

		if (numThreads > 1) {
			std::cout << " threads = " << numThreads << std::endl;
			// Sort the left half of the std::vector using a new thread
			std::thread leftThread(merge_sort, ref(vec), low, mid, numThreads / 2);
			//std::cout << leftThread.get_id() << std::endl;
			// Sort the right half of the std::vector using the current thread
			merge_sort(vec, mid + 1, high, numThreads - numThreads / 2);

			// Wait for the left thread to finish executing
			leftThread.join();
		}
		else {
			// Sort the left half of the std::vector using the current thread
			merge_sort(vec, low, mid, 1);

			// Sort the right half of the std::vector using the current thread
			merge_sort(vec, mid + 1, high, 1);
		}

		// Merge the two halves of the std::vector
		merge(vec, low, mid, high);
	}
}



#endif //MERGE_SORT_H
