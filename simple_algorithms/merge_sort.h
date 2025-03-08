//
// Created by andreas on 25.03.23.
//

#ifndef MERGE_SORT_H
#define MERGE_SORT_H
#include <vector>
#include <thread>

// Function to merge two sorted std::vectors
template <typename T>
void merge(std::vector<T>& input, int low, int mid, int high)
{
    std::vector<T> left(input.begin() + low, input.begin() + mid + 1);
    std::vector<T> right(input.begin() + mid + 1, input.begin() + high + 1);

    int i = 0, j = 0, k = low;
    while (i < left.size() && j < right.size())
        input[k++] = (left[i] <= right[j]) ? left[i++] : right[j++];

    while (i < left.size())
        input[k++] = left[i++];
    while (j < right.size())
        input[k++] = right[j++];
}

template <typename T>
void merge_sort(std::vector<T>& vec, int low, int high, int numThreads)
{
    static const int min_elements_for_threading = 100000;

    if (low < high)
    {
        int mid = low + (high - low) / 2;

        const bool should_spawn_threads = numThreads > 1 && (high - low) >= min_elements_for_threading;

        if (should_spawn_threads)
        {
            std::thread leftThread(merge_sort<T>, std::ref(vec), low, mid, numThreads / 2);
            merge_sort(vec, mid + 1, high, numThreads - numThreads / 2);
            leftThread.join();
        }
        else
        {
            merge_sort(vec, low, mid, 1);
            merge_sort(vec, mid + 1, high, 1);
        }

        merge(vec, low, mid, high);
    }
}


#endif //MERGE_SORT_H
