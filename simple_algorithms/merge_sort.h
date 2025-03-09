//
// Created by andreas on 25.03.23.
//

#ifndef MERGE_SORT_H
#define MERGE_SORT_H
#include <vector>
#include <thread>


#include <vector>
#include <algorithm>

template <typename T>
void merge(std::vector<T>& data, std::vector<T>& aux, int left, int mid, int right) {
    std::copy(data.begin() + left, data.begin() + right + 1, aux.begin() + left);

    int i = left;
    int j = mid + 1;

    for (int k = left; k <= right; ++k) {
        if (i > mid)
            data[k] = aux[j++];
        else if (j > right)
            data[k] = aux[i++];
        else if (aux[j] < aux[i])
            data[k] = aux[j++];
        else
            data[k] = aux[i++];
    }
}

template <typename T>
void merge_sort(std::vector<T>& data) {
    const int n = static_cast<int>(data.size());
    std::vector<T> aux(n);

    for (int width = 1; width < n; width *= 2) {
        for (int left = 0; left < n - width; left += 2 * width) {
            int mid = left + width - 1;
            int right = std::min(left + 2 * width - 1, n - 1);

            merge(data, aux, left, mid, right);
        }
    }
}



// Function to merge two sorted std::vectors
template <typename T>
void merge_multi(std::vector<T>& input, int low, int mid, int high)
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
void merge_sort_multi(std::vector<T>& vec, int low, int high, int numThreads)
{
    static const int min_elements_for_threading = 100000;

    if (low < high)
    {
        int mid = low + (high - low) / 2;

        const bool should_spawn_threads = numThreads > 1 && (high - low) >= min_elements_for_threading;

        if (should_spawn_threads)
        {
            std::thread leftThread(merge_sort_multi<T>, std::ref(vec), low, mid, numThreads / 2);
            merge_sort_multi(vec, mid + 1, high, numThreads - numThreads / 2);
            leftThread.join();
        }
        else
        {
            merge_sort_multi(vec, low, mid, 1);
            merge_sort_multi(vec, mid + 1, high, 1);
        }

        merge_multi(vec, low, mid, high);
    }
}


#endif //MERGE_SORT_H
