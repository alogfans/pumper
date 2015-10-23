// Status.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Returned value management.

#ifndef __STATUS__
#define __STATUS__

#include "Types.h"
#include <string>
#include <stdlib.h>
#include <execinfo.h>
#include <iostream>

namespace Pumper {
    enum ErrorLevel {
        Success = 0,
        Warning = 1,
        Error = 2
    };

    class Status {
    public:
        // Default constructor. But please DO NOT USE IT! Use STATUS_SUCCESS, 
        // STATUS_WARNING or STATUS_ERROR macros instead.
        Status(ErrorLevel error_level, const String& backtrace) 
        {
            this->error_level = error_level;
            this->backtrace = backtrace;
        }

        String GetBacktrace() 
        {
            return backtrace;
        }

        ErrorLevel GetErrorLevel() 
        {
            return error_level;
        }

    private:
        ErrorLevel error_level;
        String backtrace;
    };

    // Wrappers of constructors above. Because Success is frequent, we don't want to create
    // new object again and again.
    const Status STATUS_SUCCESS = Status(Success, "");
    
#define RETURN_SUCCESS() do { \
    return STATUS_SUCCESS; \
} while(0);

#define RETURN_WARNING(msg) do { \
    String backtrace = dump_backtrace(msg); \
    return Status(Warning, backtrace); \
} while(0);

#define RETURN_ERROR(msg) do { \
    String backtrace = dump_backtrace(msg); \
    Status status = Status(Error, backtrace); \
    std::cerr << backtrace << std::endl; \
    exit(-1); \
} while(0);

// The dump function. Available in UNIX-like environment like Linux.
// Help programmers to find bug with GDB (GNU Debugger)
String dump_backtrace(const String& message)
{
    const int max_depth = 16;
    void *buffer[max_depth] = { 0 };
    int depth;
    char **strings;
    String dump = "PANIC: " + message + "\nBacktrace(s) information:\n";
    
    depth = backtrace(buffer, max_depth);
    strings = backtrace_symbols(buffer, depth);

    if (strings == NULL)
    {
        dump = dump + "backtrace() error.\n";
    }
    else
    {
        for (int i = 0; i < depth; ++i)
            dump = dump + String(strings[i]) + "\n";
    }

    return dump;
}

} // namespace Pumper

#endif // __STATUS__

