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
#include "writearray.h"
#include "errors.h"

void initializeLookup(uint8_t base, bool aligned) {
    // TODO
}

// TODO: make it return error information instead of quitting?
void writeArrayContents(FILE* out, const uint8_t *buf, size_t length, ssize_t *cur_line_pos) {
    // TODO implement with lookup table
    size_t i=0;
    if (UNLIKELY(*cur_line_pos < 0)) {
        *cur_line_pos = fprintf(out, "%u", (unsigned)buf[i++]);
        if (UNLIKELY(*cur_line_pos < 0))
            myFatalErrno("fprintf");
    }
    for (; i<length; i++) {
        int cur_printed = fprintf(out, ",%u", (unsigned)buf[i]);
        if (UNLIKELY(cur_printed < 0))
            myFatalErrno("fprintf");
        *cur_line_pos += cur_printed;
    }
}

