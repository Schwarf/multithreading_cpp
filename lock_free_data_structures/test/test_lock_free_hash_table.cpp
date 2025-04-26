//
// Created by andreas on 26.04.25.
//
#include "./../lock_free_hash_table.h"
#include "gtest/gtest.h"

using HashTable = LockFreeHashTable<int64_t, -1, -1, 1024>;

TEST(TestLockFreeHashTable, InsertAndLookupSingleThread)
{
    HashTable table;
    EXPECT_EQ(table.lookup(42), HashTable::no_value);
    EXPECT_TRUE(table.insert(42, 100));
    EXPECT_EQ(table.lookup(42), 100);

    EXPECT_TRUE(table.insert(100, 200));
    EXPECT_EQ(table.lookup(100), 200);

    // Update existing key
    EXPECT_TRUE(table.insert(42, 300));
    EXPECT_EQ(table.lookup(42), 300);
}
