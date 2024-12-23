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
#include <stdlib.h>
#include "errors.h"
#include "handlefile.h"
#include "writearray.h"
#include "c_string_stuff.h"
#include "parameters.h"
#include "version_message.h"

#define VERSION "0.4..next"

#define DEFAULT_C_PATH "gen_arrays.c"
#define DEFAULT_H_NAME "gen_arrays.h"

static const char HELPTEXT[] =
    "arrgen version " VERSION ". Copyright 2024 Steven Marion\n"
    "Generates C arrays representing the contents of files, to embed files directly in compiled programs\n"
    "Usage:\n"
    "arrgen [OPTIONS]... FILE1 FILE2      Create a gen_arrays.c file with default parameters\n"
    "arrgen [OPTIONS]... -f FILE          Create a .c file with parameters and inputs loaded from FILE\n"
    "Options:\n"
    "    --help          Display this help text\n"
    "    --version       Display version info\n"
    "    --              End flag arguments, all following treated as input files\n"
    "-h                  Create header file (default)\n"
    "-H                  Do not create header file\n"
    "-a                  Vertically align the columns in generated arrays\n"
    "-A                  Do not vertically align (default)\n"
    "-c                  Make the array const (default)\n"
    "-C                  Do not make the array const\n"
    "-l                  Make the array lengths in the generated header macros (default)\n"
    "-L                  Make the array lengths in the generated header constexpr size_t\n"
    "    --base=         Use numerical base (8, 10, or 16) for arrays. Default 10\n"
    "-d                  Decimal (shortcut for --base=10)\n"
    "-x                  Hexadecimal (shortcut for --base=16)\n"
    "-8                  Octal (shortcut for --base=8)\n"
    "    --attributes=   In generated header, add attributes (eg __attribute__ ((whatever))) after declarations. Default off. can be used for memory alignment\n"
    "    --line_length=  Max num input bytes to print per line. Default 0 (no limit)\n"
    "    --c_path=       Put the generated .c file at this location. Default " DEFAULT_C_PATH "\n"
    "    --h_name=       Put the generated (or referenced) header file at this location relative to the .c file. Default " DEFAULT_H_NAME "\n"
    "TODO describe defaults and input file format\n"
    "TODO update this help text to match latest updates\n"
    ;

// TODO: include other build settings and preprocessor defines? feature test results?
static const char* VERSIONTEXT =
    "arrgen version " VERSION ". Copyright © 2024 Steven Marion\n"
    ARRGEN_VERSION_MESSAGE
    ;

static void parseParamsFile(const char* path)
    ATTR_NONNULL;

const char* program_name_;

