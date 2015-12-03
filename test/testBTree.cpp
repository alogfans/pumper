#include "BTree.h"
#include "PagedFile.h"
using namespace Pumper;

int main()
{
	//PagedFile::Create("Idx.idx");
	PagedFile pf;
	pf.OpenFile("Idx.idx");
	BTree bt(pf);
	/*
	for (int i = 0; i < 10000; i++)
	{
		char buf[60];
		sprintf(buf, "Item %d", i);
		bt.Insert(buf, i);
	}
	*/
	for (int i = 0; i < 10000; i++)
	{
		int ii;
		char buf[60];
		sprintf(buf, "Item %d", i);
		bt.Search(buf, ii);
		printf("%d ", ii);
	}
}
