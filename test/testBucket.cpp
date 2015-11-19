#include "Bucket.h"

using namespace Pumper;

int main()
{
	Bucket bucket;
	for (int i = 0; i < 1000; i++) 
	{
		char buf[32];
		sprintf(buf, "TEST %d Soooooooo long", i);
		bucket.Put(buf, buf);
	}

	for (int i = 0; i < 1000; i += 2) 
	{
		char buf[32];
		sprintf(buf, "TEST %d Soooooooo long", i);
		bucket.Remove(buf);
	}

	bucket.PrintDebugInfo();
	return 0;
}