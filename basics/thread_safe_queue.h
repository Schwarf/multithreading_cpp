//
// Created by andreas on 01.05.23.
//

#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H
#include <mutex>
#include <queue>
#include <condition_variable>

template<typename T>
class ThreadSafeQueue
{

public:
	ThreadSafeQueue() = default;
	ThreadSafeQueue(const ThreadSafeQueue<T> &other);
	ThreadSafeQueue &operator=(const ThreadSafeQueue<T> &other) = delete;
	void push(T value)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		data_.push(value);
		condition_.notify_one();
	}
	void wait_and_pop(T &value)
	{
		std::unique_lock<std::mutex> lock(mutex_);
		condition_.wait(lock, [this]
		{ return !data_.empty(); });
		value = data_.front();
		data_.pop();
	}
	bool try_pop(T &value);
	std::shared_ptr<T> try_pop();
	std::shared_ptr<T> wait_and_pop();
	bool empty() const;

private:
	std::mutex mutex_;
	std::queue<T> data_;
	std::condition_variable condition_;

public:
};
#endif //THREAD_SAFE_QUEUE_H
