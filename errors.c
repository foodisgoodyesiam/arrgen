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

// I don't care about optimizing these functions for speed, so repeated calls to vsnprintf are fine.

void myError(const char* restrict message, ...) {
    va_list args, args_copy;
    va_start(args, message);
    va_copy(args_copy, args);
    int len = vsnprintf(NULL, 0, message, args);
    va_end(args);
    if (LIKELY(len>=0)) {
        char buf[len+1];
        vsnprintf(buf, len+1, message, args_copy);
        fprintf(stderr, "%s: %s\n", program_name_, buf);
    }
    va_end(args_copy);
}

void myErrorErrno(const char* restrict message, ...) {
    int error = errno;
    va_list args, args_copy;
    va_start(args, message);
    va_copy(args_copy, args);
    int len = vsnprintf(NULL, 0, message, args);
    va_end(args);
    if (LIKELY(len>0)) {
//        char buf[len+1];
        char *buf = malloc(len+1);
        vsnprintf(buf, len+1, message, args_copy);
        fprintf(stderr, "%s: %s: %s\n", program_name_, buf, strerror(error));
    }
    va_end(args_copy);
}

void myFatal(const char* restrict message, ...) {
    va_list args, args_copy;
    va_start(args, message);
    va_copy(args_copy, args);
    int len = vsnprintf(NULL, 0, message, args);
    va_end(args);
    if (LIKELY(len>0)) {
        char buf[len+1];
        vsnprintf(buf, len+1, message, args_copy);
        fprintf(stderr, "%s: %s\n", program_name_, buf);
    }
    va_end(args_copy);
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
        vsnprintf(buf, len+1, message, args_copy);
        fprintf(stderr, "%s: %s: %s\n", program_name_, buf, strerror(error));
    }
    va_end(args_copy);
    exit(EXIT_FAILURE);
}

