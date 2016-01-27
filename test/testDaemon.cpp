#include "Status.h"
#include "Types.h"
#include "Daemon.h"
#include "Singleton.h"
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

using namespace std;
using namespace Pumper;


int main()
{
	Singleton<Daemon>::Instance().Start("master", 12306);	
	while(1)
	{
		sleep(1);
		Singleton<Daemon>::Instance().UpdateChanges();
	}
	return 0;
}
