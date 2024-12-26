/* Copyright © 2024 Steven Marion <steven@dragons.fish>
 *
 * This file is part of arrgen.
 *
 * arrgen is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * arrgen is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with arrgen.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "arrgen.h"
#include "errors.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#ifdef ARRGEN_HAS_WINDOWS_ERROR_FUNCTIONS
#   include <errhandlingapi.h>
#   include <windows.h>
#   include <winbase.h>
#endif

// I don't care about optimizing these functions for speed, so repeated calls to vsnprintf are fine.
// TODO can I consolidate these by having the fatal ones call the error ones?

void myError(const char* restrict message, ...) {
    va_list args, args_copy;
    va_start(args, message);
    va_copy(args_copy, args);
    int len = vsnprintf(NULL, 0, message, args);
    va_end(args);
    if (LIKELY(len>=0)) {
        char buf[len+1];
        vsnprintf(buf, (size_t)len+1, message, args_copy);
        va_end(args_copy);
        fprintf(stderr, "%s: %s\n", program_name_, buf);
    }
}

void myErrorErrno(const char* restrict message, ...) {
    int error = errno;
    va_list args, args_copy;
    va_start(args, message);
    va_copy(args_copy, args);
    int len = vsnprintf(NULL, 0, message, args);
    va_end(args);
    if (LIKELY(len>0)) {
        char buf[len+1];
        vsnprintf(buf, (size_t)len+1, message, args_copy);
        va_end(args_copy);
        fprintf(stderr, "%s: %s: %s\n", program_name_, buf, strerror(error));
    }
}

void myFatal(const char* restrict message, ...) {
    va_list args, args_copy;
    va_start(args, message);
    va_copy(args_copy, args);
    int len = vsnprintf(NULL, 0, message, args);
    va_end(args);
    if (LIKELY(len>0)) {
        char buf[len+1];
        vsnprintf(buf, (size_t)len+1, message, args_copy);
        va_end(args_copy);
        fprintf(stderr, "%s: %s\n", program_name_, buf);
    }
    exit(EXIT_FAILURE);
}

void myFatalErrno(const char* restrict message, ...) {
    int error = errno;
    va_list args, args_copy;
    va_start(args, message);
    va_copy(args_copy, args);
    int len = vsnprintf(NULL, 0, message, args);
    va_end(args);
    if (LIKELY(len>=0)) {
        char buf[len+1];
        vsnprintf(buf, (size_t)len+1, message, args_copy);
        va_end(args_copy);
        fprintf(stderr, "%s: %s: %s\n", program_name_, buf, strerror(error));
    }
    exit(EXIT_FAILURE);
}

#ifdef ARRGEN_HAS_WINDOWS_ERROR_FUNCTIONS
void myErrorWindowsError(const char *restrict message, ...) {
    DWORD error = GetLastError();
    va_list args, args_copy;
    va_start(args, message);
    va_copy(args_copy, args);
    int len = vsnprintf(NULL, 0, message, args);
    va_end(args);
    if (LIKELY(len>0)) {
        char buf[len+1];
        vsnprintf(buf, (size_t)len+1, message, args_copy);
        va_end(args_copy);
        // this is super strange, but using this API with the FORMAT_MESSAGE_ALLOCATE_BUFFER option requires to use a pointer to pointer, then cast it to a regular pointer when calling the function
        // what the heck windows?
        LPTSTR lp_buf = NULL;
        va_list empty_va_list;
        DWORD tchars_written = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, // format string with va_list, use system error definitions
            NULL, // does not matter because of the value of the first argument
            error, // the "messaage ID"
            MAKELANGID(LANG_JAPANESE, LANG_ITALIAN), // figure out the appropriate language yourself
            (LPSTR)&lp_buf, // put the pointer to the allocated buffer here
            0, // minimum size to allocate, in case I wanted to append to the buffer I guess?
            NULL // I'm not giving you a format string, so I have no arguments for the parameters
            // SOMEHOW, it appears the buffer is UTF-8? (more likely, it's ANSI, but the same in this case)
            // TODO: figure out using other languages like Japanese (will need to install first though)
        );
        fprintf(stderr, "%s: %s: %s\n", program_name_, buf, lp_buf); // it's annoying that I have to do this, seems I can't do it with the above if I'm using the above in an strerror-like way
        LocalFree(lp_buf);
    }
}

void myFatalWindowsError(const char *restrict message, ...) {
    DWORD error = GetLastError();
    va_list args, args_copy;
    va_start(args, message);
    va_copy(args_copy, args);
    int len = vsnprintf(NULL, 0, message, args);
    va_end(args);
    if (LIKELY(len>0)) {
        char buf[len+1];
        vsnprintf(buf, (size_t)len+1, message, args_copy);
        va_end(args_copy);
        // this is super strange, but using this API with the FORMAT_MESSAGE_ALLOCATE_BUFFER option requires to use a pointer to pointer, then cast it to a regular pointer when calling the function
        // what the heck windows?
        LPTSTR lp_buf = NULL;
        va_list empty_va_list;
        DWORD tchars_written = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, // format string with va_list, use system error definitions
            NULL, // does not matter because of the value of the first argument
            error, // the "messaage ID"
            MAKELANGID(LANG_JAPANESE, LANG_ITALIAN), // figure out the appropriate language yourself
            (LPSTR)&lp_buf, // put the pointer to the allocated buffer here
            0, // minimum size to allocate, in case I wanted to append to the buffer I guess?
            NULL // I'm not giving you a format string, so I have no arguments for the parameters
            // SOMEHOW, it appears the buffer is UTF-8? (more likely, it's ANSI, but the same in this case)
            // TODO: figure out using other languages like Japanese (will need to install first though)
        );
        fprintf(stderr, "%s: %s: %s\n", program_name_, buf, lp_buf); // it's annoying that I have to do this, seems I can't do it with the above if I'm using the above in an strerror-like way
#ifndef NDEBUG
        LocalFree(lp_buf);
#endif
    }
    exit(EXIT_FAILURE);
}
#endif // _WIN32 or _WIN64