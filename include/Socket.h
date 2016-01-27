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
    enum ShutdownMode
    {
        ShutdownRead = SHUT_RD,
        ShutdownWrite = SHUT_WR,
        ShutdownReadWrite = SHUT_RDWR
    };

    class Socket : public noncopyable
    {
    public:
        Socket();        
        ~Socket();

        Status Connect(const String &ip_address, int32_t port);
        Status Close();
        Status Shutdown(ShutdownMode howto);

        int32_t ReceiveBytes(int8_t *buffer, int32_t length);
        int32_t SendBytes(const int8_t *buffer, int32_t length);

        Status Listen(int32_t port, int32_t backlog = 5);
        Status Accept(Socket &tcp_client);

        Status SetNonBlocking(bool is_nonblocking = true);
        Status SetReuseAddress(bool is_reusable = true);

        int32_t GetSocketDescriptor();
        String GetAddressPort();
        int32_t GetPort();

    private:
        int32_t fd;
        struct sockaddr_in sockaddr;
    };

} // namespace Pumper

#endif // __SOCKET_H__
