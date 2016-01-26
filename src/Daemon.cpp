// Daemon.cpp
// Part of PUMPER, copyright (C) 2016 Alogfans.
//
// Handle all income requests from clients and other servers in distributed
// environment. 

#include "Daemon.h"
#include "Thread.h"
#include "Epoll.h"

namespace Pumper {
	Message Daemon::func_put(int argc, char **argv, const Message &msg)
	{
		if (!engine.IsOpened()) 
			return Message(MessageType::Exception, "File not opened", msg);

		if (argc != 3)
			return Message(MessageType::Exception, "Usage: put <key> <value>", msg);

		if (engine.Put(argv[1], argv[2]) == STATUS_SUCCESS)
			return Message(MessageType::Response,  "OK", msg);
		else
			return Message(MessageType::Exception, "Internal error", msg);
	}

	Message Daemon::func_get(int argc, char **argv, const Message &msg)
	{
		if (!engine.IsOpened()) 
			return Message(MessageType::Exception, "File not opened", msg);

		if (argc != 2)
			return Message(MessageType::Exception, "Usage: get <key> ", msg);

		if (!engine.Contains(argv[1]))
			return Message(MessageType::Response,  "(NULL)", msg);
		
		String value;
		if (engine.Get(argv[1], value) == STATUS_SUCCESS)
			return Message(MessageType::Response, value, msg);
		else
			return Message(MessageType::Exception, "Internal error", msg);
	}

	Message Daemon::func_remove(int argc, char **argv, const Message &msg)
	{
		if (!engine.IsOpened()) 
			return Message(MessageType::Exception, "File not opened", msg);

		if (argc != 2)
			return Message(MessageType::Exception, "Usage: remove <key> ", msg);

		if (!engine.Contains(argv[1]))
			return Message(MessageType::Response,  "(NULL)", msg);

		if (engine.Remove(argv[1]) == STATUS_SUCCESS)
			return Message(MessageType::Response,  "OK", msg);
		else
			return Message(MessageType::Exception, "Internal error", msg);
	}

	Message Daemon::func_list(int argc, char **argv, const Message &msg)
	{
		if (!engine.IsOpened()) 
			return Message(MessageType::Exception, "File not opened", msg);

		std::vector<String> keys = engine.ListKeys();
		char output[100];

		for (uint32_t i = 0; i < keys.size(); i++)
		{
			String value;
			engine.Get(keys[i], value);
			sprintf(output, "%s [%s, %s] ", output, keys[i].c_str(), value.c_str());
		}

		return Message(MessageType::Response, String(output), msg);
	}

	Daemon::Daemon() : epoll_thread([=]() {
		Singleton<Epoll>::Instance().Loop();
	})
	{
	}

    Daemon::~Daemon()
    {
    }

	Status Daemon::Start(const String& file, int32_t port)
	{
		// RETHROW_ON_EXCEPTION(engine.CreateDb(file));
		RETHROW_ON_EXCEPTION(engine.OpenDb(file));

		auto read_callback_bind = std::bind(&Daemon::read_callback, 
			this, std::placeholders::_1, std::placeholders::_2);

		RETHROW_ON_EXCEPTION(tcpServer.Start(port, read_callback_bind));
    	RETHROW_ON_EXCEPTION(epoll_thread.Start());
		RETURN_SUCCESS();
	}

	Status Daemon::Join()
	{
		epoll_thread.Join();
    	RETHROW_ON_EXCEPTION(Stop());
		RETURN_SUCCESS();
	}

	Status Daemon::Stop()
	{
		// RETHROW_ON_EXCEPTION(tcpServer.Stop());
		RETHROW_ON_EXCEPTION(engine.CloseDb());
		RETURN_SUCCESS();
	}

	int parse(char *buffer, char **argv)
	{
		int argc = 0;

		int i = 0;
		while (i < 128 && buffer[i] != '\0')
		{
			while(buffer[i] == ' ' || buffer[i] == '\t')
				i++;
			argv[argc++] = &buffer[i];
			while(buffer[i] != ' ' && buffer[i] != '\t' && buffer[i] != '\n')
				i++;
			buffer[i] = '\0';
			i++;
		}

		if (argv[argc - 1][0] == '\0')
			return argc - 1;

		return argc;
	}

	Message Daemon::execute_command(const Message &msg)
	{
		if (!msg.Payload().size())
			return Message(MessageType::Exception, "Unknown operation", msg);

		char command[128] = { 0 };
		int arg_cnt;
		char * arg_val[128] = { NULL };
		strcpy(command, msg.Payload().c_str());

		arg_cnt = parse(command, arg_val);
		
		if (strcasecmp(command, "put") == 0)	
			return func_put(arg_cnt, arg_val, msg);
		if (strcasecmp(command, "get") == 0)
			return func_get(arg_cnt, arg_val, msg);		
		if (strcasecmp(command, "remove") == 0)	
			return func_remove(arg_cnt, arg_val, msg);
		if (strcasecmp(command, "list") == 0)
			return func_list(arg_cnt, arg_val, msg);

		return Message(MessageType::Exception, "Unknown operation", msg);
	}

	String Daemon::read_callback(const TcpConnection& conn, const String& msg)
	{
		String output;
		Message incoming(msg);
		if (incoming.Type() == MessageType::Command)
			output = execute_command(incoming).ToPacket();
		else
			output = Message(MessageType::Exception, "Illegal Message Type", incoming).ToPacket();
		printf("IN [%s] OUT [%s]\n", msg.c_str(), output.c_str());
		return output;
	}

} // namespace Pumper
