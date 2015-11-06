// Socket.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Implement POSIX Socket in C++ style, supported RAII and the interface may be
// similar to C++1x builtin thread library, but currently support POSIX platforms
// only.

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "Status.h"
#include "Types.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

namespace Pumper {
    // forward declarations
    class TcpServer;

    enum ShutdownMode
    {
        Read = SHUT_RD,
        Write = SHUT_WR,
        ReadWrite = SHUT_RDWR
    };

    class TcpClient : public noncopyable
    {
        friend class TcpServer;
    public:
        TcpClient();        
        ~TcpClient();

        Status Connect(const String &ip_address, int32_t port);
        Status Close();
        Status Shutdown(ShutdownMode howto);

        int32_t ReceiveBytes(int8_t *buffer, int32_t length);
        int32_t SendBytes(const int8_t *buffer, int32_t length);

        Status SetNonBlocking(bool is_nonblocking = true);
        Status SetReuseAddress(bool is_reusable = true);

        int32_t GetSocketDescriptor();
        String GetAddressPort();
    private:
        int32_t fd;
        struct sockaddr_in sockaddr;
    };

    class TcpServer : public noncopyable
    {
    public:
        TcpServer();
        ~TcpServer();

        Status Listen(int32_t port, int32_t backlog = 5);
        Status Accept(TcpClient &tcp_client);
        Status Close();
        Status Shutdown(ShutdownMode howto);

        Status SetNonBlocking(bool is_nonblocking = true);
        Status SetReuseAddress(bool is_reusable = true);

        int32_t GetSocketDescriptor();
        String GetAddressPort();
    private:
        int32_t fd;
        struct sockaddr_in sockaddr;
    };

} // namespace Pumper

#endif // __SOCKET_H__
