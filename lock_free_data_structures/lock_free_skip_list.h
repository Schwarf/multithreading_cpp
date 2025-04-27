//
// Created by andreas on 27.04.25.
//

#ifndef LOCK_FREE_SKIP_LIST_H
#define LOCK_FREE_SKIP_LIST_H

#include <atomic>
#include <random>
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
        std::atomic<Node*> forward[MaxLevel + 1];

        Node(const KeyType& k, const ValueType& v, int level)
            : key(k), value(v), top_level(level), marked(false), fully_linked(false)
        {
            for (int i = 0; i <= MaxLevel; ++i)
            {
                forward[i].store(nullptr, std::memory_order_relaxed);
            }
        }
    };

    Node *head;
    Node *tail;
    float probability{};
    int top_level{};
    size_t node_count{};
    std::mt19937 generator;
    std::bernoulli_distribution distribution;
public:
    LockFreeSkipList(float probability = 0.5f)
    : probability(probability), generator(std::random_device{}()), distribution(p) {
        head = new Node(std::numeric_limits<KeyType>::lowest(), ValueType{}, MaxLevel);
        tail = new Node(std::numeric_limits<KeyType>::max(), ValueType{}, MaxLevel);
        for (int i = 0; i <= MaxLevel; ++i) {
            head->forward[i].store(tail, std::memory_order_relaxed);
        }
    }

};

#endif //LOCK_FREE_SKIP_LIST_H
