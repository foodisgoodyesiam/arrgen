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
 * TODO:
 *   -make line_length_ configurable
 *   -make option to generate the header or not
 *   -make it possible to configure other stuff
 *   -make it write to named temporary files? is that how it should work? or somehow clean up if it fails partway through
 *   -make it parse arguments, including at least --help and --version
 *   -write help text
 *   -make it possible to specify macro for array length, and name of array
 *   -make it correctly generate relative path from .c file to .h file for header inclusion?
 *   -make it willing to read from standard input
 *   -make a myError function to reduce code size from all these error messages (or add libsvenmar as a dependency)
 *   -make it have an option to print the license, that's required by GPL right?
 *   -optimize for: speed of this program, generated file size, speed of compiling the generated file
 *   -add options for generated text mode, something like: compact, aligned hex, octal, aligned decimal, etc...
 */
#define _GNU_SOURCE


#if defined(__has_include) && __has_include(<sys/mman.h>)
#   include <sys/mman.h>
#   include <sys/stat.h>
#   include <unistd.h>
#   include <fcntl.h>
#   define MMAP_SUPPORTED
#endif

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include "arrgen.h"

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

static const char HELPTEXT[] =

    "TODO\n";

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
    bool create_header; // TODO use
} ArrgenParams;

static bool handleFile(const ArrgenParams* params) ATTR_NONNULL;
static bool writeH(const ArrgenParams* params, size_t length) ATTR_NONNULL;
static bool writeC(const ArrgenParams* params, const uint8_t* buf, size_t length) ATTR_NONNULL;

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
    bool ret;
    size_t length;
#ifdef MMAP_SUPPORTED
    int fd = open(params->in_path, O_RDONLY);
    if (UNLIKELY(fd<0)) {
        fprintf(stderr, "%s: %s: could not open: %s\n", program_name_, params->in_path, strerror(errno));
        ret = false;
    } else {
        struct stat stats;
        if (UNLIKELY(fstat(fd, &stats)!=0)) {
            fprintf(stderr, "%s: %s: could not fstat: %s\n", program_name_, params->in_path, strerror(errno));
            ret = false;
        } else {
            switch (stats.st_mode & S_IFMT) {
            case S_IFREG: {
                const uint8_t* mem = (const uint8_t*) mmap(NULL, stats.st_size, PROT_READ, MAP_SHARED, fd, 0);
                if (UNLIKELY(mem==MAP_FAILED)) {
                    fprintf(stderr, "%s: %s: mmap: %s\n", program_name_, params->in_path, strerror(errno));
                    ret = false;
                } else {
                    ret = writeC(params, mem, stats.st_size);
                    length = stats.st_size;
                    if (UNLIKELY(munmap((void*)mem, stats.st_size))!=0)
                        fprintf(stderr, "%s: %s: munmap: %s\n", program_name_, params->in_path, strerror(errno));
                }
                }; break;
            case S_IFBLK:
                fprintf(stderr, "%s: %s: what the heck are you doing?\n", program_name_, params->in_path);
                ret = false;
                break;
            default:
                // TODO: handle as pipe but that probably needs a restructuring... I guess not a big one
                fprintf(stderr, "%s: %s: currently only regular files are supported, sorry\n", program_name_, params->in_path);
                ret = false;
                break;
            }
        }
        if (UNLIKELY(close(fd)!=0))
            fprintf(stderr, "%s: %s: could not close: %s\n", program_name_, params->in_path, strerror(errno));
    }
#else // MMAP_SUPPORTED
    writeC(params, "TODO stuff", strlen("TODO stuff"));
    // TODO implement using C stdio
#endif // MMAP_SUPPORTED
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
            "\n"
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

static bool writeC(const ArrgenParams* params, const uint8_t* buf, size_t length) {
    FILE *out = fopen(params->c_path, "wb"); // CLRF is icky
    bool ret = true;
    if (UNLIKELY(out==NULL)) {
        fprintf(stderr, "%s: %s: could not open: %s\n", program_name_, params->c_path, strerror(errno));
        ret = false;
    } else {
        // TODO: actually, maybe have the writeC return the number of bytes written... that way it can handle pipes gracefully? do I care about that?
        fprintf(out,
            "#include \"%s\"\n"
            "const unsigned char %s[%s] = {",
            basename(params->h_path),
            params->array_name,
            params->length_name);
        // TODO optimize this, this will be horribly slow
        for (size_t i=0; i<length-1; i++)
            fprintf(out, "%u,", (unsigned)buf[i]);
        if (length>0)
            fprintf(out, "%u", (unsigned)buf[length-1]);
        // TODO: consider whether to optimize by removing trailing zeros in the generated array
        fprintf(out, "};");

        if (UNLIKELY(fclose(out)!=0))
            fprintf(stderr, "%s: %s: could not close: %s\n", program_name_, params->c_path, strerror(errno));
    }
    return (ret);
}
