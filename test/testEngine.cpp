#include "Engine.h"
#include <stdio.h>
using namespace Pumper;

int main(int argc, char *argv[])
{
	Engine::CreateDb("TEST");
	Engine engine;
	engine.OpenDb("TEST");

	for (int i = 0; i < atoi(argv[1]); i++)
	{
		char buf[60];
		sprintf(buf, "Item %d", i);
		engine.Put(buf, buf);
	}

	engine.CloseDb();
	Engine::UnlinkDb("TEST");
}
