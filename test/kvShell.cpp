// Buffer.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// This is a simple desktop key/value shell for testing purpose.
// Currently there is nothing in the file.
//

#include "Engine.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <functional>

using namespace std;
using namespace Pumper;

#define CMD_LEN 256

typedef function<void(int, char**)> func_t;
struct exec_map_type
{
	char 		cmd[CMD_LEN];
	func_t 		func;
	String 		descriptor;
};

// ****************************************************************************

void func_create(int argc, char **argv);
void func_unlink(int argc, char **argv);
void func_pwd(int argc, char **argv);
void func_open(int argc, char **argv);
void func_close(int argc, char **argv);
void func_put(int argc, char **argv);
void func_get(int argc, char **argv);
void func_remove(int argc, char **argv);
void func_list(int argc, char **argv);
void func_exit(int argc, char **argv);
void func_help(int argc, char **argv);

// ****************************************************************************


static exec_map_type exec_map[] = {
	{"create",	func_create,	"Create new database."}, 
	{"unlink",	func_unlink,	"Unlink existed database."}, 
	{"pwd",		func_pwd,		"Display currently opened database name."}, 
	{"open",	func_open,		"Open database."}, 
	{"close",	func_close,		"Close database."}, 
	{"put",		func_put,		"Insert or update the key/value pair."}, 
	{"get",		func_get,		"Get the latest value according to key."}, 
	{"remove",	func_remove,	"Remove the key/value pair."}, 
	{"list",	func_list,		"List all key/value sets."}, 
	{"exit",	func_exit,		"Exit the program."}, 
	{"help",	func_help,		"Display help message."}, 	
};

#define N_EXEC_MAP (sizeof(exec_map) / sizeof(exec_map_type))

// ****************************************************************************

Engine engine;

void func_create(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: create <db_name>\n");
		return;
	}

	Engine::CreateDb(argv[1]);
}

void func_unlink(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: unlink <db_name>\n");
		return;
	}

	Engine::UnlinkDb(argv[1]);
}

void func_pwd(int argc, char **argv)
{
	if (engine.IsOpened())
		printf("%s\n", engine.OpenDbName().c_str());
	else
		printf("No database file opened\n");
}

void func_open(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: open <db_name>\n");
		return;
	}

	if (engine.IsOpened()) 
	{
		printf("Currently a database file is opened\n");
		return;
	}

	engine.OpenDb(argv[1]);
}

void func_close(int argc, char **argv)
{
	if (!engine.IsOpened()) 
	{
		printf("Currently no database file is opened\n");
		return;
	}

	engine.CloseDb();
}


void func_put(int argc, char **argv)
{
	if (!engine.IsOpened()) 
	{
		printf("Currently no database file is opened\n");
		return;
	}

	if (argc != 3)
	{
		printf("Usage: put <key> <value>\n");
		return;
	}

	engine.Put(argv[1], argv[2]);
}

void func_get(int argc, char **argv)
{
	if (!engine.IsOpened()) 
	{
		printf("Currently no database file is opened\n");
		return;
	}

	String value;
	if (argc != 2)
	{
		printf("Usage: get <key>\n");
		return;
	}

	if (!engine.Contains(argv[1]))
	{
		printf("Such key doesn't exist\n");
		return;
	}

	engine.Get(argv[1], value);
	printf("%s\n", value.c_str());
}

void func_remove(int argc, char **argv)
{
	if (!engine.IsOpened()) 
	{
		printf("Currently no database file is opened\n");
		return;
	}

	if (argc != 2)
	{
		printf("Usage: remove <key> \n");
		return;
	}

	if (!engine.Contains(argv[1]))
	{
		printf("Such key doesn't exist\n");
		return;
	}

	engine.Remove(argv[1]);
}

void func_list(int argc, char **argv)
{
	if (!engine.IsOpened()) 
	{
		printf("Currently no database file is opened\n");
		return;
	}

	std::vector<String> keys = engine.ListKeys();

	for (uint32_t i = 0; i < keys.size(); i++)
	{
		String value;
		engine.Get(keys[i], value);
		printf("%s <-> %s\n", keys[i].c_str(), value.c_str());
	}
}

void func_exit(int argc, char **argv)
{
	if (engine.IsOpened()) 
		engine.CloseDb();

	exit(0);
}

void func_help(int argc, char **argv)
{
	printf("Supported commands\n");
	for (uint32_t i = 0; i < N_EXEC_MAP; i++)
	{
		printf("%s\t%s\n", exec_map[i].cmd, exec_map[i].descriptor.c_str());
	}
}


// ****************************************************************************


int parse(char *buffer, char **argv)
{
	int argc = 0;

	int i = 0;
	while (i < CMD_LEN && buffer[i] != '\0')
	{
		while(buffer[i] == ' ' || buffer[i] == '\t')
			i++;
		//printf("%d\t", i);
		argv[argc++] = &buffer[i];
		while(buffer[i] != ' ' && buffer[i] != '\t' && buffer[i] != '\n')
			i++;
		//printf("%d\t", i);
		buffer[i] = '\0';
		i++;
	}

	if (argv[argc - 1][0] == '\0')
		return argc - 1;

	return argc;
}

void execute(char *command, int argc, char **argv)
{
	for (uint32_t i = 0; i < N_EXEC_MAP; i++)
	{
		if (strcasecmp(command, exec_map[i].cmd) == 0)
		{
			if (exec_map[i].func) 
			{
				exec_map[i].func(argc, argv);
			}
			else 
			{
				printf("ERROR: Command \"%s\" is binded but no corresponding function.\n", command);
			}
			return;
		}
	}
	printf("ERROR: Command \"%s\" unrecognized.\n", command);
}

int main()
{
	// Print banner
	printf("Key/value Storage System: Desktop Edition\n");
	printf("Copyright (C) 2015 Alogfans. All rights reserved.\n");
	printf("Released Version: 1.0\n");

	// Command process
	char command[CMD_LEN] = { 0 };
	int arg_cnt;
	char * arg_val[CMD_LEN] = { NULL };

	while (1) 
	{
		printf("> ");
		fgets(command, CMD_LEN, stdin);
		arg_cnt = parse(command, arg_val);
		if (arg_cnt == 0)
			continue;
		execute(arg_val[0], arg_cnt, arg_val);
	}

	return 0;
}
