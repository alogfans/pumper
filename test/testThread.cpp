#include "Status.h"
#include "Types.h"
#include "Thread.h"
#include "Lock.h"
#include "gtest/gtest.h"
#include <iostream>
#include <string>

using namespace std;
using namespace Pumper;

void hello_world()
{
    cout << "Thread_Internal: Hello world!" << endl;
}

TEST(thread_test, simple_condition)
{
    Thread thread(hello_world);
    thread.Start();
    thread.Join();
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
