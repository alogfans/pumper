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
        CB_NONE = 0x0,
        CB_RDONLY = 0x1,
        CB_WRONLY = 0x10,
        CB_RDWR = 0x11,
        CB_MASK = ~0x11,
    } PollFlag;

    struct CallbackFunc
    {
        typedef std::function<void(int32_t)> StubProc;

        StubProc Read;
        StubProc Write;
    };

    const int32_t MAX_EPOLL_FDS = 10240;

    // Poll manager. Should be singleton with Singleton<> wrapper class.
    class Epoll
    {
    public:
        Epoll();
        ~Epoll();

        // Create or add epoll properties (fds and operations that detected)
        Status AddCallback(int32_t fd, PollFlag flag, const std::shared_ptr<CallbackFunc> callback_func);

        // Remove flags for fd (may be partial, e.g. listen to READ event instead of READ and WRITE event)
        Status RemoveCallback(int32_t fd, PollFlag flag);

        // Remove all callbacks of file descriptor fd, and it will block until the iteration ensures that
        // the fd will not be handled anymore
        Status PurgeCallbacks(int32_t fd);

        // Determine whether this call back existed in epoll system
        bool ExistCallback(int32_t fd, PollFlag flag, const std::shared_ptr<CallbackFunc> callback_func);

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
        std::shared_ptr<CallbackFunc> callbacks[MAX_EPOLL_FDS];
    };
} // namespace Pumper

#endif // __EPOLL_H__
