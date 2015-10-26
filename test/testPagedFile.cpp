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
    pp.AllocatePage(page_id);    
    EXPECT_EQ(page_id, 0);
    pp.AllocatePage(page_id);    
    EXPECT_EQ(page_id, 1);
    pp.FetchPage(0, &ptr);
    strcpy(ptr, "Hello world\n");
    pp.UnpinPage(0);
    pp.Close();
}

TEST(storage_test, read)
{
    PagedFile pp;
    char *ptr;
    pp.OpenFile("test.dat");
    pp.FetchPage(0, &ptr);
    EXPECT_EQ(strcmp(ptr, "Hello world\n"), 0);
    pp.UnpinPage(0);
    pp.Close();
    pp.Unlink("test.dat");
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
