//
// Created by andreas on 23.04.23.
//

#ifndef LOCK_FREE_STACK_H
#define LOCK_FREE_STACK_H
#include <atomic>
#include <memory>
#include "hazard_pointer.h"
#include "reclamation.h"

template <typename T>
class LockFreeStack
{
private:
    struct Node
    {
        // We use a shared pointer instead of T the payload of type T lives in its own control block
        // separated from the Node
        // When pop-ing we just grab a copy and then retire the node. T element remains alive even if Node is deleted
        // immediately
        std::shared_ptr<T> data;
        // Need a simple pointer here for atomics.
        Node* next;

        explicit Node(T const& value)
            : data(std::make_shared<T>(value)),
              next(nullptr)
        {
        }
    };
    // Multiple threads will concurrently try to insert at (or remove from) the front of the list.
    // We need an atomic compare-and-swap on head so that two pushes or pops can race but only one “wins” at a time,
    // without ever taking a lock.
    std::atomic<Node*> head;

public:
    void push(const T& new_value)
    {
        auto const new_node = new Node(new_value);
        new_node->next = head.load();
        // Typically use 'compare_exchange_weak' in a loop because it can fail spuriously
        // But better performance than compare_exchange_strong
        while (!head.compare_exchange_weak(new_node->next, new_node));
    }

    std::shared_ptr<T> top() {
        auto& hazard_pointer = get_hazard_pointer_for_current_thread();
        Node* current = nullptr;
        // Loop until we load the same head twice under our hazard pointer
        do {
            current = head.load(std::memory_order_acquire);
            hazard_pointer.store(current);
            // retry if head changed under us
        } while (current && head.load(std::memory_order_acquire) != current);

        std::shared_ptr<T> result;
        if (current) {
            result = current->data;    // atomic bump of the shared_ptr control block
        }
        hazard_pointer.store(nullptr);
        return result;         // empty shared_ptr if stack was empty
    }


    std::shared_ptr<T> pop()
    {
        auto & hazard_pointer = get_hazard_pointer_for_current_thread();
        Node* old_head = head.load();
        do
        {
            Node* temp_node = nullptr;
            do
            {
                temp_node = old_head;
                hazard_pointer.store(old_head);
                old_head = head.load();
            }
            while (old_head != temp_node);
        }
        while (old_head &&
            !head.compare_exchange_strong(old_head, old_head->next));
        hazard_pointer.store(nullptr);
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
};
#endif //LOCK_FREE_STACK_H
