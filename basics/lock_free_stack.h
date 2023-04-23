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
		// We allocate the data on the heap in the Node
		: value_(std::make_shared<T>(value)){}
	};
	std::atomic<Node*> head;
	std::atomic<unsigned int> number_of_threads_in_pop{};
	std::atomic<Node*> to_be_deleted;
	static void delete_nodes(Node * node)
	{
		while(node)
		{
			Node * next = node->next;
			delete node;
			node = next;
		}
	}
	void try_reclaim(Node * old_head)
	{
		if(number_of_threads_in_pop == 1)
		{
			Node* nodes_to_be_deleted = to_be_deleted.exchange(nullptr);
			if(number_of_threads_in_pop == 1)
			{
				delete_nodes(to_be_deleted);
			}
			else if(nodes_to_be_deleted)
			{
				chain_pending_nodes(nodes_to_be_deleted);
			}
			delete old_head;
		}
		else
		{
			chain_pending_nodes(old_head);
			--number_of_threads_in_pop;
		}
	}
	

public:
	void push(T const & new_value)
	{
		Node * const new_node = new Node(new_value);
		new_node->next = head->load();
		// Typically use 'compare_exchange_weak' in a loop because in can fail spuriously
		while(!head.compare_exchange_weak(new_node->next, new_node));
	}

	std::shared_ptr<T> popo()
	{
		++number_of_threads_in_pop;
		Node* current_head = head.load();
		while(current_head &&  !head.compare_exchange_weak(current_head, current_head->next));
		std::shared_ptr<T> result;
		if(current_head)
		{
			result.swap(current_head->value);
		}
		return current_head ? current_head->value_ : nullptr;
	}

};
#endif //LOCK_FREE_STACK_H
