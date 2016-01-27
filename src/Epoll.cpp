// Epoll.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Provide an efficient Linux socket multiplexer for multi-thread
// networking invocations (especially as protobuf-based RPC system)
// 
// Epoll class will use Linux epoll(5) to handle input or output request
// and call the corresponding user-defined functions

#include "Epoll.h"
#include "Socket.h"
#include <string.h>

namespace Pumper {
    Epoll::Epoll() : cond(mutex), pending_changes(false)
    {
        pollfd = epoll_create(MAX_EPOLL_FDS);
        ERROR_ASSERT(pollfd >= 0);
        memset(&callback_list, 0, sizeof(callback_list));
        memset(&fd_status, 0, sizeof(fd_status));
    }

    Epoll::~Epoll()
    {
        // Only at exit!
    }

    Status Epoll::AddCallback(std::shared_ptr<Socket> socket, PollFlag flag, 
        EventHandler callback_func)
    {
        int32_t fd = socket->GetSocketDescriptor();
        // printf("Trigger fd = %x\n", flag);
        socket_list[fd] = socket;
        WARNING_ASSERT(fd < MAX_EPOLL_FDS);
        LockGuard lock_guard(mutex);

        struct epoll_event ev;
        int32_t mode;

        if (fd_status[fd])
            mode = EPOLL_CTL_MOD;       // existed fd for triggering
        else
            mode = EPOLL_CTL_ADD;

        fd_status[fd] |= (int32_t) flag;

        ev.events = 0;
        ev.data.fd = fd;
        if (fd_status[fd] & PollRead)
            ev.events |= EPOLLIN;
        if (fd_status[fd] & PollWrite)
            ev.events |= EPOLLOUT;

        WARNING_ASSERT(!epoll_ctl(pollfd, mode, fd, &ev));

        callback_list[fd] = callback_func;

        RETURN_SUCCESS();
    }

    Status Epoll::RemoveCallback(std::shared_ptr<Socket> socket, PollFlag flag)
    {
        LockGuard lock_guard(mutex);

        int32_t fd = socket->GetSocketDescriptor();
        fd_status[fd] &= ~(int32_t) flag;

        struct epoll_event ev;
        int32_t mode;

        if (fd_status[fd])
            mode = EPOLL_CTL_MOD;                       // existed fd for triggering
        else
            mode = EPOLL_CTL_DEL;        

        ev.events = 0;
        ev.data.fd = fd;

        if (fd_status[fd] & PollRead)
            ev.events |= EPOLLIN;
        if (fd_status[fd] & PollWrite)
            ev.events |= EPOLLOUT;

        WARNING_ASSERT(!epoll_ctl(pollfd, mode, fd, &ev));

        if (mode == EPOLL_CTL_DEL) 
            callback_list[fd] = EventHandler();         // Default value

        RETURN_SUCCESS();
    }

    // remove all callback_list related to fd
    // the return guarantees that callback_list related to fd will never be called again
    Status Epoll::PurgeCallbacks(std::shared_ptr<Socket> socket)
    {
        LockGuard lock_guard(mutex);

        int32_t fd = socket->GetSocketDescriptor();
        struct epoll_event ev;

        ev.events = EPOLLET;
        ev.data.fd = fd;

        WARNING_ASSERT(!epoll_ctl(pollfd, EPOLL_CTL_DEL, fd, &ev));

        pending_changes = true;
        cond.Wait();
        callback_list[fd] = EventHandler();
        RETURN_SUCCESS();
    }

    void Epoll::Poll()
    {
        {
            LockGuard lock_guard(mutex);
            if (pending_changes)
            {
                pending_changes = false;
                cond.NotifyAll();
            }
        }

        int32_t nfds = epoll_wait(pollfd, ready, MAX_EPOLL_FDS, -1);
        int32_t fd;

        for (int32_t i = 0; i < nfds; i++) {
            fd = ready[i].data.fd;
            // printf("Register event\n");
            if (ready[i].events & EPOLLIN) 
                callback_list[fd].onRead(socket_list[fd]);
            if (ready[i].events & EPOLLOUT) 
                callback_list[fd].onWrite(socket_list[fd]);
            if (ready[i].events & (EPOLLRDHUP | EPOLLERR))
                callback_list[fd].onClose(socket_list[fd]);
        }
    }

    void Epoll::Loop()
    {
        while (true) 
            Poll();
    }

} // namespace Pumper

