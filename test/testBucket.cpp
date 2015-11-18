#include "Bucket.h"

using namespace Pumper;

int main()
{
	Bucket bucket;
	//bucket.PrintDebugInfo();
	bucket.TryPut("TEST", "ABCDEFGHIJKLMNOPQRSTUVWXYZOPQRSTUVWXYZ");
	bucket.TryPut("TEST", "Hello world");
	bucket.Remove("TEST");
	bucket.TryPut("ABCDEFGHIJKLMNOPQRS", "ABCDEFGHIJKLMNOPQRSTUVWXYZOPQRSTUVWXYZ");
	bucket.PrintDebugInfo();
	return 0;
}