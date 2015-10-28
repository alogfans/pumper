#include "Status.h"
#include "Types.h"
#include "PagedFile.h"
#include "gtest/gtest.h"
#include <iostream>
#include <string>

using namespace std;
using namespace Pumper;

TEST(storage_test, basic)
{
    PagedFile pp;
    int32_t page_id;
    char *ptr;
    pp.Create("test.dat");
    pp.OpenFile("test.dat");

    for (int i = 0; i < 100; i++)
    {
        pp.AllocatePage(page_id);    
        EXPECT_EQ(page_id, i);
        pp.FetchPage(i, &ptr);
        strcpy(ptr, "Hello world\n");
        pp.MarkDirty(i);
        pp.UnpinPage(i);
    }

    pp.ReleasePage(88);
    pp.AllocatePage(page_id);    
    EXPECT_EQ(page_id, 88);
    pp.Close();
}

TEST(storage_test, read)
{
    PagedFile pp;
    char *ptr;
    pp.OpenFile("test.dat");
    for (int i = 0; i < 100; i++)
    {
        pp.FetchPage(i, &ptr);

        if (i != 88)
            EXPECT_EQ(strcmp(ptr, "Hello world\n"), 0);

        pp.UnpinPage(i);
    }

    pp.Close();
    // pp.Unlink("test.dat");
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
