// Status.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Returned value management. In error backtracing, I reuse the code so that
// we can get the c++ style function information.
// https://panthema.net/2008/0901-stacktrace-demangled/
//
// stacktrace.h (c) 2008, Timo Bingmann from http://idlebox.net/
// published under the WTFPL v2.0
//

#ifndef __STATUS_H__
#define __STATUS_H__

#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <cxxabi.h>

#include "Types.h"
#include "Singleton.h"

// Use the following macros to return the status.
#define RETURN_SUCCESS() do { \
    return STATUS_SUCCESS; \
} while(0);

#define RETURN_WARNING(msg) do { \
    char buffer[256]; \
    sprintf(buffer, "%s\n  at function %s:%s() at line %d", msg, __FILE__, __func__, __LINE__); \
    Status status = Status(Error, buffer); \
    return status; \
} while(0);

#define RETURN_ERROR(msg) do { \
    char buffer[256]; \
    sprintf(buffer, "%s\n  at function %s:%s() at line %d", msg, __FILE__, __func__, __LINE__); \
    Status status = Status(Error, buffer); \
    exit(-1); \
} while(0);

#define WARNING_ASSERT(cond) do { \
    if (!(cond)) RETURN_WARNING("Assertion Warning: " #cond) \
} while(0);

#define ERROR_ASSERT(cond) do { \
    if (!(cond)) RETURN_ERROR("Assertion Error: " #cond) \
} while(0);

#define RETHROW_ON_EXCEPTION(cond) do { \
    Status status = (cond); \
    if (!(status == STATUS_SUCCESS)) \
        return status; \
} while(0);

namespace Pumper {

    enum LogLevel {
        Success = 0,
        Warning = 1,
        Error = 2
    };

    const int32_t LOG_ENTRY_LENGTH = 1024;
    const int32_t STACK_TRACE_LENGTH = 65536;
    const LogLevel ignored_level = LogLevel::Success;

    class Logger
    {
    public:
        Logger();
        ~Logger();
        void Append(LogLevel log_level, const String &message, bool with_stacktrace = false);

    private:
        void print_stacktrace(unsigned int max_frames = 63);        
        int32_t log_fd;
    };

    class Status {
    public:
        // Default constructor. But please DO NOT USE IT! Use STATUS_SUCCESS, 
        // STATUS_WARNING or STATUS_ERROR macros instead.
        Status(LogLevel log_level, const String& backtrace) 
        {
            this->log_level = log_level;
            this->backtrace = backtrace;
            Singleton<Logger>::Instance().Append(log_level, backtrace, true);        
        }

        bool operator==(const Status& that) const
        {
            return this->log_level == that.log_level && this->backtrace == that.backtrace;
        }

        String GetBacktrace() 
        {
            return backtrace;
        }

        LogLevel GetLogLevel() 
        {
            return log_level;
        }

    private:
        LogLevel log_level;
        String backtrace;
    };

    // Wrappers of constructors above. Because Success is frequent, we don't want to create
    // new object again and again.
    const Status STATUS_SUCCESS = Status(Success, "");

} // namespace Pumper

#endif // __STATUS_H__

