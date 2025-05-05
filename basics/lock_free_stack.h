//
// Created by andreas on 04.05.25.
//

#ifndef LOCK_FREE_STACK_H
#define LOCK_FREE_STACK_H
#include <atomic>
#include <cstddef>
#include <stdexcept>
#include <thread>


constexpr std::size_t max_hazard_pointers = 50;

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

    HazardPointerOwner() : hazard_pointer(nullptr)
    {
        for (auto& hp : hazard_pointers)
        {
            std::thread::id old_id;
            if (hp.id.compare_exchange_strong(
                old_id, std::this_thread::get_id()))
            {
                hazard_pointer = &hp;
                break;
            }
        }
        if (!hazard_pointer)
        {
            throw std::out_of_range("No hazard pointers available!");
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


template <typename T>
concept NodeConcept = requires(T a)
{
    { T::data };
    { *a.next } -> std::same_as<T&>;
};

template <typename T>
struct Node
{
    T data;
    Node* next;

    explicit Node(T data): data(data), next(nullptr)
    {
    }
};

template <typename T, NodeConcept Node = Node<T>>
class RetireList
{
    struct RetiredNode
    {
        Node* node;
        RetiredNode* next;

        explicit RetiredNode(Node* p) : node(p), next(nullptr)
        {
        }

        ~RetiredNode()
        {
            delete node;
        }
    };

    std::atomic<RetiredNode*> RetiredNodes;

    void addToRetiredNodes(RetiredNode* retiredNode)
    {
        retiredNode->next = RetiredNodes.load();
        while (!RetiredNodes.compare_exchange_strong(retiredNode->next, retiredNode));
    }

public:
    bool isInUse(Node* node)
    {
        for (auto& hp : hazard_pointers)
        {
            if (hp.pointer.load() == node) return true;
        }
        return false;
    }

    void addNode(Node* node)
    {
        addToRetiredNodes(new RetiredNode(node));
    }

    void deleteUnusedNodes()
    {
        RetiredNode* current = RetiredNodes.exchange(nullptr);
        while (current)
        {
            RetiredNode* const next = current->next;
            if (!isInUse(current->node)) delete current;
            else addToRetiredNodes(current);
            current = next;
        }
    }
};

template <typename T, NodeConcept Node = Node<T>>
class LockFreeStack
{
    std::atomic<Node*> head;
    RetireList<T> retireList;

public:
    LockFreeStack() = default;
    LockFreeStack(const LockFreeStack&) = delete;
    LockFreeStack& operator=(const LockFreeStack&) = delete;

    void push(T val)
    {
        Node* const new_node = new Node(val);
        new_node->next = head.load();
        while (!head.compare_exchange_strong(new_node->next, new_node));
    }

    T pop()
    {
        auto& hazard_pointer = get_hazard_pointer();
        Node* old_head = head.load();
        do
        {
            Node* temp_node;
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
            throw std::out_of_range("The stack is empty!");
        hazard_pointer.store(nullptr);
        auto result = old_head->data;
        if (retireList.isInUse(old_head))
            retireList.addNode(old_head);
        else
            delete old_head;
        retireList.deleteUnusedNodes();
        return result;
    }

    // --- New: lock‐free, hazard‐pointer‐protected top() ---
    T top() const
    {
        auto& hazardPointer = get_hazard_pointer();
        Node* current = head.load(std::memory_order_acquire);

        // Loop until head is stable under protection
        while (true)
        {
            // Protect the candidate
            hazardPointer.store(current, std::memory_order_release);

            // Reload head and check it didn't change
            Node* current_updated = head.load(std::memory_order_acquire);
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
            throw std::out_of_range("The stack is empty!");
        }

        // Read data while still protected
        T result = current->data;

        // Release our hazard pointer
        hazardPointer.store(nullptr, std::memory_order_release);
        return result;
    }
};

#endif //LOCK_FREE_STACK_H
