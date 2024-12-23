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
#include "c_string_stuff.h"
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "errors.h"

#if defined(__GNUC__) || defined(__clang__)
#   define USE_FANCY_INT_PARSING
static uint8_t parseDigit(char c, uint8_t base)
    ATTR_CONST;

#else
#   include <stdlib.h>
#   include <errno.h>
#endif // 0

static uint8_t parseBase(char c)
    ATTR_CONST;

#define NAME_PREFIX "ARRGEN_"

// hmm. is there an alternative to malloc for this?
char* createCName(const char* name ATTR_NONSTRING, size_t name_length, const char* suffix) {
    const size_t prefix_length = strlen(NAME_PREFIX);
    const size_t suffix_length = strlen(suffix);
    const size_t out_length = prefix_length+name_length+suffix_length+1; // +1 for null terminator
    char *ret = malloc(out_length);
    if (UNLIKELY(ret==NULL))
        myFatalErrno("malloc");
    memcpy(ret, NAME_PREFIX, prefix_length);
    for (size_t i=0; i<name_length; i++) {
        if (name[i]>='a' && name[i]<='z')
            ret[prefix_length+i] = name[i]+('A'-'a');
        else if ((name[i]>='A' && name[i]<='Z') || (name[i]>='0' && name[i]<='9'))
            ret[prefix_length+i] = name[i];
        else
            ret[prefix_length+i] = '_';
    }
    memcpy(&ret[prefix_length+name_length], suffix, suffix_length+1);
    return ret;
}

char* pathRelativeToFile(const char* base_file_path, const char* relative_path) {
    // this could be optimized in many ways, but it's so far from a bottleneck that it doens't matter
    size_t ret_length = strlen(base_file_path) + strlen(relative_path);
    char *ret = malloc(ret_length+1); // +2 for null terminator, in the worst case...? worse than the worst case
    if (UNLIKELY(ret==NULL))
        myFatal("could not allocate %zu bytes", ret_length+1);
    strcpy(ret, base_file_path);
    char* spot_to_insert_relative_path = strrchr(ret, '/');
    if (spot_to_insert_relative_path==NULL)
        spot_to_insert_relative_path = ret; // means there are no directory separators, so just copy relative_path into the return string
    else
        spot_to_insert_relative_path++; // means insert it after the last /
    strcpy(spot_to_insert_relative_path, relative_path);
    DLOG("base_file_path: %s, relative_path: %s, resulting concatenation: %s, allocated size: %zu, actual size: %zu", base_file_path, relative_path, ret, ret_length, strlen(ret));
    return ret;
}

#if __STDC_VERSION__ < 202000L
char* duplicateString(const char* str) {
    return duplicateStringLen(str, strlen(str));
}
#endif

char* duplicateStringLen(const char* str ATTR_NONSTRING, size_t length) {
    char* ret = malloc(length+1);
    if (UNLIKELY(ret==NULL))
        myFatal("could not allocate %zu bytes", length+1);
    memcpy(ret, str, length);
    ret[length] = '\0';
    return ret;
}

#ifdef ARRGEN_USE_CUSTOM_BASENAME
const char* customBasename(const char* path) {
    const char* ret = strrchr(path, '/');
    if (ret==NULL)
        return path;
    ret++;
    return ret;
}
#endif // ARRGEN_USE_CUSTOM_BASENAME

// TODO: consider if I should just rely on the strto function? hmm
uint32_t parseUint32(const char* arg, size_t length) {
    uint32_t ret = 0;
    uint8_t base = 10;
    const char* c = arg;
    // check if it's not base 10
    if (UNLIKELY(length>2 && arg[0] == '0' && arg[1] >= 'a' && arg[1] <= 'z')) {
        base = parseBase(arg[1]);
        if (UNLIKELY(base==0))
            myFatal("unrecognized base prefix 0%c", arg[1]);
        c+=2;
    }
    // TODO: consider if I want to intead make it treat empty strings as zero. probably not
    if (UNLIKELY(c==&arg[length]))
        myFatal("empty string is not an integer");
#ifdef USE_FANCY_INT_PARSING
    for (; c!=&arg[length]; c++) {
        uint8_t to_add = parseDigit(*c, base); //this is probably not very optimized for code size
        if (LIKELY(to_add<base)) {
            if (UNLIKELY(__builtin_mul_overflow(ret, base, &ret)) || UNLIKELY(__builtin_add_overflow(ret, to_add, &ret)))
                myFatal("%s: too big", arg);
        } else
            myFatal("%s: not a positive integer", arg);
    }
#else // USE_FANCY_INT_PARSING
    // skip leading zeros so it's not interpreted as octal
    for (; c!=&arg[length] && *c=='0'; c++);
    // annoying that I have to do this to get null termination
    // TODO investigate how smart the compiler is about this pointer arithmetic, vs giving it indices
    size_t num_len = &arg[length]-c;
    char temp_buf[num_len+1];
    memcpy(temp_buf, c, num_len);
    temp_buf[num_len] = '\0';
    char* endptr;
    errno = 0;
    unsigned long long temp_value = strtoull(c, &endptr, base);
    if (UNLIKELY(errno!=0))
        myFatalErrno("%s", arg);
    else if (endptr!=&arg[length])
        myFatal("%s: not a positive integer", arg);
    else if (UNLIKELY(temp_value > UINT32_MAX))
        myFatal("%s: too big", arg);
    ret = temp_value;
#endif // USE_FANCY_INT_PARSING
    DLOG("parsed %s as %" PRIu32 " with base %" PRIu8, arg, ret, base);
    return ret;
}

bool parseBool(const char *potential_bool, const char *param_name) {
    if (!strcmp(potential_bool, "yes") || !strcmp(potential_bool, "true"))
        return true;
    else if (!strcmp(potential_bool, "no") || !strcmp(potential_bool, "false"))
        return false;
    else
        myFatal("%s must be yes or no, not %s", param_name, potential_bool);
}
// TODO: does msvc have equivalents to the builtins?

static uint8_t parseBase(char c) {
    switch (c) {
        case 'b': return 2u;
        case 't': return 3u; // why not
        case 'o': return 8u;
        case 'x': return 16u;
        default: return 0u;
    }
}

#ifdef USE_FANCY_INT_PARSING
static uint8_t parseDigit(char c, uint8_t base) {
    if (c>='a')
        return c-'a'+10;
    else if (c>='A')
        return c-'A'+10;
    else if (LIKELY(c>='0'))
        return c - '0';
    return base; //this would need to use base more if any weirder bases were added... eg base64
}
#endif // USE_FANCY_INT_PARSING

