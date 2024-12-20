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

#ifndef C_STRING_STUFF_H_INCLUDED
#define C_STRING_STUFF_H_INCLUDED
#include "arrgen.h"

const char* createCName(const char* name ATTR_NONSTRING, size_t name_length, const char* suffix)
    ATTR_ACCESS(read_only, 1, 2)
    ATTR_ACCESS(read_only, 3)
    ATTR_MALLOC
    ATTR_RETURNS_NONNULL
    ATTR_NODISCARD
    ATTR_NONNULL;

// TODO: figure out if there's a convenient way to not need this
char* duplicateString(const char* str)
    ATTR_ACCESS(read_only, 1)
    ATTR_MALLOC
    ATTR_RETURNS_NONNULL
    ATTR_NODISCARD
    ATTR_NONNULL;

char* duplicateStringLen(const char* str ATTR_NONSTRING, size_t length)
    ATTR_ACCESS(read_only, 1, 2)
    ATTR_MALLOC
    ATTR_RETURNS_NONNULL
    ATTR_NONNULL;

uint32_t parseUint32(const char* arg, size_t length)
    ATTR_ACCESS(read_only, 1, 2)
    ATTR_NONNULL;

bool parseBool(const char *potential_bool, const char *param_name)
    ATTR_ACCESS(read_only, 1)
    ATTR_ACCESS(read_only, 2)
    ATTR_NONNULL;

// hmm. clean this up to be more portable (why did I write it this way? glibc should be the special case not the assumed default)
#if !defined(__GLIBC__) && !defined(__CYGWIN__)
const char* customBasename(const char* path)
    ATTR_ACCESS(read_only, 1)
    ATTR_NONNULL;
#   define ARRGEN_USE_CUSTOM_BASENAME
#   define ARRGEN_BASENAME(a) customBasename(a)
#else
#   define ARRGEN_BASENAME(a) basename(a)
#endif

#endif // C_STRING_STUFF_H_INCLUDED
