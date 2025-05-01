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
    float probability{};
    size_t node_count{};
    std::mt19937 generator;
    std::bernoulli_distribution distribution;
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
        : probability(probability), generator(std::random_device{}()), distribution(p)
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
};

#endif //LOCK_FREE_SKIP_LIST_H
