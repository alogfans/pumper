#include "Status.h"
#include "Types.h"
#include "PagedFile.h"
#include "PageHandle.h"
#include "gtest/gtest.h"
#include <iostream>
#include <string>

using namespace std;
using namespace Pumper;

TEST(storage_test, basic)
{
    PagedFile pp;
    PageHandle ph;
    pp.Create("test.dat");
    pp.OpenFile("test.dat");

    for (int i = 0; i < 100; i++)
    {
        int32_t page_id;
        pp.AllocatePage(page_id);
        ph.OpenPage(pp, i);
        ph.Write("Hello world\n", 13);
        ph.ClosePage();
    }
    pp.Close();
}

TEST(storage_test, read)
{
    PagedFile pp;
    PageHandle ph;
    char *ptr = new char[13];
    pp.OpenFile("test.dat");
    for (int i = 0; i < 100; i++)
    {
        ph.OpenPage(pp, i);
        ph.Read(ptr, 13);
        EXPECT_EQ(strcmp(ptr, "Hello world\n"), 0);
        ph.ClosePage();
    }
    pp.Close();
    delete [] ptr;
    pp.Unlink("test.dat");
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
