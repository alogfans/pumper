// Types.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Some constants or macros of datatypes.

#ifndef __TYPES_H__
#define __TYPES_H__

#include <string>
#include <functional>

namespace Pumper {
    // Data Types
    typedef unsigned long long uint64_t;
    typedef long long int64_t;
    typedef unsigned int uint32_t;
    typedef int int32_t;
    typedef unsigned short uint16_t;
    typedef short int16_t;
    typedef unsigned char uint8_t;
    typedef char int8_t;
    
    // define String type as std::string, so have ability to replace with my own string 
    // library.
    typedef std::string String;

    // Forbidden copy or assignment
    class noncopyable {
    protected:
        noncopyable() { }
        ~noncopyable() { }
    private:
        noncopyable(const noncopyable&);
        const noncopyable& operator=(const noncopyable&);
    };

    // Constants with type
    const int32_t MEMORY_FD = -1;
    const int32_t INVALID_FD = -2;
    const int32_t INVALID_PAGE_ID = -1;
    const int32_t INVALID_SLOT_ID = -1;
    const int32_t ALL_PAGES = -2;

    const ssize_t SIZEOF_HEADER = 32;
    const ssize_t PAGE_ZERO_OFFSET = SIZEOF_HEADER;
    
    const int32_t BUFFER_SIZE = 20;
    const int32_t PAGE_SIZE = 4096;
    
    const int32_t MESSAGE_SIZE = 1024;

} // namespace Pumper

#endif // __TYPES_H__

