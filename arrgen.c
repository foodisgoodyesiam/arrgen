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

static bool handleFile(const char* in_path, const char* c_path, const char* h_path);

int main(int arg_num, const char** args) {
    program_name_ = args[0];
    if (UNLIKELY(arg_num < 2)) {
        printf("%s: you forgot to give me a file\n", program_name_);
        return 1;
    }
    const char* in_path = args[1];
    int c_path_len = snprintf(c_path_, PATH_MAX, "%s.c", in_path);
    if (c_path_len >= (PATH_MAX-1)) {
        printf("%s: %s.c: output path too long\n", program_name_, in_path);
        return 1;
    }
    int h_path_len = snprintf(h_path_, PATH_MAX, "%s.h", in_path);
    if (h_path_len >= (PATH_MAX-1)) {
        printf("%s: %s.h: output path too long\n", program_name_, in_path);
        return 1;
    }
    return handleFile(in_path, c_path_, h_path_) ? 1 : 0;
}

static bool handleFile(const char* in_path, const char* c_path, const char* h_path) {
    FILE* out = fopen(c_path, "wb"); // CLRF is icky
    bool ret;
    if (UNLIKELY(out==NULL)) {
        printf("%s: %s: could not open: %s\n", program_name_, c_path, strerror(errno));
        ret = false;
    } else {
        fprintf(out, "#include \"%s\"\n\n", basename(h_path));
#ifdef MMAP_SUPPORTED
        // TODO
#else // MMAP_SUPPORTED
        // TODO
#endif // MMAP_SUPPORTED
        if (UNLIKELY(fclose(out)!=0))
            printf("%s: %s: could not close: %s\n", program_name_, c_path, strerror(errno));
    }
    return ret;
}
