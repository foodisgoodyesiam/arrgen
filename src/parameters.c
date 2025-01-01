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
    .path_original = NULL,
    .path_to_open = NULL,
    .length_name = NULL,
    .array_name = NULL,
    .attributes = NULL,
    .line_length = 0U,
    .base = 10U,
    .aligned = false, // whether or not to print numbers in fixed-width columns
    .make_const = true,
};

void newInputFile(const char* path, bool from_params_file) {
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
    input->path_original = duplicateString(path);
    if (defaults_.attributes!=NULL)
        input->attributes = duplicateString(defaults_.attributes);
    input->path_to_open = (from_params_file ? pathRelativeToFile(params_->params_file, path) : input->path_original);
}

bool parseParameterLine(const char* arg, bool from_params_file) {
    const char* equals_pos = strchr(arg, '=');
    if (UNLIKELY(equals_pos==NULL))
        return false;
    size_t parameter_name_length = equals_pos-arg;
    bool defaults_end_reached = params_->num_inputs > 0;
    const ArrgenParameter *parameter = identifyParameter(arg, parameter_name_length);
    if (LIKELY(parameter!=NULL)) {
        if (defaults_end_reached) {
            if (UNLIKELY(!parameter->valid_individual))
            myFatal("%s: global-only parameters must precede parameters specific to input files", arg);
        } else if (UNLIKELY(!parameter->valid_global))
            myFatal("%s: parameter must follow a specific input file", arg);
        parameter->handler(equals_pos+1, defaults_end_reached ? &params_->inputs[params_->num_inputs-1] : &defaults_, from_params_file);
        return true;
    }
    return false;
}

void registerCPath(const char* str, InputFileParams* params ATTR_UNUSED, bool from_params_file) {
    if (UNLIKELY(params_->c_path!=NULL))
        myFatal("cannot give %s more than once", "c_path");
    // is there a point to asserting non-null? a segfault is already a strong assertion
    //assert(params_->params_file!=NULL);
    params_->c_path = (from_params_file ? pathRelativeToFile(params_->params_file, str) : duplicateString(str));
}

void registerHName(const char* str, InputFileParams* params ATTR_UNUSED, bool from_params_file ATTR_UNUSED) {
    if (UNLIKELY(params_->h_name!=NULL))
        myFatal("cannot give %s more than once", "h_name");
    params_->h_name = duplicateString(str);
}

void registerExtraHeader(const char* str, InputFileParams* params ATTR_UNUSED, bool from_params_file ATTR_UNUSED) {
    params_->header_top_text = sprintfAppend(params_->header_top_text, "#include \"%s\"\n", str);
}

void registerExtraSystemHeader(const char* str, InputFileParams* params ATTR_UNUSED, bool from_params_file ATTR_UNUSED) {
    params_->header_top_text = sprintfAppend(params_->header_top_text, "#include <%s>\n", str);
}

void registerCreateHeader(const char* str, InputFileParams* params ATTR_UNUSED, bool from_params_file ATTR_UNUSED) {
    params_->create_header = parseBool(str, "create_header");
}

void registerLengthName(const char* str, InputFileParams* params, bool from_params_file ATTR_UNUSED) {
    if (UNLIKELY(params->length_name!=NULL))
        myFatal("cannot give %s for a target more than once", "length_name");
    params->length_name = duplicateString(str);
}

void registerArrayName(const char* str, InputFileParams* params, bool from_params_file ATTR_UNUSED) {
    if (UNLIKELY(params->length_name!=NULL))
        myFatal("cannot give %s for a target more than once", "array_name");
    params->array_name = duplicateString(str);
}

void registerAttributes(const char* str, InputFileParams* params, bool from_params_file ATTR_UNUSED) {
    params->attributes = sprintfAppend(params->attributes, "%s\n", str);
}

void registerLineLength(const char* str, InputFileParams* params, bool from_params_file ATTR_UNUSED) {
    params->line_length = parseUint32(str, strlen(str));
}

void registerBase(const char* str, InputFileParams* params, bool from_params_file ATTR_UNUSED) {
    if (!strcmp(str, "16"))
        params->base = 16U;
    else if (!strcmp(str, "10"))
        params->base = 10U;
    else if (!strcmp(str, "8"))
        params->base = 8U;
    else
        myFatal("invalid base %s", str);
}

void registerAligned(const char* str, InputFileParams* params, bool from_params_file ATTR_UNUSED) {
    params->aligned = parseBool(str, "aligned");
}

void registerMakeConst(const char* str, InputFileParams* params, bool from_params_file ATTR_UNUSED) {
    params->make_const = parseBool(str, "const");
}

void registerConstexpr(const char* str, InputFileParams* params ATTR_UNUSED, bool from_params_file ATTR_UNUSED) {
    params_->constexpr_length = parseBool(str, "constexpr_length");
}


