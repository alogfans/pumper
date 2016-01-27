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
#include "EventHandler.h"

#include <functional>
#include <memory>
#include <sys/epoll.h>

namespace Pumper {

    typedef enum {
        PollNone = 0x0,
        PollRead = 0x1,
        PollWrite = 0x10,
        PollReadWrite = 0x11,
        PollMaskFlag = ~0x11,
    } PollFlag;

    class Socket;    
    const int32_t MAX_EPOLL_FDS = 1024;

    // Poll manager. Should be singleton with Singleton<> wrapper class.
    class Epoll
    {
    public:
        Epoll();
        ~Epoll();

        // Create or add epoll properties (fds and operations that detected)
        Status AddCallback(std::shared_ptr<Socket> socket, PollFlag flag, EventHandler callback_func);

        // Remove flags for fd (may be partial, e.g. listen to READ event instead of READ and WRITE event)
        Status RemoveCallback(std::shared_ptr<Socket> socket, PollFlag flag);

        // Remove all callbacks of file descriptor fd, and it will block until the iteration ensures that
        // the fd will not be handled anymore
        Status PurgeCallbacks(std::shared_ptr<Socket> socket);

        void Poll();
        // Start iteration. Requires to run in a seperate thread!
        void Loop();
    private:
        MutexLock mutex;
        Condition cond;
        //Thread thread;

        bool pending_changes;           // false default.
        int32_t pollfd;                 // epoll object handler
        struct epoll_event ready[MAX_EPOLL_FDS];
        int32_t fd_status[MAX_EPOLL_FDS];
        EventHandler callback_list[MAX_EPOLL_FDS];
        std::shared_ptr<Socket> socket_list[MAX_EPOLL_FDS];
    };
} // namespace Pumper

#endif // __EPOLL_H__
