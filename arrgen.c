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
 */

#include <stdio.h>
#include <stdint.h>
#if defined(__has_include) && __has_include(<sys/mman.h>)
#   include <sys/mman.h>
#   define MMAP_SUPPORTED
#endif

static int handleFile(const char* in_path, const char* out_path);

int main(int arg_num, const char** args) {
    printf("Initial commit woo!\n");
}

static int handleFile(const char* in_path, const char* out_path) {
    //TODO
}
