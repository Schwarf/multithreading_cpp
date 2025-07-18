//
// Created by andreas on 04.05.25.
//

#ifndef LOCK_FREE_STACK_H
#define LOCK_FREE_STACK_H
#include <atomic>

#include <thread>
#include <optional>
#include "retire_list.h"
#include "hazard_pointer.h"

template <typename T>
struct StackNode
{
    T data;
    StackNode* next;

    explicit StackNode(T data): data(data), next(nullptr)
    {
    }
};

template <typename T>
class LockFreeStack
{
    std::atomic<StackNode<T>*> head;
    RetireList<StackNode<T>> retireList;

public:
    LockFreeStack() = default;
    LockFreeStack(const LockFreeStack&) = delete;
    LockFreeStack& operator=(const LockFreeStack&) = delete;

    /**
     * @brief Pushes a value onto the top of the stack.
     *
     * This operation is lock-free and thread-safe. In the presence of
     * multiple threads, the order in which elements are pushed is
     * not guaranteed. The most recent successful push will be at the top.
     *
     * @param value The value to be pushed onto the stack.
     */
    void push(T val)
    {
        StackNode<T>* const new_node = new StackNode<T>(val);
        new_node->next = head.load();
        while (!head.compare_exchange_strong(new_node->next, new_node));
    }


    std::optional<T> pop()
    {
        // thread-local hazard pointer to prevent premature deletion
        auto& hazard_pointer = get_hazard_pointer();
        StackNode<T>* old_head = head.load();
        do
        {
            StackNode<T>* temp_node;
            do
            {
                temp_node = old_head;
                hazard_pointer.store(old_head);
                old_head = head.load();
            }
            while (old_head != temp_node);
        }
        while (old_head && !head.compare_exchange_strong(old_head, old_head->next)) ;

        if (!old_head)
        {
            hazard_pointer.store(nullptr, std::memory_order_relaxed);
            return std::nullopt;
        }

        hazard_pointer.store(nullptr);
        T result = old_head->data;
        if (retireList.is_in_use(old_head))
            retireList.add_node(old_head);
        else
            delete old_head;
        retireList.delete_unused_nodes();
        return result;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        // acquire‐load pairs with pushes/releases to guarantee we see an up-to-date head
        return head.load(std::memory_order_acquire) == nullptr;
    }

    // --- New: lock‐free, hazard‐pointer‐protected top() ---
    std::optional<T> top() const
    {
        auto& hazardPointer = get_hazard_pointer();
        StackNode<T>* current = head.load(std::memory_order_acquire);

        // Loop until head is stable under protection
        while (true)
        {
            // Protect the candidate
            hazardPointer.store(current, std::memory_order_release);

            // Reload head and check it didn't change
            StackNode<T>* current_updated = head.load(std::memory_order_acquire);
            if (current_updated == current)
            {
                // Either empty (p==nullptr) or stable non-null head
                break;
            }
            // head moved, retry with new pointer
            current = current_updated;
        }

        if (!current)
        {
            hazardPointer.store(nullptr, std::memory_order_release);
            return std::nullopt;
        }

        // Read data while still protected
        T result = current->data;

        // Release our hazard pointer
        hazardPointer.store(nullptr, std::memory_order_release);
        return result;
    }
};

#endif //LOCK_FREE_STACK_H
