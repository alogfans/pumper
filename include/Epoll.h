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
	
}

#endif // __EPOLL_H__
