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


TEST(LockFreeHashTableTest, InsertSameKeyMultipleTimes) {
    HashTable table;

    EXPECT_TRUE(table.insert(55, 1));
    EXPECT_TRUE(table.insert(55, 2));
    EXPECT_TRUE(table.insert(55, 3));

    EXPECT_EQ(table.lookup(55), 3); // Last inserted value should persist
}

TEST(LockFreeHashTableTest, FullTableThrowsException) {
    HashTable table;

    for (int i = 0; i < 1024; ++i) {
        EXPECT_TRUE(table.insert(i, i * 10));
    }

    EXPECT_FALSE(table.insert(2048, 9999));
}

