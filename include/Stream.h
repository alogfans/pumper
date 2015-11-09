// Stream.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// A simple I/O stream (with char type) in order to allow the
// non-blocking I/O framework working correctly

#ifndef __STREAM_H__
#define __STREAM_H__

#include "Status.h"
#include "Types.h"

namespace Pumper {
    class Stream {
    public:
        Stream() {}
        ~Stream() {}

        // Return the content of byte flow in raw style
        const int8_t *Peek()
        {
            return buffer.c_str();
        }

        int32_t BytesCanRead()
        {
            return (int32_t) buffer.size();
        }

        // Just clear the first nbytes of buffer stream
        void Retrieve(int32_t nbytes)
        {
            buffer = buffer.substr(nbytes, buffer.size());
        }

        void Append(const String &data)
        {
            buffer.append(data);
        }

        // Gather nbytes character to string type and clear the part of buffer 
        // inside. If nbytes < 0, all bytes will be cleared
        String RetrieveToString(int32_t nbytes = -1)
        {
            if (nbytes < 0)
                nbytes = BytesCanRead();
            String result(Peek(), nbytes);
            Retrieve(nbytes);
            return result;
        }

    private:
        String buffer;
    };
} // namespace Pumper

#endif // __STREAM_H__
