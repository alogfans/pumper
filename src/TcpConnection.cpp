// TcpConnection.cpp
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Provide an efficient Linux socket multiplexer for multi-thread
// networking invocations (especially as protobuf-based RPC system)

#include "TcpConnection.h"
#include "TcpServer.h"
#include "Socket.h"
#include "Singleton.h"
#include "Epoll.h"

namespace Pumper {

    TcpConnection::TcpConnection(std::shared_ptr<Socket> client, ReadCallback read_callback, 
        TcpServer *tcp_server) : client(client), read_callback(read_callback), tcp_server(tcp_server)
    {
        EventHandler callback_func;
        callback_func.onRead = std::bind(&TcpConnection::onRead, this, std::placeholders::_1);
        callback_func.onClose = std::bind(&TcpConnection::onClose, this, std::placeholders::_1);
        Singleton<Epoll>::Instance().AddCallback(client, PollRead, callback_func);
    }

    TcpConnection::~TcpConnection()
    {

    }

    void TcpConnection::onRead(std::shared_ptr<Socket> socket)
    {
        char internal_buffer[128] = { 0 };
        
        int length = socket->ReceiveBytes(internal_buffer, 128);
        if (length == 0)
            return;

        std::string received_buffer(internal_buffer);
        std::string sent_buffer;
        if (read_callback)
            sent_buffer = read_callback(*this, received_buffer);

        memset(internal_buffer, 0, 128);
        strcpy(internal_buffer, sent_buffer.c_str());
        socket->SendBytes(internal_buffer, 128);
    }

    void TcpConnection::onClose(std::shared_ptr<Socket> socket)
    {
        Singleton<Epoll>::Instance().PurgeCallbacks(socket);
        tcp_server->RemoveConnection(socket);
        // Destory the socket
        socket.reset();
    }

    std::string TcpConnection::ToString() const
    {
        return client->GetAddressPort();
    }

    void TcpConnection::Write(const std::string &data)
    {
        char internal_buffer[128] = { 0 };
        strcpy(internal_buffer, data.c_str());
        client->SendBytes(internal_buffer, 128);
    }    

} // namespace Pumper

