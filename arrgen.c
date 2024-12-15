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

#define VERSION "0.0.1.next"

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
    "    --base=         Use numerical base (8, 10, or 16) for arrays. Default 10\n"
    "-d                  Decimal (shortcut for --base=10)\n"
    "-x                  Hexadecimal (shortcut for --base=16)\n"
    "-8                  Octal (shortcut for --base=8)\n"
    "    --alignment=    In generated header, add __attribute__ ((aligned)) with specified alignment. Default 0 (off)\n"
    "    --line_length=  Max num input bytes to print per line. Default 0 (no limit)\n"
    "    --c_path=       Put the generated .c file at this location. Default " DEFAULT_C_PATH "\n"
    "    --h_name=       Put the generated (or referenced) header file at this location relative to the .c file. Default " DEFAULT_H_NAME "\n"
    "TODO describe defaults and input file format\n"
    ;

static OutputFileParams *params_ = NULL;
static size_t current_params_size_ = sizeof(OutputFileParams) + sizeof(InputFileParams)*1; // current size of the allocated buffer for params
static const char* params_file_ = NULL;
static uint32_t default_alignment_ = 0U; // default memory alignment of generated arrays. 0 = no alignment
static size_t default_line_length_ = 0U;
static uint8_t default_base_ = 10U;
static bool default_aligned_ = false; // whether or not to print numbers in fixed-width columns

const char* program_name_;

static void addInputParam(const char* path);
static uint8_t parseBase(const char* str);
static uint32_t parseAlignment(const char* str);
static size_t parseLineLength(const char* str);

int main(int arg_num, const char** args) {
    DLOG("arrgen_pagesize_ = %u", arrgen_pagesize_);
    DLOG("current_params_size_ = %zu", current_params_size_);
    program_name_ = args[0];

    params_ = malloc(current_params_size_);
    if (UNLIKELY(params_==NULL))
        myFatalErrno("failed to allocate memory");
    params_->c_path = NULL;
    params_->h_name = NULL;
    params_->create_header = true;
    params_->num_inputs = 0;

    bool flags_end_found = false;
    bool skip_second_arg = false;
    for (int i=1; i<arg_num; i+=(1+skip_second_arg)) {
        skip_second_arg = false;
        if (!flags_end_found && LIKELY(args[i][0]=='-')) {
            if (args[i][1]=='-') {
                // TODO: use a lookup table here to reduce the duplication and code size
                if (args[i][2]=='\0')
                    flags_end_found = true;
                else if (!strcmp(&args[i][2], "help")) {
                    fwrite(HELPTEXT, strlen(HELPTEXT), 1, stdout);
                    return 0;
                } else if (!strcmp(&args[i][2], "help")) {
                    fprintf(stdout, "arrgen version " VERSION ". Copyright 2024 Steven Marion\n");
                    return 0;
                } else if (!strncmp(&args[i][2], "c_path=", strlen("c_path="))) {
                    if (UNLIKELY(params_->c_path != NULL))
                        myFatal("cannot give c_path more than once");
                    params_->c_path = &args[i][strlen("--c_path=")];
                } else if (!strncmp(&args[i][2], "h_name=", strlen("h_name="))) {
                    if (UNLIKELY(params_->h_name != NULL))
                        myFatal("cannot give h_name more than once");
                    params_->h_name = &args[i][strlen("--h_name=")];
                } else if (!strncmp(&args[i][2], "alignment=", strlen("alignment="))) {
                    default_alignment_ = parseAlignment(&args[i][strlen("--alignment=")]);
                } else if (!strncmp(&args[i][2], "line_length=", strlen("line_length="))) {
                    default_line_length_ = parseLineLength(&args[i][strlen("--line_length=")]);
                } else if (!strncmp(&args[i][2], "base=", strlen("base=")))
                    default_base_ = parseBase(&args[i][strlen("--base=")]);
                else
                    myFatal("unknown long flag %s", args[i]);
            } else
                for (const char* c=&args[i][1]; *c!='\0'; c++)
                    switch (*c) {
                    case 'h': params_->create_header = true; continue;
                    case 'H': params_->create_header = false; continue;
                    case 'a': default_aligned_ = true; continue;
                    case 'A': default_aligned_ = false; continue;
                    case 'd': default_base_ = 10U; continue;
                    case 'x': default_base_ = 16U; continue;
                    case '8': default_base_ = 8U; continue;
                    case 'f':
                        if (params_file_ != NULL)
                            myFatal("you can only give one file if using -f");
                        else if (i+1 == arg_num)
                            myFatal("you passed -f but did not give a file");
                        params_file_ = args[i+1];
                        skip_second_arg = true;
                        break;
                    default:
                        myFatal("unknown short flag %c", *c);
                    }
        } else // parse as input file
            addInputParam(args[i]);
    }

    if (params_file_==NULL) {
        if (UNLIKELY(params_->num_inputs == 0))
            myFatal("you forgot to give me any files");
    } else {
        if (UNLIKELY(params_->num_inputs >= 0))
            myFatal("%s: cannot give other files on command line if passing settings file %s", params_->inputs[0].path, params_file_);
        // TODO
        myFatal("TODO: settings file parsing not implemented yet");
    }

    // these current defaults make it not usable yet
    if (params_->c_path == NULL)
        params_->c_path = DEFAULT_C_PATH;
    if (params_->h_name == NULL)
        params_->h_name = DEFAULT_H_NAME;

    for (size_t i=0; i<params_->num_inputs; i++) {
        InputFileParams *input = &params_->inputs[i];
        // TODO around here: go through params and set the remaining NULLs to defaults. figure out how to track which ones are malloc'd? Or maybe I just don't bother to free them
        if (input->array_name==NULL)
            input->array_name = "ARRAY_NAME_TODO";
        if (input->length_name==NULL)
            input->length_name = "LENGTH_NAME_TODO";
    }

    bool status = handleFile(params_);
    free(params_);
    return !status;
}

static void addInputParam(const char* path) {
    params_->num_inputs++;
    size_t new_needed_size = sizeof(OutputFileParams) + sizeof(InputFileParams)*(params_->num_inputs);
    if (new_needed_size > current_params_size_) {
        size_t new_size = sizeof(OutputFileParams) + sizeof(InputFileParams)*(params_->num_inputs*2);
        DLOG("reallocating params_ %p from size %zu (%zu elements) to %zu (%zu elements)", params_, current_params_size_, params_->num_inputs-1, new_size, params_->num_inputs*2);
        params_ = (OutputFileParams*)realloc(params_, new_size);
        if (UNLIKELY(params_==NULL))
            myFatalErrno("could not allocate memory");
        current_params_size_ = new_size;
    }
    InputFileParams *input = &params_->inputs[params_->num_inputs-1];
    input->path = path;
    // initialize to defaults
    input->aligned = default_aligned_;
    input->base = default_base_;
    input->alignment = default_aligned_;
    input->line_length = default_line_length_;
    // these will be initialized later. hmm, this is not a clean way to do it... TODO clean up?
    input->length_name = NULL;
    input->array_name = NULL;
}

static uint8_t parseBase(const char* str) {
    if (!strcmp(str, "16"))
        return 16U;
    else if (!strcmp(str, "10"))
        return 10U;
    else if (!strcmp(str, "8"))
        return 8U;
    else
        myFatal("invalid base %s", str);
}

static uint32_t parseAlignment(const char* str) {
    // TODO
    myFatal("TODO alignment parsing not implemented yet");
}

static size_t parseLineLength(const char* str) {
    // TODO
    myFatal("TODO line length parsing not implemented yet");
}

