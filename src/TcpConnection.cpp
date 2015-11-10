// TcpConnection.cpp
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Provide an efficient Linux socket multiplexer for multi-thread
// networking invocations (especially as protobuf-based RPC system)

#include "TcpConnection.h"
#include "TcpServer.h"
#include "Singleton.h"
#include "Epoll.h"

namespace Pumper {

    TcpConnection::TcpConnection(std::shared_ptr<Socket> client, ReadCallback read_callback, 
        TcpServer *tcp_server) : client(client), read_callback(read_callback), tcp_server(tcp_server)
    {
        EventHandler callback_func;
        callback_func.onRead = std::bind(&TcpConnection::onRead, this, std::placeholders::_1);
        callback_func.onClose = std::bind(&TcpConnection::onClose, this, std::placeholders::_1);
        Singleton<Epoll>::Instance().AddCallback(client, PollReadWrite, callback_func);
    }

    TcpConnection::~TcpConnection()
    {

    }

    void TcpConnection::onRead(std::shared_ptr<Socket> socket)
    {
        int nbytes = 100;
        std::string received_buffer, sent_buffer;
        received_buffer.resize(nbytes);
        socket->ReceiveBytes(&received_buffer.front(), nbytes);
        if (read_callback)
            sent_buffer = read_callback(*this, received_buffer);
        socket->SendBytes(&sent_buffer.front(), sent_buffer.size());
    }

    void TcpConnection::onClose(std::shared_ptr<Socket> socket)
    {
        Singleton<Epoll>::Instance().PurgeCallbacks(socket);
        tcp_server->RemoveConnection(socket);
        // Destory the socket
        socket.reset();
    }

} // namespace Pumper

