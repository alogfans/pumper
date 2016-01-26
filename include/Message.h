// Message.h
// Part of PUMPER, copyright (C) 2016 Alogfans.
//
// The messages transferred in the distributed system and clients.
// Including operation, result or error reportings.

#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "Types.h"
#include "Status.h"

namespace Pumper {
	enum MessageType {
		Command = 1,
		Response,
		Exception
	};

    class Message
    {
    public:
    	// Create a command message
        Message(MessageType type, const String& command);
        // Create a response or exception message, distingished by type. The payload
        // must be text mode
        Message(MessageType type, const String& payload, const Message& refer);
        // according raw stream create a message, and the following funcs will interpret
        Message(const String& raw_data);
        ~Message();

        // Copy assignments and comparator
        Message(const Message &rhs);
        bool operator==(const Message &rhs) const;
        Message& operator=(const Message &rhs);

        // Elements for receiving packet
        int SeqNumber() const;
        MessageType Type() const;
        String Payload() const;
        
        // Convert to packet for sending
        String ToPacket() const;
    private:
    	int         seq_number;
        MessageType type;
    	String      payload;
    	static int  seq_cnt;
    };
} // namespace Pumper

#endif // __MESSAGE_H__
