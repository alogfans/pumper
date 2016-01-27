// Socket.cpp
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Implement POSIX Socket in C++ style, supported RAII and the interface may be
// similar to C++1x builtin thread library, but currently support POSIX platforms
// only.

#include "Status.h"
#include "Types.h"
#include "Socket.h"

namespace Pumper {

    Socket::Socket()
    {
        fd = socket(AF_INET, SOCK_STREAM, 0);
        ERROR_ASSERT(fd >= 0);
        memset(&sockaddr, 0, sizeof(sockaddr));
    }

    Socket::~Socket()
    {
        if (fd >= 0)
            Close();
    }

    Status Socket::Connect(const String &ip_address, int32_t port)
    {
        WARNING_ASSERT(port > 1023 && port < 65536);
        memset(&sockaddr, 0, sizeof(sockaddr));
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = htons(port);
        WARNING_ASSERT(inet_aton(ip_address.c_str(), &sockaddr.sin_addr));
        WARNING_ASSERT(!connect(fd, (struct sockaddr *) &sockaddr, sizeof(struct sockaddr_in)));

        RETURN_SUCCESS();
    }

    Status Socket::Close()
    {
        WARNING_ASSERT(fd >= 0);
        close(fd);
        fd = -1;
        memset(&sockaddr, 0, sizeof(struct sockaddr_in));
        RETURN_SUCCESS();
    }

    Status Socket::Shutdown(ShutdownMode howto)
    {
        WARNING_ASSERT(fd >= 0);
        WARNING_ASSERT(!shutdown(fd, (int32_t) howto));
        
        if (howto == ShutdownReadWrite)
        {
            fd = -1;
            memset(&sockaddr, 0, sizeof(struct sockaddr_in));
        }

        RETURN_SUCCESS();
    }

    int32_t Socket::ReceiveBytes(int8_t *buffer, int32_t length)
    {
        if (fd < 0)
            return -1;

        int32_t nleft = length, nbytes = 0;
        int8_t *ptr = buffer;

        while (nleft > 0)
        {
            nbytes = read(fd, ptr, nleft);
            if (nbytes < 0)
            {
                if (errno == EINTR)
                    nbytes = 0;         // can be recovered
                else
                    return -1;          // premature fault
            }

            if (nbytes == 0)
                break;
            nleft -= nbytes;
            ptr += nbytes;
        }

        return length - nleft;
    }

    int32_t Socket::SendBytes(const int8_t *buffer, int32_t length)
    {
        if (fd < 0)
            return -1;

        int32_t nleft = length, nbytes = 0;
        int8_t *ptr = const_cast<int8_t *> (buffer);
        
        while (nleft > 0)
        {
            nbytes = write(fd, ptr, nleft);
            if (nbytes < 0)
            {
                if (errno == EINTR)
                    nbytes = 0;         // can be recovered
                else
                    return -1;          // premature fault
            }

            if (nbytes == 0)
                break;
            nleft -= nbytes;
            ptr += nbytes;
        }

        return length - nleft;
    }

    Status Socket::Listen(int port, int backlog)
    {
        WARNING_ASSERT(port > 1023 && port < 65536);
        memset(&sockaddr, 0, sizeof(sockaddr));
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = htons(port);
        sockaddr.sin_addr.s_addr = INADDR_ANY;

        WARNING_ASSERT(!bind(fd, (struct sockaddr *) &sockaddr, sizeof(struct sockaddr_in)));
        WARNING_ASSERT(!listen(fd, backlog));
        RETURN_SUCCESS();
    }

    Status Socket::Accept(Socket &tcp_client)
    {
        WARNING_ASSERT(fd >= 0);

        int32_t cli_fd;
        socklen_t cli_sockaddrlen = sizeof(struct sockaddr_in);
        struct sockaddr_in cli_sockaddr;
        cli_fd = accept(fd, (struct sockaddr *) &cli_sockaddr, &cli_sockaddrlen);
        WARNING_ASSERT(cli_fd >= 0);

        if (tcp_client.fd >= 0)
            tcp_client.Close();
        WARNING_ASSERT(tcp_client.fd < 0);
        
        tcp_client.fd = cli_fd;
        tcp_client.sockaddr = cli_sockaddr;

        RETURN_SUCCESS();
    }

    Status Socket::SetNonBlocking(bool is_nonblocking)
    {
        WARNING_ASSERT(fd >= 0);
        int32_t flags;
        flags = fcntl(fd, F_GETFL, 0);
        if (is_nonblocking)
            flags |= O_NONBLOCK;
        else
            flags &= ~O_NONBLOCK;
        WARNING_ASSERT(!fcntl(fd, F_SETFL, flags));
        RETURN_SUCCESS();
    }

    Status Socket::SetReuseAddress(bool is_reusable)
    {
        WARNING_ASSERT(fd >= 0);

        int32_t mode = (is_reusable ? 1 : 0);
        WARNING_ASSERT(!setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &mode, sizeof(int32_t)));
        RETURN_SUCCESS();
    }
    
    int32_t Socket::GetSocketDescriptor()
    {
        return fd;
    }

    String Socket::GetAddressPort()
    {
        char buffer[128] = { 0 };
        if (fd < 0)
            return String(buffer);
        sprintf(buffer, "%s:%d", inet_ntoa(sockaddr.sin_addr), ntohs(sockaddr.sin_port));
        return String(buffer);
    }

    int32_t Socket::GetPort()
    {
        return ntohs(sockaddr.sin_port);
    }

} // namespace Pumper
