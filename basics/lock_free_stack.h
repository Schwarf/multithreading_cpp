//
// Created by andreas on 23.04.23.
//

#ifndef LOCK_FREE_STACK_H
#define LOCK_FREE_STACK_H
#include <atomic>
#include <memory>
#include <stack>
#include <thread>

constexpr unsigned int maximum_number_of_threads{24};

struct HazardPointer
{
	std::atomic<std::thread::id> thread_id;
	std::atomic<void*> pointer;
};

inline HazardPointer hazard_pointers[maximum_number_of_threads];

class HazardPointerOwner
{
private:
	HazardPointer * hazard_pointer;
public:
	HazardPointerOwner(const HazardPointer & rhs) = delete;
	HazardPointerOwner operator=(const HazardPointer & rhs) = delete;
	HazardPointerOwner():
	hazard_pointer(nullptr)
	{
	}

};


template <typename T>
class LockFreeStack
{
private:
	struct Node
	{

		std::shared_ptr<T> value_;
		Node * next;
		explicit Node(T const & value)
		// We allocate the data on the heap in the Node
		: value_(std::make_shared<T>(value)),
		next(nullptr)
		{}
	};
	std::atomic<Node *> head;
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

	void chain_nodes_for_deletion(Node * node)
	{
		Node * last = node;
		while(Node* const next = last->next)
		{
			last = next;
		}
		chain_nodes_for_deletion(node, last);
	}

	void chain_nodes_for_deletion(Node * first, Node* last)
	{
		last->next = to_be_deleted;
		while(!to_be_deleted.compare_exchange_weak(last->next, first));
	}

	void chain_single_node_for_deletion(Node* node)
	{
		chain_nodes_for_deletion(node, node);
	}

	void try_reclaim_memory(Node * old_head)
	{
		if(number_of_threads_in_pop == 1)
		{
			Node* nodes_to_be_deleted = to_be_deleted.exchange(nullptr);
			if(number_of_threads_in_pop == 1)
			{
				delete_nodes(nodes_to_be_deleted);
			}
			else if(nodes_to_be_deleted)
			{
				chain_nodes_for_deletion(nodes_to_be_deleted);
			}
			delete old_head;
		}
		else
		{
			chain_single_node_for_deletion(old_head);
			--number_of_threads_in_pop;
		}
	}
	

public:
	void push(const T & new_value)
	{
		Node * const new_node = new Node(new_value);
		new_node->next = head.load();
		// Typically use 'compare_exchange_weak' in a loop because in can fail spuriously
		while(!head.compare_exchange_weak(new_node->next, new_node));
	}

	std::shared_ptr<T> pop()
	{
		++number_of_threads_in_pop;
		Node* old_head = head.load();
		while(old_head &&  !head.compare_exchange_weak(old_head, old_head->next));
		std::shared_ptr<T> result;
		if(old_head)
		{
			result.swap(old_head->value_);
		}
		try_reclaim_memory(old_head);
		return result;
	}

};
#endif //LOCK_FREE_STACK_H
