// TcpClient.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//

#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#include "Status.h"
#include "Lock.h"
#include "Socket.h"
#include "EventHandler.h"

#include <memory>
#include <functional>
#include <map>

namespace Pumper {
    class TcpConnection;

    class TcpClient : public noncopyable {
    public:
        
    private:
        std::shared_ptr<Socket> socket;
    };
} // namespace Pumper

#endif // __TCP_CLIENT_H__
