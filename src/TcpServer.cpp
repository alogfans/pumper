// TcpServer.cpp
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Provide an efficient Linux socket multiplexer for multi-thread
// networking invocations (especially as protobuf-based RPC system)


#include "TcpServer.h"
#include "TcpConnection.h"

#include <functional>
#include <memory>
#include <sys/epoll.h>

namespace Pumper {

    TcpServer::TcpServer()
    {

    }

    TcpServer::~TcpServer()
    {

    }

    Status TcpServer::Start(int32_t port, ReadCallback read_callback)
    {
        std::shared_ptr<Socket> socket = std::make_shared<Socket>();
        RETHROW_ON_EXCEPTION(socket->SetNonBlocking());
        RETHROW_ON_EXCEPTION(socket->SetReuseAddress());
        RETHROW_ON_EXCEPTION(socket->Listen(port));
        callback_map[socket] = read_callback;
        RETURN_SUCCESS();
    }

    void TcpServer::CreateConnection(std::shared_ptr<Socket> socket, int32_t howmany)
    {
        for (int32_t i = 0; i < howmany; i++)
        {
            std::shared_ptr<Socket> client = std::make_shared<Socket>();
            socket->Accept(*client);
            printf("Notify: %s ARRIVAL\n", client->GetAddressPort().c_str());
            std::shared_ptr<TcpConnection> connection(
                new TcpConnection(client, callback_map[socket], this));
            connection_pool[socket] = connection;
        }
    }

    void TcpServer::RemoveConnection(std::shared_ptr<Socket> socket)
    {
        connection_pool.erase(socket);
    }

} // namespace Pumper
