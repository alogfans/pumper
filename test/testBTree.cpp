#include "BTree.h"
using namespace Pumper;

int main()
{
	BTree bt;
	bt.Insert("Hello world", 12306);
	bt.PrintDebugInfo();
}