#include "Status.h"
#include "Types.h"
#include "HashTable.h"
#include <iostream>
#include <string>

using namespace std;
using namespace Pumper;

int main()
{
    HashTable hash_table(10);
    for (int i = 0; i < 10; i++)
        hash_table.Insert(i, i, i);
    hash_table.PrintDebugInfo();
    return 0;
}
