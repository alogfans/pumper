#include "Status.h"
#include "Types.h"
#include "Daemon.h"
#include <string.h>
#include <stdio.h>

using namespace std;
using namespace Pumper;

int main()
{
	Daemon daemon;
	Engine::CreateDb("master");
	daemon.Start("master", 12306);
	daemon.Join();
	return 0;
}
