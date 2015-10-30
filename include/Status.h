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

// Use the following macros to return the status.
#define RETURN_SUCCESS() do { \
    return STATUS_SUCCESS; \
} while(0);

#define RETURN_WARNING(msg) do { \
    Status status = Status(Warning, msg); \
    printf("%s\n  at function %s:%s() at line %d\n", msg, __FILE__, __func__, __LINE__); \
    return status; \
} while(0);

#define RETURN_ERROR(msg) do { \
    Status status = Status(Error, msg); \
    printf("%s\n  function: %s:%s()\n  line: %d\n", msg, __FILE__, __func__, __LINE__); \
    print_stacktrace(); \
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

        bool operator==(const Status& that) const
        {
            return this->error_level == that.error_level && this->backtrace == that.backtrace;
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
    
    static inline void print_stacktrace(FILE *out = stderr, unsigned int max_frames = 63)
    {
        fprintf(out, "Stacktrace:\n");

        // storage array for stack trace address data
        void* addrlist[max_frames+1];

        // retrieve current stack addresses
        int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

        if (addrlen == 0) {
            fprintf(out, "  <empty, possibly corrupt>\n");
            return;
        }

        // resolve addresses into strings containing "filename(function+address)",
        // this array must be free()-ed
        char** symbollist = backtrace_symbols(addrlist, addrlen);

        // allocate string which will be filled with the demangled function name
        size_t funcnamesize = 256;
        char* funcname = (char*)malloc(funcnamesize);

        // iterate over the returned symbol lines. skip the first, it is the
        // address of this function.
        for (int i = 1; i < addrlen; i++)
        {
            char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

            // find parentheses and +address offset surrounding the mangled name:
            // ./module(function+0x15c) [0x8048a6d]
            for (char *p = symbollist[i]; *p; ++p)
            {
                if (*p == '(')
                    begin_name = p;
                else if (*p == '+')
                    begin_offset = p;
                else if (*p == ')' && begin_offset) {
                    end_offset = p;
                    break;
                }
            }

            if (begin_name && begin_offset && end_offset && begin_name < begin_offset)
            {
                *begin_name++ = '\0';
                *begin_offset++ = '\0';
                *end_offset = '\0';

                // mangled name is now in [begin_name, begin_offset) and caller
                // offset in [begin_offset, end_offset). now apply
                // __cxa_demangle():

                int status;
                char* ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
                if (status == 0) {
                    funcname = ret; // use possibly realloc()-ed string
                    fprintf(out, "  %s : %s+%s\n", symbollist[i], funcname, begin_offset);
                }
                else {
                    // demangling failed. Output function name as a C function with
                    // no arguments.
                    fprintf(out, "  %s : %s()+%s\n", symbollist[i], begin_name, begin_offset);
                }
            }
            else
            {
                // couldn't parse the line? print the whole line.
                fprintf(out, "  %s\n", symbollist[i]);
            }
        }

        free(funcname);
        free(symbollist);
    }

} // namespace Pumper

#endif // __STATUS_H__

