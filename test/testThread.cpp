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
    cout << "Thread_Internal: Hello world = " <<  CurrentThread::Name() << endl;
    sleep(2);
}

TEST(thread_test, simple_condition)
{
    Thread thread(hello_world, "Hello world");
    thread.Start();
    cout << "Outer: Hello world = " <<  CurrentThread::Name() << endl;
    sleep(1);
    thread.Join();
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
