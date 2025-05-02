//
// Created by andreas on 23.04.23.
//

#ifndef LOCK_FREE_STACK_H
#define LOCK_FREE_STACK_H
#include <atomic>
#include <memory>
#include <thread>

unsigned int constexpr max_hazard_pointers = 100;

struct HazardPointer
{
    std::atomic<std::thread::id> id;
    std::atomic<void*> pointer;
};

inline HazardPointer hazard_pointers[max_hazard_pointers];

class HazardPointerOwner
{
    HazardPointer* hazard_pointer;

public:
    HazardPointerOwner(HazardPointerOwner const&) = delete;
    HazardPointerOwner operator=(HazardPointerOwner const&) = delete;

    HazardPointerOwner():
        hazard_pointer(nullptr)
    {
        for (auto& hp : hazard_pointers)
        {
            std::thread::id old_id;
            if (hp.id.compare_exchange_strong(old_id, std::this_thread::get_id()))
            {
                hazard_pointer = &hp;
                break;
            }
        }
        if (!hazard_pointer)
        {
            throw std::runtime_error("No hazard pointers available");
        }
    }

    std::atomic<void*>& get_pointer()
    {
        return hazard_pointer->pointer;
    }

    ~HazardPointerOwner()
    {
        hazard_pointer->pointer.store(nullptr);
        hazard_pointer->id.store(std::thread::id());
    }
};

inline std::atomic<void*>& get_hazard_pointer_for_current_thread()
{
    thread_local static HazardPointerOwner hazard;
    return hazard.get_pointer();
}

inline bool outstanding_hazard_pointers_for(void* pointer)
{
    for (auto & hazard_pointer : hazard_pointers)
    {
        if (hazard_pointer.pointer.load() == pointer)
        {
            return true;
        }
    }
    return false;
}

template <typename T>
class LockFreeStack
{
private:
    struct Node
    {
        std::shared_ptr<T> value_;
        Node* next;

        explicit Node(T const& value)
        // We allocate the data on the heap in the Node
            : value_(std::make_shared<T>(value)),
              next(nullptr)
        {
        }
    };

    std::atomic<Node*> head;
    std::atomic<unsigned int> number_of_threads_in_pop{};
    std::atomic<Node*> to_be_deleted;

    static void delete_nodes(Node* node)
    {
        while (node)
        {
            Node* next = node->next;
            delete node;
            node = next;
        }
    }

    void chain_nodes_for_deletion(Node* node)
    {
        Node* last = node;
        while (Node* const next = last->next)
        {
            last = next;
        }
        chain_nodes_for_deletion(node, last);
    }

    void chain_nodes_for_deletion(Node* first, Node* last)
    {
        last->next = to_be_deleted;
        while (!to_be_deleted.compare_exchange_weak(last->next, first));
    }

    void chain_single_node_for_deletion(Node* node)
    {
        chain_nodes_for_deletion(node, node);
    }

    void try_reclaim_memory(Node* old_head)
    {
        if (number_of_threads_in_pop == 1)
        {
            Node* nodes_to_be_deleted = to_be_deleted.exchange(nullptr);
            if (number_of_threads_in_pop == 1)
            {
                delete_nodes(nodes_to_be_deleted);
            }
            else if (nodes_to_be_deleted)
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
    void push(const T& new_value)
    {
        Node* const new_node = new Node(new_value);
        new_node->next = head.load();
        // Typically use 'compare_exchange_weak' in a loop because in can fail spuriously
        while (!head.compare_exchange_weak(new_node->next, new_node));
    }

    std::shared_ptr<T> pop()
    {
        std::atomic<void*>& hp = get_hazard_pointer_for_current_thread();
        Node* old_head = head.load();
        do
        {
            Node* temp;
            do
            {
                temp = old_head;
                hp.store(old_head);
                old_head = head.load();
            }
            while (old_head != temp);
        }
        while (old_head &&
            !head.compare_exchange_strong(old_head, old_head->next));
        hp.store(nullptr);
        std::shared_ptr<T> result;
        if (old_head)
        {
            result.swap(old_head->data);
            if (outstanding_hazard_pointers_for(old_head))
            {
                reclaim_later(old_head);
            }
            else
            {
                delete old_head;
            }
            delete_nodes_with_no_hazards();
        }
        return result;
    }

    // std::shared_ptr<T> pop()
    // {
    // 	++number_of_threads_in_pop;
    // 	Node* old_head = head.load();
    // 	while(old_head &&  !head.compare_exchange_weak(old_head, old_head->next));
    // 	std::shared_ptr<T> result;
    // 	if(old_head)
    // 	{
    // 		result.swap(old_head->value_);
    // 	}
    // 	try_reclaim_memory(old_head);
    // 	return result;
    // }
};
#endif //LOCK_FREE_STACK_H
