// Types.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Some constants or macros of datatypes.

#ifndef __TYPES_H__
#define __TYPES_H__

#include <string>

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

    
} // namespace Pumper

#endif // __TYPES_H__

