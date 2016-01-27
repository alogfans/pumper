// TcpServer.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Provide an efficient Linux socket multiplexer for multi-thread
// networking invocations (especially as protobuf-based RPC system)

#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "Status.h"
#include "Lock.h"
#include "Socket.h"
#include "EventHandler.h"

#include <memory>
#include <functional>
#include <map>

namespace Pumper {
    class TcpConnection;

    // Is singleton too. But it could manipulate many server fds at the same time
    class TcpServer : public noncopyable {
    public:
        TcpServer();
        ~TcpServer();

        Status Start(int32_t port, ReadCallback read_callback);
        Status Stop(int32_t port = -1);
        
        // The following methods will be used by Epoll.h
        void CreateConnection(std::shared_ptr<Socket> socket);
        void RemoveConnection(std::shared_ptr<Socket> socket);

    private:
        std::map<std::shared_ptr<Socket>, ReadCallback> callback_map;
        std::map<std::shared_ptr<Socket>, std::shared_ptr<TcpConnection> > connection_pool;
    };
} // namespace Pumper

#endif // __TCP_SERVER_H__
