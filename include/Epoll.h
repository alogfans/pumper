// Epoll.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Provide an efficient Linux socket multiplexer for multi-thread
// networking invocations (especially as protobuf-based RPC system)
// 
// Epoll class will use Linux epoll(5) to handle input or output request
// and call the corresponding user-defined functions

#ifndef __EPOLL_H__
#define __EPOLL_H__

#include "Status.h"
#include "Lock.h"
#include "Thread.h"

#include <functional>
#include <memory>
#include <sys/epoll.h>

namespace Pumper {

    typedef enum {
        None = 0x0,
        ReadOnly = 0x1,
        WriteOnly = 0x10,
        ReadWrite = 0x11,
        MaskFlag = ~0x11,
    } PollFlag;

    class ICallback
    {   
    public:     
        virtual Status Read(int fd) = 0;
        virtual Status Write(int fd) = 0;
    };

    const int32_t MAX_EPOLL_FDS = 1024;

    // Poll manager. Should be singleton with Singleton<> wrapper class.
    class Epoll
    {
    public:
        Epoll();
        ~Epoll();

        // Create or add epoll properties (fds and operations that detected)
        Status AddCallback(int32_t fd, PollFlag flag, const std::shared_ptr<ICallback> callback_func);

        // Remove flags for fd (may be partial, e.g. listen to READ event instead of READ and WRITE event)
        Status RemoveCallback(int32_t fd, PollFlag flag);

        // Remove all callbacks of file descriptor fd, and it will block until the iteration ensures that
        // the fd will not be handled anymore
        Status PurgeCallbacks(int32_t fd);

        // Determine whether this call back existed in epoll system
        bool ExistCallback(int32_t fd, PollFlag flag, const std::shared_ptr<ICallback> callback_func);

        // Start iteration. Requires to run in a seperate thread!
        Status StartIteration();
    private:
        MutexLock mutex;
        Condition cond;
        //Thread thread;

        bool pending_changes;           // false default.
        int32_t pollfd;                 // epoll object handler
        struct epoll_event ready[MAX_EPOLL_FDS];
        int32_t fd_status[MAX_EPOLL_FDS];
        std::shared_ptr<ICallback> callbacks[MAX_EPOLL_FDS];
    };
} // namespace Pumper

#endif // __EPOLL_H__
