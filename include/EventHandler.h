// EventHandler.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//

#ifndef __EVENT_HANDLER_H__
#define __EVENT_HANDLER_H__

#include "Status.h"
#include "Socket.h"

#include <pthread.h>
#include <functional>
#include <memory>

namespace Pumper {
    class TcpConnection;

    // That end user could use
    typedef std::function<std::string(const TcpConnection&, const std::string&)> ReadCallback;

    // Internal
    typedef std::function<void(std::shared_ptr<Socket>)> EventCallback;

    // Stub function
    static void event_callback_stub (std::shared_ptr<Socket> socket)
    {
        printf("Warning! Invoke undefined callback function. Socket: %s\n", socket->GetAddressPort().c_str());
    }


    struct EventHandler
    {
        EventHandler() : 
            onRead(&event_callback_stub), 
            onWrite(&event_callback_stub),
            onException(&event_callback_stub),
            onClose(&event_callback_stub) { }
            
        EventCallback onRead;
        EventCallback onWrite;
        EventCallback onException;
        EventCallback onClose;
    };

} // namespace Pumper

#endif // __EVENT_HANDLER_H__
