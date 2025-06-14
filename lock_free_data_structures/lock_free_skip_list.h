//
// Created by andreas on 27.04.25.
//

#ifndef LOCK_FREE_SKIP_LIST_H
#define LOCK_FREE_SKIP_LIST_H

#include <atomic>
#include <random>
#include <array>
#include <thread>
#include <concepts>
#include <hazard_pointer.h>
#include <retire_list.h>

template <typename KeyType, typename ValueType, int MaxLevel>
    requires std::integral<KeyType>
class LockFreeSkipList
{
private:
    struct Node
    {
        KeyType key;
        ValueType value;
        int top_level;
        std::atomic<bool> marked;
        std::atomic<bool> fully_linked;
        std::array<std::atomic<Node*>, MaxLevel + 1> forward;

        Node(const KeyType& k, const ValueType& v, int level)
            : key(k), value(v), top_level(level), marked(false), fully_linked(false)
        {
            for (int i = 0; i <= MaxLevel; ++i)
            {
                forward[i].store(nullptr, std::memory_order_relaxed);
            }
        }
    };
    Node* head;
    Node* tail;
    RetireList<Node> retire_list;
    float probability{};
    size_t node_count{};
    std::mt19937 generator;
    std::bernoulli_distribution distribution;
    Node* protect(std::atomic<Node*>& incoming_pointer) {
        Node* node_pointer;
        auto& hazard_pointer = get_hazard_pointer();
        do {
            node_pointer = incoming_pointer.load(std::memory_order_acquire);
            hazard_pointer.store(node_pointer, std::memory_order_seq_cst);
        } while (node_pointer != incoming_pointer.load(std::memory_order_acquire));
        return node_pointer;
    }
    // Randomly generate a level for a new node
    int randomLevel()
    {
        int level{};
        while (level < MaxLevel && distribution(generator))
        {
            level++;
        }
        return level;
    }

public:
    LockFreeSkipList(float probability = 0.5f)
        : probability(probability), generator(std::random_device{}()), distribution(probability)
    {
        head = new Node(std::numeric_limits<KeyType>::lowest(), ValueType{}, MaxLevel);
        tail = new Node(std::numeric_limits<KeyType>::max(), ValueType{}, MaxLevel);
        for (int i = 0; i <= MaxLevel; ++i)
        {
            head->forward[i].store(tail, std::memory_order_relaxed);
        }
    }

    ~LockFreeSkipList()
    {
        // TODO: This is most likely wrong ... probably needs hazard pointers or something else ...
        Node* node = head;
        while (node)
        {
            Node* next = node->forward[0].load();
            delete node;
            node = next;
        }
    }

    bool find_node(const KeyType& key, std::array<Node*, MaxLevel + 1>& predecessors,
                   std::array<Node*, MaxLevel + 1>& successors)
    {
        bool found = false;
        Node* previous = head;
        for (int level = MaxLevel - 1; level > -1; --level)
        {
            auto current = previous->forward[level].load(std::memory_order_acquire);
            while (true)
            {
                auto next = current->forward[level].load(std::memory_order_acquire);
                if (current->marked.load(std::memory_order_relaxed))
                {
                    if (!previous->forward[level].compare_exchange_strong(current, next))
                        return find_node(key, predecessors, successors);
                    current = next;
                }
                else
                {
                    if (current->key < key)
                    {
                        previous = current;
                        current = next;
                    }
                    else
                        break;
                }
            }
            predecessors[level] = previous;
            successors[level] = current;
        }
        auto candidate = successors[0];
        if (candidate->key == key && candidate->fully_linked.load(std::memory_order_acquire) && !candidate->marked.load(
            std::memory_order_acquire))
            found = true;
        return found;
    }

    bool insert(const KeyType& key, const ValueType& value)
    {
        std::array<Node*, MaxLevel + 1> predecessors;
        std::array<Node*, MaxLevel + 1> successors;
        while (true)
        {
            if (find_node(key, predecessors, successors))
                return false;
            int new_level = randomLevel();
            auto new_node = new Node(key, value, new_level);
            for (int level = 0; level <= new_level; ++level)
            {
                new_node->forward[level].store(successors[level], std::memory_order_relaxed);
            }

            auto previous = predecessors[0];
            auto next = successors[0];
            if (!previous->forward[0].compare_exchange_strong(next, new_node))
            {
                delete new_node;
                continue;
            }
            // link higher levels
            for (int level = 1; level <= new_level; ++level)
            {
                while (true)
                {
                    previous = predecessors[level];
                    next = successors[level];
                    if (predecessors[level]->forward[level].compare_exchange_strong(next, new_node))
                        break;
                    find_node(key, predecessors, successors);
                }
            }
            new_node->fully_linked.store(true, std::memory_order_release);
            return true;
        }
    }

    bool remove(const KeyType& key)
    {
        std::array<Node*, MaxLevel + 1> predecessors;
        std::array<Node*, MaxLevel + 1> successors;
        Node * node_to_remove = nullptr;
        bool is_marked = false;
        int top_level{-1};
        while (true)
        {
            if (!find_node(key, predecessors, successors))
                return false;
            node_to_remove = successors[0];
            if (!is_marked)
            {
                top_level = node_to_remove->top_level;
                bool expected = false;
                if (!node_to_remove->marked.compare_exchange_strong(expected, true))
                    return false;
                is_marked = true;
            }
            // unlink
            for (int level = top_level; level >= 0; --level)
            {
                Node * next{};
                do
                {
                    next = node_to_remove->forward[level].load(std::memory_order_acquire);
                }while (!node_to_remove->forward[level].compare_exchange_strong(next, next));
                predecessors[level]->forward[level].compare_exchange_strong(node_to_remove, next);
            }
            delete node_to_remove;
            return true;
        }
    }

    bool search(const KeyType& key, ValueType& value)
    {
        auto previous = head;
        for (int level = MaxLevel ; level > -1; --level)
        {
            auto current = previous->forward[level].load(std::memory_order_acquire);
            while (current && current->key < key)
            {
                previous = current;
                current = current->forward[level].load(std::memory_order_acquire);
            }
        }
        auto current = previous->forward[0].load(std::memory_order_acquire);
        if(current && current->key == key && current->fully_linked.load(std::memory_order_acquire) && !current->marked.load(std::memory_order_acquire))
        {
            value = current->value;
            return true;
        }
        return false;
    }
};

#endif //LOCK_FREE_SKIP_LIST_H
