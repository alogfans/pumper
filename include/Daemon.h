// Daemon.h
// Part of PUMPER, copyright (C) 2016 Alogfans.
//
// Handle all income requests from clients and other servers in distributed
// environment. 

#ifndef __DAEMON_H__
#define __DAEMON_H__

#include "Status.h"
#include "TcpServer.h"
#include "Engine.h"
#include "Message.h"
#include "Thread.h"

#include <functional>

namespace Pumper {
    class Daemon
    {
    public:
    	Daemon();
    	~Daemon();

    	Status Start(const String& file, int32_t port);
        Status Join();
        Status Stop();
    private: 
    	Message execute_command(const Message &msg);
        Thread epoll_thread;
        Engine engine;
    	TcpServer tcpServer;

        // Generic callback function
        String read_callback(const TcpConnection& conn, const String& msg);
        Message func_put(int argc, char **argv, const Message &msg);
        Message func_get(int argc, char **argv, const Message &msg);
        Message func_remove(int argc, char **argv, const Message &msg);
        Message func_list(int argc, char **argv, const Message &msg);        
    };

   
} // namespace Pumper

#endif // __DAEMON_H__
