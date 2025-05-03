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
        std::shared_ptr<T> data;
        Node* next;

        explicit Node(T const& value)
            : data(std::make_shared<T>(value)),
              next(nullptr)
        {
        }
    };

    std::atomic<Node*> head;

public:
    void push(const T& new_value)
    {
        Node* const new_node = new Node(new_value);
        new_node->next = head.load();
        // Typically use 'compare_exchange_weak' in a loop because in can fail spuriously
        while (!head.compare_exchange_weak(new_node->next, new_node));
    }

    std::unique_ptr<T> top() {
        auto& hp = get_hazard_pointer_for_current_thread();
        Node* p = nullptr;
        do {
            p = head.load(std::memory_order_acquire);
            hp.store(p);
        } while (p && head.load(std::memory_order_acquire) != p);

        std::unique_ptr<T> result;
        if (p) {
            // clone the payload
            result = std::make_unique<T>(*p->data);
        }
        hp.store(nullptr);
        return result;  // nullptr if stack was empty
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
};
#endif //LOCK_FREE_STACK_H
