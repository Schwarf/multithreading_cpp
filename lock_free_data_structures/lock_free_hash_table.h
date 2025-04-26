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

// The hash-table size is allocated once at compile time
template <typename KeyValueType, KeyValueType NO_KEY, KeyValueType NO_VALUE, size_t TableSize>
class LockFreeHashTable {
  public:
    struct Entry{
      std::atomic<KeyValueType> key;
      std::atomic<KeyValueType> value;
    };
  LockFreeHashTable() : table(TableSize) {
    for(size_t i = 0; i < TableSize; i++){
      table[i].key.store(NO_KEY, std::memory_order_relaxed);
      table[i].value.store(NO_VALUE, std::memory_order_relaxed);
    }
  }

  private:
    std::vector<Entry> table;
};

#endif //LOCK_FREE_HASH_TABLE_H
