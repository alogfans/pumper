// Message.cpp
// Part of PUMPER, copyright (C) 2016 Alogfans.
//
// The messages transferred in the distributed system and clients.
// Including operation, result or error reportings.

#include "Message.h"

namespace Pumper {
    int Message::seq_cnt = 0;

    Message::Message(MessageType type, const String& command) : 
        seq_number(seq_cnt), type(type), payload(command)
    {
        seq_cnt++;
    }

    Message::Message(MessageType type, const String& payload, const Message& refer) : 
        seq_number(refer.SeqNumber()), type(type), payload(payload)
    {
    }

    Message::Message(const String& raw_data) : seq_number(0)
    {
        uint32_t i = 0;
        while (raw_data[i] >= '0' && raw_data[i] <= '9') 
        {
            seq_number = seq_number * 10 + raw_data[i] - '0';
            i++;
        }

        // Next symbol: type selection
        if (raw_data[i] == 'C')
            type = MessageType::Command;
        else if (raw_data[i] == 'R')
            type = MessageType::Response;
        else
            type = MessageType::Exception;

        if (i + 1 < raw_data.size())
            payload = raw_data.substr(i + 1, raw_data.size());
    }

    Message::~Message()
    {
    }

    Message::Message(const Message &rhs) : 
        seq_number(rhs.seq_number), type(rhs.type), payload(rhs.payload)
    {
    }

    bool Message::operator==(const Message &rhs) const
    {
        if (seq_number != rhs.seq_number)
            return false;
        if (type != rhs.type)
            return false;
        if (payload != rhs.payload)
            return false;
        return true;
    }

    Message& Message::operator=(const Message &rhs)
    {
        if (*this == rhs)
            return *this;

        seq_number = rhs.seq_number;
        type = rhs.type;
        payload = rhs.payload;
        return *this;
    }

    int Message::SeqNumber() const
    {
        return seq_number;
    }

    MessageType Message::Type() const
    {
        return type;
    }

    String Message::Payload() const
    {
        return payload;
    }

    String Message::ToPacket() const
    {
        char seq_char[5];
        sprintf(seq_char, "%d", seq_number);
        String builder(seq_char);

        if (type == MessageType::Command)
            builder += "C";
        else if (type == MessageType::Response)
            builder += "R";
        else
            builder += "E";

        builder += payload;
        return builder.substr(0, MESSAGE_SIZE);
    }

} // namespace Pumper
