#include "Status.h"
#include "Types.h"
#include <iostream>
#include <string>

using namespace std;
using namespace Pumper;

Status backtrace(int depth)
{
    if (depth == 0)
        RETURN_ERROR("Hello World");
    backtrace(depth - 1);
    RETURN_SUCCESS();
}

int main()
{
    backtrace(5);
    return 0;
}
