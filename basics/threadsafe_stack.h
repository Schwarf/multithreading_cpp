//
// Created by andreas on 08.02.23.
//

#include<exception>
#include<memory>
#include<mutex>
#include<stack>

struct EmptyStack: std::exception
{
	[[nodiscard]] const char *what() const throw() override
	{
		return "Thread-Safe stack is empty";
	}
};

template<typename T>
class ThreadSafeStack
{
public:
	ThreadSafeStack() =default;
	ThreadSafeStack(const ThreadSafeStack &other)
	{
		std::lock_guard<std::mutex> lock(other.mutex_);
		stack_ = other.stack_;
	}
	ThreadSafeStack &operator=(const ThreadSafeStack &other) = delete;
	void push(T value)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		stack_.push(value);
	}
	std::shared_ptr<T> pop()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (stack_.empty()) throw EmptyStack();
		std::shared_ptr<T> const result(std::make_shared<T>(stack_.top()));
		stack_.pop();
		return result;
	}
	void pop(T &return_value)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (stack_.empty()) throw EmptyStack();
		return_value = stack_.top();
		stack_.pop();
	}

	bool empty() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return stack_.empty();
	}

private:
	std::stack<T> stack_;
	mutable std::mutex mutex_; // M & M rule: Keyword mutable allows to change mutex state in a const member function

};