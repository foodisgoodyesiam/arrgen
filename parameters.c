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
#include "parameters.h"
#include "errors.h"
#include "c_string_stuff.h"
#include <stdlib.h>

OutputFileParams *params_ = NULL; // allocated to the below size at the start of main
size_t current_params_size_ = sizeof(OutputFileParams) + sizeof(InputFileParams)*1;
InputFileParams defaults_ = {
    .path = NULL,
    .length_name = NULL,
    .array_name = NULL,
    .attributes = NULL,
    .line_length = 0U,
    .base = 10U,
    .aligned = false, // whether or not to print numbers in fixed-width columns
    .make_const = true,
};

void newInputFile(const char* path) {
    params_->num_inputs++;
    size_t new_needed_size = sizeof(OutputFileParams) + sizeof(InputFileParams)*(params_->num_inputs);
    if (new_needed_size > current_params_size_) {
        size_t new_size = sizeof(OutputFileParams) + sizeof(InputFileParams)*(params_->num_inputs*2);
        DLOG("reallocating params_ %p from size %zu (%zu elements) to %zu (%zu elements)", params_, current_params_size_, params_->num_inputs-1, new_size, params_->num_inputs*2);
        params_ = (OutputFileParams*)realloc(params_, new_size);
        if (UNLIKELY(params_==NULL))
            myFatalErrno("failed to allocate %zu bytes", new_size);
        current_params_size_ = new_size;
    }
    InputFileParams *input = &params_->inputs[params_->num_inputs-1];
    // initialize to defaults
    *input = defaults_;
    input->path = path;
}

typedef struct {
    const char* const parameter_name;
    void (*const handler)(const char*, InputFileParams*);
    const bool valid_global;
    const bool valid_individual;
} ArrgenParameter;

static const ArrgenParameter ARGUMENTS[] = {
    {"c_path", registerCPath, true, false},
    {"h_path", registerHPath, true, false},
    {"create_header", registerCreateHeader, true, false},
    {"array_name", registerArrayName, false, true},
    {"length_name", registerLengthName, false, true},
    {"attributes", registerAttributes, true, true},
    {"line_length", registerLineLength, true, true},
    {"base", registerBase, true, true},
    {"aligned", registerAligned, true, true},
    {"const", registerMakeConst, true, true},
};

#define NUM_ARGUMENTS (sizeof(ARGUMENTS)/sizeof(ArrgenParameter))

bool parseParameterLine(const char* arg) {
    const char* equals_pos = strchr(arg, '=');
    if (UNLIKELY(equals_pos==NULL))
        return false;
    size_t parameter_name_length = equals_pos-arg;
    // TODO: use gperf, why not
#if 0
    char parameter_name[parameter_name_length+1];
    memcpy(parameter_name, arg, parameter_name_length);
    parameter_name[parameter_name_length] = '\0';
    TODO
#else
    bool defaults_end_reached = params_->num_inputs>0;
    for (unsigned i=0; i<NUM_ARGUMENTS; i++) {
        if (!strncmp(arg, ARGUMENTS[i].parameter_name, parameter_name_length)) {
            if (defaults_end_reached) {
                if (UNLIKELY(!ARGUMENTS[i].valid_global))
                    myFatal("%s: parameter must follow a specific input file", ARGUMENTS[i].parameter_name);
            } else if (UNLIKELY(!ARGUMENTS[i].valid_individual))
                myFatal("%s: global-only parameters must precede parameters specific to input files", ARGUMENTS[i].parameter_name);
            ARGUMENTS[i].handler(equals_pos+1, defaults_end_reached ? &params_->inputs[params_->num_inputs-1] : &defaults_);
            return true;
        }
    }
    return false;
#endif
}

void registerCPath(const char* str, InputFileParams* params ATTR_UNUSED) {
    if (UNLIKELY(params_->c_path!=NULL))
        myFatal("cannot give c_path more than once");
    params_->c_path = duplicateString(str);
}

void registerHPath(const char* str, InputFileParams* params ATTR_UNUSED) {
    if (UNLIKELY(params_->h_name!=NULL))
        myFatal("cannot give h_name more than once");
    params_->h_name = duplicateString(str);
}

void registerCreateHeader(const char* str, InputFileParams* params ATTR_UNUSED) {
    params_->create_header = parseBool(str, "create_header");
}

void registerLengthName(const char* str, InputFileParams* params) {
    params->length_name = duplicateString(str);
}

void registerArrayName(const char* str, InputFileParams* params) {
    params->array_name = duplicateString(str);
}

void registerAttributes(const char* str, InputFileParams* params) {
    params->attributes = duplicateString(str);
}

void registerLineLength(const char* str, InputFileParams* params) {
    params->line_length = parseUint32(str, strlen(str));
}

void registerBase(const char* str, InputFileParams* params) {
    if (!strcmp(str, "16"))
        params->base = 16U;
    else if (!strcmp(str, "10"))
        params->base = 10U;
    else if (!strcmp(str, "8"))
        params->base = 8U;
    else
        myFatal("invalid base %s", str);
}

void registerAligned(const char* str, InputFileParams* params) {
    params->aligned = parseBool(str, "aligned");
}

void registerMakeConst(const char* str, InputFileParams* params) {
    params->make_const = parseBool(str, "const");
}


