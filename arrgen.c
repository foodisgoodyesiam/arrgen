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
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <malloc.h>
#include "errors.h"
#include "handlefile.h"
#include "writearray.h"

#define VERSION "0.0.0.next"

static const char HELPTEXT[] =
    "arrgen version " VERSION ". Copyright 2024 Steven Marion\n"
    "Generates C arrays representing the contents of files, to embed files directly in compiled programs\n"
    "Usage:\n"
    "arrgen [OPTIONS]... FILE1 FILE2      Create a gen_arrays.c file with default parameters\n"
    "arrgen [OPTIONS]... -f FILE          Create a .c file with parameters and inputs loaded from FILE\n"
    "TODO\n"
    ;

static char c_path_[PATH_MAX];
static char h_name_[PATH_MAX];
const char* program_name_;

int main(int arg_num, const char** args) {
    DLOG("arrgen_pagesize_ = %u", arrgen_pagesize_);
    program_name_ = args[0];
    if (UNLIKELY(arg_num < 2))
        myFatal("you forgot to give me a file");

    uint8_t base = 10U; // must be 8, 10, or 16
    bool aligned = false;
    initializeLookup(base, aligned);

    static OutputFileParams* params;
    // initial default skeleton, will add better argument parsing later
    params = malloc(sizeof(OutputFileParams) + sizeof(InputFileParams)*1);
    params->num_inputs = 1;
    if (UNLIKELY(params==NULL))
        myFatalErrno("failed to allocate memory");

    params->inputs[0].path = args[1];
    int c_path_len = snprintf(c_path_, PATH_MAX, "%s.c", params->inputs[0].path);
    if (c_path_len >= (PATH_MAX-1))
        myFatal("%s.c: output path too long", params->inputs[0].path);
    params->c_path = c_path_;

    int h_name_len = snprintf(h_name_, PATH_MAX, "%s.h", ARRGEN_BASENAME(params->inputs[0].path));
    params->h_name = h_name_;

    params->create_header = true;

    params->line_length = 0U;

    params->inputs[0].length_name = "LENGTH_NAME_TODO";

    params->inputs[0].array_name = "ARRAY_NAME_TODO";

    bool status = handleFile(params);
    free(params);
    return !status;
}

