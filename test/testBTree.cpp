#include "BTree.h"
using namespace Pumper;

int main()
{
	BTree bt;
	for (int i = 0; i < 1000; i++)
	{
		char buf[60];
		sprintf(buf, "Item %d", i);
		bt.Insert(buf, i);
		// printf("xxx\n");
		// bt.PrintDebugInfo();
	}

	for (int i = 0; i < 1000; i++)
	{
		int ii;
		char buf[60];
		sprintf(buf, "Item %d", i);
		bt.Search(buf, ii);
		printf("%d ", ii);
	}
}
