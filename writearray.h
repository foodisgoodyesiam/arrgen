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

#ifndef WRITEARRAY_H_INCLUDED
#define WRITEARRAY_H_INCLUDED
#include "arrgen.h"
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief initialize the lookup table for the writeArrayContents function. must be called before it's run
*/
void initializeLookup(uint8_t base, bool aligned);

/**
 * @brief writes array
 * @param out the file to write to
 * @param buf the bytes to turn into text
 * @param length the number of bytes in buf
 * @param cur_line_pos the current position in the output line, should be -1 the first time this is called for a given array
*/
void writeArrayContents(FILE* out, const uint8_t *buf, size_t length, ssize_t *cur_line_pos)
    ATTR_ACCESS(read_only, 2, 3)
    ATTR_ACCESS(read_write, 4)
    ATTR_HOT
    ATTR_NONNULL;

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // WRITEARRAY_H_INCLUDED
