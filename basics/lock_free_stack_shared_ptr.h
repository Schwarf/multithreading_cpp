//
// Created by andreas on 23.04.23.
//

#ifndef LOCK_FREE_STACK_H
#define LOCK_FREE_STACK_H
#include <atomic>
#include <memory>
#include <thread>
#include <stdexcept>

unsigned int constexpr max_hazard_pointers = 100;

struct HazardPointer
{
    std::atomic<std::thread::id> id{std::thread::id()};
    std::atomic<void*> pointer{nullptr};
};

inline HazardPointer hazard_pointers[max_hazard_pointers] = {};

class HazardPointerOwner
{
    HazardPointer* hazard_pointer;

public:
    HazardPointerOwner(HazardPointerOwner const&) = delete;
    HazardPointerOwner operator=(HazardPointerOwner const&) = delete;

    HazardPointerOwner(): hazard_pointer(nullptr)
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

inline std::atomic<void*>& get_hazard_pointer()
{
    thread_local static HazardPointerOwner hazard;
    return hazard.get_pointer();
}

inline bool outstanding_hazard_pointers_for(void* pointer)
{
    for (auto& hazard_pointer : hazard_pointers)
    {
        if (hazard_pointer.pointer.load() == pointer)
        {
            return true;
        }
    }
    return false;
}

template <typename T>
void do_delete(void* pointer)
{
    delete static_cast<T*>(pointer);
}

struct data_to_reclaim
{
    void* data;
    std::function<void(void*)> deleter;
    data_to_reclaim* next;

    template <typename T>
    explicit data_to_reclaim(T* pointer):
        data(pointer),
        deleter(&do_delete<T>),
        next(nullptr)
    {
    }

    ~data_to_reclaim()
    {
        deleter(data);
    }
};

inline std::atomic<data_to_reclaim*> nodes_to_reclaim;


inline void add_to_reclaim_list(data_to_reclaim* node)
{
    node->next = nodes_to_reclaim.load();
    while (!nodes_to_reclaim.compare_exchange_weak(node->next, node));
}

template <typename T>
void reclaim_later(T* data)
{
    add_to_reclaim_list(new data_to_reclaim(data));
}

inline void delete_nodes_with_no_hazards()
{
    data_to_reclaim* current = nodes_to_reclaim.exchange(nullptr);
    while (current)
    {
        data_to_reclaim* const next = current->next;
        if (!outstanding_hazard_pointers_for(current->data))
        {
            delete current;
        }
        else
        {
            add_to_reclaim_list(current);
        }
        current = next;
    }
}

template <typename T>
class LockFreeStackSharedPtr
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

    std::shared_ptr<T> top()
    {
        auto& hazard_pointer = get_hazard_pointer();
        Node* current = nullptr;
        // Loop until we load the same head twice under our hazard pointer
        do
        {
            current = head.load(std::memory_order_acquire);
            hazard_pointer.store(current);
            // retry if head changed under us
        }
        while (current && head.load(std::memory_order_acquire) != current);

        std::shared_ptr<T> result;
        if (current)
        {
            result = current->data; // atomic bump of the shared_ptr control block
        }
        hazard_pointer.store(nullptr);
        return result; // empty shared_ptr if stack was empty
    }

    // Problem: in a lock-free list, once you unlink a node, another thread might actually delete it (reclaim memory)
    // before you finish reading its contents.
    // Solution: you’ve got a global hazard-pointer array + per-thread slot. Before touching any node you “announce” it
    // via your thread’s hazard pointer; reclamation routines must scan all hazard pointers
    // and only delete nodes not currently announced.
    std::shared_ptr<T> pop()
    {
        auto& hazard_pointer = get_hazard_pointer();
        Node* old_head = head.load();

        // Problem: between the moment we load head into old_head and the moment we store that pointer in our hazard
        // slot, another thread could have popped and reclaimed that node -- leaving us with a dangling pointer.
        // Solution:
        // Load old_head
        // Immediately publish it in hp
        // Reload head to confirm it didn’t change in the meantime
        // If it did change, repeat -- his guarantees that whenever you hold hazard_pointer = old_head,
        // that pointer will remain valid until you clear hazard_pointer.
        do
        {
            Node* temp_node = nullptr;
            do
            {
                temp_node = old_head;
                hazard_pointer.store(old_head); // accessing old_head now
                old_head = head.load();  // see if head changed
            }
            while (old_head != temp_node);
        }
        // Problem: multiple threads might race to pop the same node. You must ensure only one wins the unlink, and the others retry.
        // Solution: a CAS on head from old_head to old_head->next:
        while (old_head &&
            !head.compare_exchange_strong(old_head, old_head->next));
        hazard_pointer.store(nullptr);
        std::shared_ptr<T> result;
        if (old_head)
        {
            // Get the payload
            result.swap(old_head->data);
            // Can we delete `old_head` right now, or do we have to defer?
            if (outstanding_hazard_pointers_for(old_head))
            {
                // Some other thread still has `old_head` pinned in its hazard slot.
                // We cannot delete it yet or we’d risk dangling-pointer accesses.
                reclaim_later(old_head);
            }
            else
            {
                delete old_head;
            }
            // Sweep through any previously-deferred nodes
            delete_nodes_with_no_hazards();
        }
        return result;
    }
};
#endif //LOCK_FREE_STACK_H
