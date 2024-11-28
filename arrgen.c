/* Copyright © 2019 2024 Steven Marion <steven@dragons.fish>
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
 * TODO:
 *   -make line_length_ configurable
 *   -make option to generate the header or not
 *   -make it possible to configure other stuff
 *   -make it write to named temporary files? is that how it should work? or somehow clean up if it fails partway through
 *   -make it parse arguments, including at least --help and --version
 *   -write help text
 *   -make it possible to specify macro for array length, and name of array
 *   -make it correctly generate relative path from .c file to .h file for header inclusion?
 *   -make 
 */
#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#if defined(__has_include) && __has_include(<sys/mman.h>)
#   include <sys/mman.h>
#   define MMAP_SUPPORTED
#endif

#if defined(__CYGWIN__) || defined(_WIN64) || defined(_WIN32)
#   define PATH_MAX 1000 //should be 260 but whatever
#elif defined(__APPLE__)
#   include <sys/syslimits.h>
#elif defined(__has_include) && __has_include(<linux/limits.h>)
#   include <linux/limits.h>
#else
#   warning "could not detect PATH_MAX, defaulting to 4096"
#   define PATH_MAX 4096
#endif

#if defined(__has_builtin)
#   if __has_builtin(__builtin_expect)
#       define LIKELY(a) __builtin_expect((a), true)
#       define UNLIKELY(a) __builtin_expect((a), false)
#   else
#       define LIKELY(a) (a)
#       define UNLIKELY(a) (a)
#   endif
#endif

#if !defined(__GLIBC__) && !defined(__CYGWIN__)
static const char* basename(const char* path) {
    const char* ret = strrchr(path, '/');
    if (ret==nullptr)
        return path;
    ret++;
    return ret;
}
#endif

static const char* program_name_;

#define VERSION "0.0.0.next"

//static unsigned line_length_ = 400;
static char c_path_[PATH_MAX];
static char h_path_[PATH_MAX];

typedef struct {
    const char* in_path;
    const char* c_path;
    const char* h_path; 
    const char* length_name;
    const char* array_name;
    size_t line_length; // TODO use
} ArrgenParams;

static bool handleFile(const ArrgenParams* params);
static bool writeH(const ArrgenParams* params, size_t length);
static bool writeC(const ArrgenParams* params, uint8_t* buf, size_t length);

int main(int arg_num, const char** args) {
    program_name_ = args[0];
    if (UNLIKELY(arg_num < 2)) {
        fprintf(stderr, "%s: you forgot to give me a file\n", program_name_);
        return 1;
    }
    ArrgenParams params;
    // initial default skeleton, will add better argument parsing later
    params.in_path = args[1];
    int c_path_len = snprintf(c_path_, PATH_MAX, "%s.c", params.in_path);
    if (c_path_len >= (PATH_MAX-1)) {
        fprintf(stderr, "%s: %s.c: output path too long\n", program_name_, params.in_path);
        return 1;
    }
    params.c_path = c_path_;

    int h_path_len = snprintf(h_path_, PATH_MAX, "%s.h", params.in_path);
    if (h_path_len >= (PATH_MAX-1)) {
        fprintf(stderr, "%s: %s.h: output path too long\n", program_name_, params.in_path);
        return 1;
    }
    params.h_path = h_path_;
    
    params.length_name = "LENGTH_NAME_TODO";

    params.array_name = "ARRAY_NAME_TODO";

    return handleFile(&params);
}

static bool handleFile(const ArrgenParams* params) {
    bool ret = true;
    size_t length = 10U;
#ifdef MMAP_SUPPORTED
        // TODO
#else // MMAP_SUPPORTED
        // TODO
#endif // MMAP_SUPPORTED
    writeC(params, "TODO\0 stuff", strlen("TODO\0 stuff"));
    return (ret && (params->h_path == NULL || writeH(params, length)));
}

static bool writeH(const ArrgenParams* params, size_t length) {
    FILE *out = fopen(params->h_path, "wb");
    bool ret;
    if (UNLIKELY(out==NULL)) {
        fprintf(stderr, "%s: %s: could not open: %s\n", program_name_, params->h_path, strerror(errno));
        ret = false;
    } else {
        fprintf(out,
            "#define %s %zuU\n"
            "#ifdef __cplusplus\n"
            "extern \"C\" {\n"
            "#endif // __cplusplus\n"
            "\n"
            "extern const unsigned char %s[%s];\n"
            "\n"
            "#ifdef __cplusplus\n"
            "}\n"
            "#endif // __cplusplus\n",
            params->length_name,
            length,
            params->array_name,
            params->length_name);
        if (UNLIKELY(fclose(out)!=0))
            fprintf(stderr, "%s: %s: could not close: %s\n", program_name_, params->h_path, strerror(errno));
        ret = true;
    }
    return (ret);
}

static bool writeC(const ArrgenParams* params, uint8_t* buf, size_t length) {
    FILE *out = fopen(params->c_path, "wb"); // CLRF is icky
    bool ret = true;
    if (UNLIKELY(out==NULL)) {
        fprintf(stderr, "%s: %s: could not open: %s\n", program_name_, params->c_path, strerror(errno));
        ret = false;
    } else {
        size_t array_size = 10; // temporary value
        fprintf(out, "#include \"%s\"\n\n", basename(params->h_path));
        // TODO write the content
        if (UNLIKELY(fclose(out)!=0))
            fprintf(stderr, "%s: %s: could not close: %s\n", program_name_, params->c_path, strerror(errno));
    }
    return (ret);
}