int main(int arg_num, const char** args) {
    DLOG("arrgen_pagesize_ = %u", arrgen_pagesize_);
    DLOG("current_params_size_ = %zu", current_params_size_);
    program_name_ = args[0];

    params_ = malloc(current_params_size_);
    if (UNLIKELY(params_==NULL))
        myFatalErrno("failed to allocate %zu bytes", current_params_size_);
    params_->c_path = NULL;
    params_->h_name = NULL;
    params_->params_file = NULL;
    params_->create_header = true;
    params_->constexpr_length = false;
    params_->num_inputs = 0;

    bool flags_end_found = false;
    bool skip_second_arg = false;
    for (int i=1; i<arg_num; i+=(1+skip_second_arg)) {
        skip_second_arg = false;
        if (!flags_end_found && LIKELY(args[i][0]=='-')) {
            if (args[i][1]=='-') {
                // the lookup table probably increases size by a lot, but it should be more maintainable
                if (parseParameterLine(&args[i][2], false)) {
                    // bloop
                } else if (args[i][2]=='\0')
                    flags_end_found = true;
                else if (!strcmp(&args[i][2], "help")) {
                    fwrite(HELPTEXT, strlen(HELPTEXT), 1, stdout);
                    return 0;
                } else if (!strcmp(&args[i][2], "version")) {
                    fwrite(VERSIONTEXT, strlen(VERSIONTEXT), 1, stdout);
                    return 0;
                } else
                    myFatal("unknown long flag %s", args[i]);
            } else
                for (const char* c=&args[i][1]; *c!='\0'; c++)
                    switch (*c) {
                    case 'h': params_->create_header = true; continue;
                    case 'H': params_->create_header = false; continue;
                    case 'l': params_->constexpr_length = false; continue;
                    case 'L': params_->constexpr_length = true; continue;
                    case 'a': defaults_.aligned = true; continue;
                    case 'A': defaults_.aligned = false; continue;
                    case 'c': defaults_.make_const = true; continue;
                    case 'C': defaults_.make_const = false; continue;
                    case 'd': defaults_.base = 10U; continue;
                    case 'x': defaults_.base = 16U; continue;
                    case '8': defaults_.base = 8U; continue;
                    case 'f':
                        if (params_->params_file != NULL)
                            myFatal("you can only give one file if using -f");
                        else if (i+1 == arg_num)
                            myFatal("you passed -f but did not give a file");
                        params_->params_file = args[i+1];
                        skip_second_arg = true;
                        continue;
                    default:
                        myFatal("unknown short flag %c", *c);
                    }
        } else // parse as input file
            newInputFile(args[i], false);
    }

    if (params_->params_file==NULL) {
        if (UNLIKELY(params_->num_inputs == 0))
            myFatal("you forgot to give me any files");
    } else {
        if (UNLIKELY(params_->num_inputs > 0))
            myFatal("%s: cannot give other files on command line if passing settings file %s", params_->inputs[0].path_original, params_->params_file);
        parseParamsFile(params_->params_file);
    }

    // hmm. if I bother to free any of this memory, will need to probably duplicate this string instead of just assigning the pointer
    if (params_->c_path == NULL)
        params_->c_path = DEFAULT_C_PATH;
    if (params_->h_name == NULL)
        params_->h_name = DEFAULT_H_NAME;

    for (size_t i=0; i<params_->num_inputs; i++) {
        InputFileParams *input = &params_->inputs[i];
        // TODO figure out how to track which ones are malloc'd? Or maybe I just don't bother to free them
        if (input->array_name==NULL)
            input->array_name = createCName(input->path_original, strlen(input->path_original), "");
        if (input->length_name==NULL)
            input->length_name = createCName(input->path_original, strlen(input->path_original), "_LENGTH");
        // alignment null is fine
    }

    bool status = handleFile(params_);
    free(params_);
    exit(status ? EXIT_SUCCESS : EXIT_FAILURE);
}

static void parseParamsFile(const char* path) {
    FILE* in = fopen(path, "rt");
    if (UNLIKELY(in==NULL))
        myFatalErrno("%s", path);
    size_t buf_size = PATH_MAX;
    char *buf = malloc(buf_size);
    if (UNLIKELY(buf==NULL))
        myFatalErrno("failed to allocate %zu bytes", buf_size);
    ssize_t num_read;
    unsigned cur_line = 0;
#ifdef ARRGEN_GETLINE_SUPPORTED
    // TODO find a cleaner way to write this ifdef
    while (LIKELY((num_read = getline(&buf, &buf_size, in)) > 0)) {
#else
    while (LIKELY(fgets(buf, PATH_MAX, in)!=NULL)) {
        num_read = strlen(buf);
        if (UNLIKELY(num_read>=(PATH_MAX-1)))
            myFatal("%s: lines too long (system does not have GNU getline)", path);
#endif
        cur_line++;
        // remove the trailing newline if it's there
        if (buf[num_read-1]=='\n') {
            num_read--;
            buf[num_read] = '\0';
        }
        // skip empty lines
        if (UNLIKELY(num_read==0))
            continue;
        // parse the line
        switch (buf[0]) {
        case '#': // it's a comment line
            break;
        case '@': // it's an input file path
            newInputFile(&buf[1], true);
            break;
        case '%': // it's a parameter
            if (!parseParameterLine(&buf[1], true))
                myFatal("%s: line %u: invalid parameter line %s", path, cur_line, buf);
            break;
        default: // it's invalid
            myFatal("%s: %s: currently all non-empty lines must start with %% or #, try --help", path, buf);
        }
    }
    int error = errno;
    if (!LIKELY(feof(in))) {
        errno = error;
        myFatalErrno("%s", path);
    }
    // TODO: free stuff, but only if DEBUG is defined
    free(buf);
}

