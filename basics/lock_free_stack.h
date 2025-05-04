//
// Created by andreas on 04.05.25.
//

#ifndef LOCK_FREE_STACK_H
#define LOCK_FREE_STACK_H
#include <atomic>
#include <cstddef>
#include <stdexcept>
#include <thread>

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
    explicit Node(T data): data(data), next(nullptr) {}
};

constexpr std::size_t MaxHazardPointers = 50;

template <typename T, NodeConcept Node = Node<T>>
struct HazardPointer
{
    std::atomic<std::thread::id> id;
    std::atomic<Node*> pointer;
};

template <typename T>
HazardPointer<T> HazardPointers[MaxHazardPointers];

template <typename T, NodeConcept Node = Node<T>>
class HazardPointerOwner
{
    HazardPointer<T>* hazardPointer;

public:
    HazardPointerOwner(HazardPointerOwner const&) = delete;
    HazardPointerOwner operator=(HazardPointerOwner const&) = delete;

    HazardPointerOwner() : hazardPointer(nullptr)
    {
        for (std::size_t i = 0; i < MaxHazardPointers; ++i)
        {
            std::thread::id old_id;
            if (HazardPointers<T>[i].id.compare_exchange_strong(
                old_id, std::this_thread::get_id()))
            {
                hazardPointer = &HazardPointers<T>[i];
                break;
            }
        }
        if (!hazardPointer)
        {
            throw std::out_of_range("No hazard pointers available!");
        }
    }

    std::atomic<Node*>& getPointer()
    {
        return hazardPointer->pointer;
    }

    ~HazardPointerOwner()
    {
        hazardPointer->pointer.store(nullptr);
        hazardPointer->id.store(std::thread::id());
    }
};

template <typename T, NodeConcept Node = Node<T>>
std::atomic<Node*>& getHazardPointer()
{
    thread_local static HazardPointerOwner<T> hazard;
    return hazard.getPointer();
}

template <typename T, NodeConcept Node = Node<T>>
class RetireList
{
    struct RetiredNode
    {
        Node* node;
        RetiredNode* next;

        RetiredNode(Node* p) : node(p), next(nullptr)
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
        for (std::size_t i = 0; i < MaxHazardPointers; ++i)
        {
            if (HazardPointers<T>[i].pointer.load() == node) return true;
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
        Node* const newMyNode = new Node(val);
        newMyNode->next = head.load();
        while (!head.compare_exchange_strong(newMyNode->next, newMyNode));
    }

    T pop()
    {
        std::atomic<Node*>& hazardPointer = getHazardPointer<T>();
        Node* oldHead = head.load();
        do
        {
            Node* tempMyNode;
            do
            {
                tempMyNode = oldHead;
                hazardPointer.store(oldHead);
                oldHead = head.load();
            }
            while (oldHead != tempMyNode);
        }
        while (oldHead && !head.compare_exchange_strong(oldHead, oldHead->next)) ;
        if (!oldHead) throw std::out_of_range("The stack is empty!");
        hazardPointer.store(nullptr);
        auto res = oldHead->data;
        if (retireList.isInUse(oldHead)) retireList.addNode(oldHead);
        else delete oldHead;
        retireList.deleteUnusedNodes();
        return res;
    }

    // --- New: lock‐free, hazard‐pointer‐protected top() ---
    T top() const
    {
        auto& hazardPointer = getHazardPointer<T>();
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
