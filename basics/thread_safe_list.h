//
// Created by andreas on 16.04.23.
//

#ifndef THREAD_SAFE_LIST_H
#define THREAD_SAFE_LIST_H
#include <list>
#include <mutex>
template<typename T>
class ThreadSafeList
{
private:
	std::list<T> data_;
	mutable std::mutex mutex_;
public:
	ThreadSafeList() = default;
	ThreadSafeList(const ThreadSafeList &other)
	{
		std::lock_guard<std::mutex> lock(other.mutex_);
		data_ = other.data_;
	}
	// Delete the copy-assignment since in a copy another mutex is used which can lead to race conditions
	// if the two instances are accessed concurrently
	ThreadSafeList &operator=(const ThreadSafeList &) = delete;

	T front() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return data_.front();
	}

	T back() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return data_.back();
	}

	bool empty() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return data_.empty();
	}

	size_t size() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return data_.size();
	}

	size_t max_size() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return data_.max_size();
	}


};

#endif //THREAD_SAFE_LIST_H
