//
// Created by andreas on 08.02.23.
//
#include <exception>
#include <memory>
#include <mutex>
#include <stack>

template<typename T>
class ThreadSafeStack
{
private:
	std::stack<T> data_;
	mutable std::mutex mutex_;
public:
	ThreadSafeStack() = default;
	ThreadSafeStack(const ThreadSafeStack &other)
	{
		std::lock_guard<std::mutex> lock(other.mutex_);
		data_ = other.data_;
	}
	ThreadSafeStack &operator=(const ThreadSafeStack &) = delete;
	void push(const T &value)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		data_.push(value);
	}
	void push(T &&value)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		data_.push(value);
	}
	std::shared_ptr<T> pop()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (data_.empty())
			return nullptr;
		std::shared_ptr<T> const result(std::make_shared<T>(data_.top()));
		data_.pop();
		return result;
	}

	bool pop(T &value)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (data_.empty())
			return false;
		value = data_.top();
		data_.pop();
		return true;
	}

	bool empty() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return data_.empty();
	}

	bool size() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return data_.size();
	}

};
