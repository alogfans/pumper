#include "DataFile.h"
#include "PagedFile.h"
#include <string.h>
#include <string>
#include <stdio.h>
using namespace Pumper;

int main()
{
	PagedFile pf;
	DataFile df(pf);
	DataFile::Create("temp.db");
	df.OpenFile("temp.db");
	for (int i = 0; i < 1000; i++)
	{
		char buf[60];
		sprintf(buf, "Item %d", i);
		df.Put(buf, buf);
	}
	std::string val;
	df.Get("Item 555", val);
	printf("Getting = %s\n", val.c_str());
	df.Remove("Item 555");
	printf("%d\n", df.Contains("Item 555"));
	df.Close();

	return 0;
}