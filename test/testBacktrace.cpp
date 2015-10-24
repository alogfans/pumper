#include "Status.h"
#include "Types.h"
#include <iostream>
#include <string>

using namespace std;
using namespace Pumper;

Status backtrace(int depth)
{
    if (depth == 0)
        ERROR_ASSERT(1 < 0);
    backtrace(depth - 1);
    RETURN_SUCCESS();
}

int main()
{
    backtrace(5);
    return 0;
}
