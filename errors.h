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

#ifndef ERRORS_H_INCLUDED
#define ERRORS_H_INCLUDED
#include <stdnoreturn.h>
#include "arrgen.h"

extern const char* program_name_;

/**
 * @brief prints formatted error message and string describing meaning of errno in format (program_name: message: errno meaning) to standard error
 * @param message printf-formatted message string
*/
void myErrorErrno(const char* restrict message, ...)
    ATTR_COLD
    ATTR_NONNULL_N(1)
    ATTR_NOTHROW
    ATTR_LEAF
    ATTR_FORMAT(printf, 1, 2);

/**
 * @brief prints formatted error message in format (program_name: message) to standard error
 * @param message printf-formatted message string
*/
void myError(const char* restrict message, ...)
    ATTR_COLD
    ATTR_NONNULL_N(1)
    ATTR_NOTHROW
    ATTR_LEAF
    ATTR_FORMAT(printf, 1, 2);

/**
 * @brief prints formatted error message and string describing meaning of errno in format (program_name: message: errno meaning) to standard error, then quits
 * @param message printf-formatted message string
*/
noreturn void myFatalErrno(const char* restrict message, ...)
    ATTR_COLD
    ATTR_NONNULL_N(1)
    ATTR_NOTHROW
    ATTR_LEAF
    ATTR_FORMAT(printf, 1, 2);

/**
 * @brief prints formatted error message preceeded by program name to standard error, then quits
 * @param message printf-formatted message string
*/
noreturn void myFatal(const char* restrict message, ...)
    ATTR_COLD
    ATTR_NONNULL_N(1)
    ATTR_NOTHROW
    ATTR_LEAF
    ATTR_FORMAT(printf, 1, 2);

#endif // ERRORS_H_INCLUDED
