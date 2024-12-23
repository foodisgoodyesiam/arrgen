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

#ifndef PARAMETERS_H_INCLUDED
#define PARAMETERS_H_INCLUDED
#include "arrgen.h"
#include "handlefile.h"

typedef struct ArrgenParameter {
    int name_offset;
    void (*const handler)(const char*, InputFileParams*, bool);
    const bool valid_global;
    const bool valid_individual;
} ArrgenParameter;

#ifdef __cplusplus
extern "C" {
#define register
#endif // __cplusplus

// TODO: is this usage of read_only actually right, for strings where the null terminator is one past the length indicated by len?>
const struct ArrgenParameter* identifyParameter(register const char *str, register size_t len)
    ATTR_ACCESS(read_only, 1, 2)
    ATTR_PURE
    ATTR_NONNULL;

// TODO find a cleaner solution that doesn't give current_params_size_ external linkage.
// there's a lot of fan-out between parameters.c and parameters.h... can I clean this up? do I need to?
extern OutputFileParams *params_;
extern size_t current_params_size_; // current size of the allocated buffer for params
extern InputFileParams defaults_;

// increment the number of inputs, and initialize the parameters of the newly added input file
void newInputFile(const char* path, bool from_params_file)
    ATTR_NONNULL;
bool parseParameterLine(const char* arg, bool from_params_file)
    ATTR_NONNULL;

void registerCPath(const char* str, InputFileParams* params ATTR_UNUSED, bool from_params_file)
    ATTR_NONNULL;
void registerHName(const char* str, InputFileParams* params ATTR_UNUSED, bool from_params_file ATTR_UNUSED)
    ATTR_NONNULL;
void registerCreateHeader(const char* str, InputFileParams* params ATTR_UNUSED, bool from_params_file ATTR_UNUSED)
    ATTR_NONNULL;
void registerLengthName(const char* str, InputFileParams* params, bool from_params_file ATTR_UNUSED)
    ATTR_NONNULL;
void registerArrayName(const char* str, InputFileParams* params, bool from_params_file ATTR_UNUSED)
    ATTR_NONNULL;
void registerAttributes(const char* str, InputFileParams* params, bool from_params_file ATTR_UNUSED)
    ATTR_NONNULL;
void registerLineLength(const char* str, InputFileParams* params, bool from_params_file ATTR_UNUSED)
    ATTR_NONNULL;
void registerBase(const char* str, InputFileParams* params, bool from_params_file ATTR_UNUSED)
    ATTR_NONNULL;
void registerAligned(const char* str, InputFileParams* params, bool from_params_file ATTR_UNUSED)
    ATTR_NONNULL;
void registerMakeConst(const char* str, InputFileParams* params, bool from_params_file ATTR_UNUSED)
    ATTR_NONNULL;
void registerConstexpr(const char* str, InputFileParams* params ATTR_UNUSED, bool from_params_file ATTR_UNUSED)
    ATTR_NONNULL;

#ifdef __cplusplus
}
#undef register
#endif // __cplusplus
#endif // PARAMETERS_H_INCLUDED
