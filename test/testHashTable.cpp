#include "Status.h"
#include "Types.h"
#include "HashTable.h"
#include "gtest/gtest.h"
#include <iostream>
#include <string>

using namespace std;
using namespace Pumper;

TEST(hashtable_test, simple_condition)
{
    HashTable hash_table(4);
    EXPECT_EQ(hash_table.IsExisted(1, 2), false);
    hash_table.Insert(1, 2, 5);
    int32_t slot_id = 0;
    EXPECT_EQ(hash_table.TryFind(1, 2, slot_id), true);
    EXPECT_EQ(slot_id, 5);
    EXPECT_EQ(hash_table.IsExisted(2, 1), false);
    hash_table.Remove(1, 2);
	EXPECT_EQ(hash_table.IsExisted(1, 2), false);
}

TEST(hashtable_test, random_condition)
{
    HashTable hash_table(1);
    for (int i = 0; i < 10000; i++)
    	hash_table.Insert(i, i, i);
    for (int i = 0; i < 10000; i++) {
        if (i % 13 == 0) {
            Status status = hash_table.Remove(i, i);
            EXPECT_EQ(status, STATUS_SUCCESS);           
        }
    }
    for (int i = 0; i < 10000; i++) {
        if (i % 13 == 0)
            EXPECT_EQ(hash_table.IsExisted(i, i), false);
        else
            EXPECT_EQ(hash_table.IsExisted(i, i), true);
    }   
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
