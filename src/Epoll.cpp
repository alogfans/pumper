// Epoll.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Provide an efficient Linux socket multiplexer for multi-thread
// networking invocations (especially as protobuf-based RPC system)
// 
// Epoll class will use Linux epoll(5) to handle input or output request
// and call the corresponding user-defined functions

#include "Epoll.h"
#include <string.h>

namespace Pumper {
    Epoll::Epoll() : cond(mutex), pending_changes(false)
    {
        pollfd = epoll_create(MAX_EPOLL_FDS);
        ERROR_ASSERT(pollfd >= 0);
        memset(&callbacks, 0, sizeof(callbacks));
        memset(&fd_status, 0, sizeof(fd_status));
    }

    Epoll::~Epoll()
    {
        // Only at exit!
    }

    Status Epoll::AddCallback(int32_t fd, PollFlag flag, const std::shared_ptr<ICallback> callback_func)
    {
        WARNING_ASSERT(fd < MAX_EPOLL_FDS);
        LockGuard lock_guard(mutex);

        struct epoll_event ev;
        int32_t mode;

        if (fd_status[fd])
            mode = EPOLL_CTL_MOD;       // existed fd for triggering
        else
            mode = EPOLL_CTL_ADD;

        fd_status[fd] |= (int32_t) flag;

        ev.events = EPOLLET;
        ev.data.fd = fd;
        if (fd_status[fd] & ReadOnly)
            ev.events |= EPOLLIN;
        if (fd_status[fd] & WriteOnly)
            ev.events |= EPOLLOUT;

        WARNING_ASSERT(!epoll_ctl(pollfd, mode, fd, &ev));

        WARNING_ASSERT(!callbacks[fd] || callbacks[fd] == callback_func);
        callbacks[fd] = callback_func;

        RETURN_SUCCESS();
    }

    Status Epoll::RemoveCallback(int32_t fd, PollFlag flag)
    {
        LockGuard lock_guard(mutex);

        fd_status[fd] &= ~(int32_t) flag;

        struct epoll_event ev;
        int32_t mode;

        if (fd_status[fd])
            mode = EPOLL_CTL_MOD;       // existed fd for triggering
        else
            mode = EPOLL_CTL_DEL;        

        ev.events = EPOLLET;
        ev.data.fd = fd;

        if (fd_status[fd] & ReadOnly)
            ev.events |= EPOLLIN;
        if (fd_status[fd] & WriteOnly)
            ev.events |= EPOLLOUT;

        WARNING_ASSERT(!epoll_ctl(pollfd, mode, fd, &ev));

        if (mode == EPOLL_CTL_DEL) 
            callbacks[fd] = NULL;

        RETURN_SUCCESS();
    }

    // remove all callbacks related to fd
    // the return guarantees that callbacks related to fd will never be called again
    Status Epoll::PurgeCallbacks(int32_t fd)
    {
        LockGuard lock_guard(mutex);

        struct epoll_event ev;

        ev.events = EPOLLET;
        ev.data.fd = fd;

        WARNING_ASSERT(!epoll_ctl(pollfd, EPOLL_CTL_DEL, fd, &ev));

        pending_changes = true;
        cond.Wait();
        callbacks[fd] = NULL;
        RETURN_SUCCESS();
    }

    bool Epoll::ExistCallback(int32_t fd, PollFlag flag, const std::shared_ptr<ICallback> callback_func)
    {
        LockGuard lock_guard(mutex);
        if (!callbacks[fd] || callbacks[fd] != callback_func)
            return false;
        return (fd_status[fd] & MaskFlag) == flag;
    }

    Status Epoll::StartIteration()
    {
        while (true)
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

                if (callbacks[fd] && ready[i].events & EPOLLIN) 
                    callbacks[fd]->Read(fd);
                if (callbacks[fd] && ready[i].events & EPOLLOUT) 
                    callbacks[fd]->Write(fd);
            }
        }

        RETURN_SUCCESS();
    }

} // namespace Pumper

