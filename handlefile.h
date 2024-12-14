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

#ifndef HANDLEFILE_H_INCLUDED
#define HANDLEFILE_H_INCLUDED
#include "arrgen.h"

typedef struct {
    const char* path;
    const char* length_name;
    const char* array_name;
} InputFileParams;

typedef struct {
    const char* c_path;
    const char* h_path;
    size_t line_length; // TODO use
    bool create_header; // TODO use
    // TODO add more style stuff. base, etc
    size_t num_inputs;
    InputFileParams inputs[];
} OutputFileParams;

ARRGEN_EXTERNC
bool handleFile(const OutputFileParams* params)
    ATTR_ACCESS(read_only(1))
    ATTR_NONNULL;

#endif // HANDLEFILE_H_INCLUDED
