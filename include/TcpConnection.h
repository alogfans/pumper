// TcpConnection.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Provide an efficient Linux socket multiplexer for multi-thread
// networking invocations (especially as protobuf-based RPC system)

#ifndef __TCP_CONNECTION_H__
#define __TCP_CONNECTION_H__

#include "Status.h"
#include "Lock.h"
#include "Socket.h"
#include "EventHandler.h"

#include <memory>
#include <functional>

namespace Pumper {
    class TcpConnection;
    class TcpServer;
    

    class TcpConnection : public noncopyable {
    public:
        TcpConnection(std::shared_ptr<Socket> client, ReadCallback read_callback, TcpServer *tcp_server);
        ~TcpConnection();

        std::string ToString() const;
        void Write(const std::string &data);
    private:
        void onRead(std::shared_ptr<Socket> socket);
        void onClose(std::shared_ptr<Socket> socket);

        std::shared_ptr<Socket> client;
        ReadCallback read_callback;
        TcpServer *tcp_server;
    };
} // namespace Pumper

#endif // __TCP_CONNECTION_H__
