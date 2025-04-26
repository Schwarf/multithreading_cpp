//
// Created by andreas on 26.04.25.
//

#ifndef LOCK_FREE_HASH_TABLE_H
#define LOCK_FREE_HASH_TABLE_H
// Following the implementation from paper: Lock-free parallel dynamic programming.
// This implementation is for the purpose of solving dynamic programming problems using
// a parallelized top-down approach
#include <atomic>
#include <cstdint>
#include <vector>

// The hash-table size is allocated once at compile time and it uses a simple modular hash.
// This requires that the problem size (number of hash values) must be estimated before.
template <typename KeyValueType, KeyValueType NO_KEY, KeyValueType NO_VALUE, size_t TableSize>
class LockFreeHashTable
{
public:
    static constexpr KeyValueType no_key = NO_KEY;
    static constexpr KeyValueType no_value = NO_VALUE;

    struct Entry
    {
        std::atomic<KeyValueType> key;
        std::atomic<KeyValueType> value;
    };

    LockFreeHashTable() : table(TableSize)
    {
        for (size_t i = 0; i < TableSize; i++)
        {
            table[i].key.store(NO_KEY, std::memory_order_relaxed);
            table[i].value.store(NO_VALUE, std::memory_order_relaxed);
        }
    }

    bool insert(KeyValueType key, KeyValueType value)
    {
        size_t hash_value = hash(key);
        size_t slots_examined{};
        while (slots_examined < TableSize)
        {
            auto& entry = table[hash_value];
            KeyValueType expected = NO_KEY;
            if (entry.key.load(std::memory_order_acquire) == NO_KEY)
            {
                if (entry.key.compare_exchange_strong(expected, key, std::memory_order_acq_rel))
                {
                    entry.value.store(value, std::memory_order_release);
                    return true;
                }
            }
            else if (entry.key.load(std::memory_order_acquire) == key)
            {
                entry.value.store(value, std::memory_order_release);
                return true;
            }
            ++slots_examined;
            hash_value = (hash_value + 1) % TableSize;
        }
        return false;
    }

    KeyValueType lookup(KeyValueType key) const
    {
        size_t hash_value = hash(key);
        size_t slots_examined{};
        while (slots_examined < TableSize)
        {
            const auto& entry = table[hash_value];
            auto entry_key = entry.key.load(std::memory_order_acquire);
            if (entry_key == NO_KEY)
            {
                return NO_VALUE;
            }
            if (entry_key == key)
            {
                auto entry_value = entry.value.load(std::memory_order_acquire);
                // Is this needed to be thread safe?
                return (entry_value != NO_VALUE) ? entry_value : NO_VALUE;
            }
            ++slots_examined;
            hash_value = (hash_value + 1) % TableSize;
        }
        return NO_VALUE;
    }

private:
    std::vector<Entry> table;

    static size_t hash(KeyValueType k)
    {
        return static_cast<size_t>(k) % TableSize;
    }
};

#endif //LOCK_FREE_HASH_TABLE_H
