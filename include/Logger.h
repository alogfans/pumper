// Logger.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// A singleton class, for manipulating logger of whole system.

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "Status.h"
#include "Singleton.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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
        Logger()
        {
            log_fd = open("pumper.log", O_APPEND | O_WRONLY | O_CREAT, 0644);
            // if (log_fd < 0)
            // bugcheck("open() from Logger failed.");
        }

        ~Logger()
        {
            close(log_fd);
        }

        void Append(LogLevel log_level, const String &message, bool with_stacktrace = false)
        {
            if ((int32_t) log_level <= (int32_t) ignored_level)
                return;

            char buffer[LOG_ENTRY_LENGTH] = { 0 };
            char level_format[8] = { 0 };
            time_t time_stamp = time(NULL);

            if (log_level == LogLevel::Warning)
                strcpy(level_format, "Warning");
            else if (log_level == LogLevel::Error)
                strcpy(level_format, "Error");

            sprintf(buffer, "%s", ctime(&time_stamp));
            buffer[strlen(buffer) - 1] = '\0';      // suppress \n
            sprintf(buffer, "%s: [%s] %s\n", buffer, level_format, message.c_str());
            write(log_fd, buffer, strlen(buffer));

            fprintf(stderr, "%s", buffer);

            if (with_stacktrace)
                print_stacktrace();
        }

    private:
        void print_stacktrace(unsigned int max_frames = 63)
        {
            char out[STACK_TRACE_LENGTH];
            sprintf(out, "Stacktrace:\n");

            // storage array for stack trace address data
            void* addrlist[max_frames+1];

            // retrieve current stack addresses
            int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

            if (addrlen == 0) {
                sprintf(out, "%s  <empty, possibly corrupt>\n", out);
                return;
            }

            // resolve addresses into strings containing "filename(function+address)",
            // this array must be free()-ed
            char** symbollist = backtrace_symbols(addrlist, addrlen);

            // allocate string which will be filled with the demangled function name
            size_t funcnamesize = 256;
            char* funcname = (char*) malloc(funcnamesize);

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
                        sprintf(out, "%s  %s : %s+%s\n", out, symbollist[i], funcname, begin_offset);
                    }
                    else {
                        // demangling failed. Output function name as a C function with
                        // no arguments.
                        sprintf(out, "%s  %s : %s()+%s\n", out, symbollist[i], begin_name, begin_offset);
                    }
                }
                else
                {
                    // couldn't parse the line? print the whole line.
                    sprintf(out, "%s  %s\n", out, symbollist[i]);
                }
            }

            free(funcname);
            free(symbollist);

            write(log_fd, out, strlen(out));
            fprintf(stderr, "%s\n", out);
        }

    private:
        int32_t log_fd;
    };


} // namespace Pumper

#endif // __LOGGER_H__
