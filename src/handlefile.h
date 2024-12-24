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
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// why are these defined here and not in parameters.h?

typedef struct {
    const char* path_original; // path to file, as originally specified by user
    const char* path_to_open; // path to file, relative to current working directory (may be different because above can be relative to parameter file, if specified in parameter file)
    const char* length_name;
    const char* array_name;
    const char* attributes;
    uint32_t line_length;
    uint8_t base;
    bool aligned;
    bool make_const;
} InputFileParams;

typedef struct {
    const char* c_path; // file path of the c file to generate, relative to current working directory
    const char* h_name; // file path of the header, relative to the directory containing the c file
    char* header_top_text; // extra lines to insert in the top of the generated header file, verbatim (ie relative to the header file)
    const char* params_file; // the file the settings were loaded from, if any
    bool create_header;
    bool constexpr_length; // make the lengths constexpr instead of defines
    size_t num_inputs;
    InputFileParams inputs[];
} OutputFileParams;

bool handleFile(const OutputFileParams* params)
    ATTR_ACCESS(read_only, 1)
    ATTR_NONNULL;

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // HANDLEFILE_H_INCLUDED
