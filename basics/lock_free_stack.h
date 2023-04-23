//
// Created by andreas on 23.04.23.
//

#ifndef LOCK_FREE_STACK_H
#define LOCK_FREE_STACK_H
#include <atomic>
#include <memory>

template <typename T>
class LockFreeStack
{
private:
	struct Node
	{
		std::shared_ptr<T> value_;
		Node * next;
		Node(T const & value)
		: value_(std::make_shared<T>(value)){}
	};
	std::atomic<Node*> head;
public:
	void push(T const & new_value)
	{
		Node * const new_node = new Node(new_value);
		new_node->next = head->load();
		// Typically use 'compare_exchange_weak' in a loop because in can fail spuriously
		while(!head.compare_exchange_weak(new_node->next, new_node));
	}

};
#endif //LOCK_FREE_STACK_H
