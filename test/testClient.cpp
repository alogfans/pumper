// Buffer.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// This is a simple desktop key/value shell for testing purpose.
// Currently there is nothing in the file.
//

#include "Engine.h"
#include "Socket.h"
#include "Message.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <functional>

using namespace std;
using namespace Pumper;

#define CMD_LEN 256

int main()
{
	// Print banner
	printf("Key/value Storage System: Network Edition\n");
	printf("Copyright (C) 2016 Alogfans. All rights reserved.\n");
	printf("Released Version: 1.0\n");

	Socket socket;
	socket.Connect("127.0.0.1", 12306);
	char command[CMD_LEN] = { 0 };
	
	while (1) 
	{
		printf("> ");
		fgets(command, CMD_LEN, stdin);
		command[strlen(command) - 1] = '\0';
		if (strcmp(command, "exit") == 0)
		{
			socket.Close();
			return 0;
		}
		
		char buf[MESSAGE_SIZE];
		strcpy(buf, Message(MessageType::Command, command).ToPacket().c_str());
		socket.SendBytes(buf, MESSAGE_SIZE);
		socket.ReceiveBytes(buf, MESSAGE_SIZE);
		Message out(buf);
		if (out.Type() == MessageType::Response)
		{
			printf("Response: [%s]\n", out.Payload().c_str());
		}
		else
		{
			printf("Exception: [%s]\n", out.Payload().c_str());
		}
	}

	return 0;
}
