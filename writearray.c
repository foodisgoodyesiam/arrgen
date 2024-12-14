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

void initializeLookup(uint8_t base, bool aligned) {
    // TODO
}

// TODO: make it return error information
void writeArrayContents(FILE* out, const uint8_t *buf, size_t length) {
    // TODO implement with lookup table
    for (size_t i=0; i<length; i++)
        fprintf(out, "%u,", (unsigned)buf[i]);
}

