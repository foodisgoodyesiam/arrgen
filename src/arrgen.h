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

#ifndef ARRGEN_H_INCLUDED
#define ARRGEN_H_INCLUDED
// TODO: figure out a better feature test method. Maybe this is an opportunity to learn autotools? Could also use gnulib to get their implementation of getline.
#if defined(__APPLE__) || defined(__linux__) || defined(__CYGWIN__)
#   define ARRGEN_GETLINE_SUPPORTED
#elif defined(__STDC_ALLOC_LIB__)
#   define ARRGEN_GETLINE_SUPPORTED
#   define __STDC_WANT_LIB_EXT2__ 1
#else
#   warning "This system doesn't support GNU getline. TODO find a better solution here"
#endif

#ifndef _GNU_SOURCE // to silence annoying warnings because g++ predefines this and gcc doesn't
#   define _GNU_SOURCE
#endif
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if defined(__CYGWIN__) || defined(_WIN64) || defined(_WIN32)
#   define PATH_MAX 1000 //should be 260 but whatever
#elif defined(__APPLE__)
#   include <sys/syslimits.h>
#elif defined __linux__
#   include <linux/limits.h>
#else
#   warning "could not detect PATH_MAX, defaulting to 4096"
#   define PATH_MAX 4096
#endif

#ifdef __has_include
#   if __has_include(<sys/mman.h>)
#       define ARRGEN_MMAP_SUPPORTED
#   endif
#elif defined(__linux__) || defined(__APPLE__)
#   define ARRGEN_MMAP_SUPPORTED
#endif

#ifndef __has_attribute
#   define __has_attribute(a) 0
#   define ARRGEN_H_TEMP_HAS_ATTRIBUTE // to not mess up headers included after this
#endif
// TODO: add backup checks based on GCC version, if it's GCC and attribute was introduced before __has_attribute
#if __has_attribute(nonnull)
#   define ATTR_NONNULL __attribute__ ((nonnull))
#   define ATTR_NONNULL_N(a) __attribute__ ((nonnull (a)))
#else
#   define ATTR_NONNULL
#   define ATTR_NONNULL_N(a)
#endif
#if __has_attribute(access)
#   define ATTR_ACCESS(...) __attribute__ ((access (__VA_ARGS__)))
#else
#   define ATTR_ACCESS(...)
#endif
#if __has_attribute(leaf)
#   define ATTR_LEAF __attribute__ ((leaf))
#else
#   define ATTR_LEAF
#endif
#if __has_attribute(cold)
#   define ATTR_COLD __attribute__ ((cold))
#else
#   define ATTR_COLD
#endif
#if __has_attribute(hot)
#   define ATTR_HOT __attribute__ ((hot))
#else
#   define ATTR_HOT
#endif
#if __has_attribute(format)
#   define ATTR_FORMAT(...) __attribute__ ((format(__VA_ARGS__)))
#else
#   define ATTR_FORMAT(...)
#endif
#if __has_attribute(nothrow)
#   define ATTR_NOTHROW __attribute__ ((nothrow))
#else
#   define ATTR_NOTHROW
#endif
#if __has_attribute(nonstring)
#   define ATTR_NONSTRING __attribute__ ((nonstring))
#else
#   define ATTR_NONSTRING
#endif
#if __has_attribute(malloc)
#   define ATTR_MALLOC __attribute__ ((malloc))
#else
#   define ATTR_MALLOC
#endif
#if __has_attribute(unused)
#   define ATTR_UNUSED __attribute__ ((unused))
#else
#   define ATTR_UNUSED
#endif
#if __has_attribute(returns_nonnull)
#   define ATTR_RETURNS_NONNULL __attribute__ ((returns_nonnull))
#else
#   define ATTR_RETURNS_NONNULL
#endif
#if __has_attribute(const)
#   define ATTR_CONST __attribute__ ((const))
#else
#   define ATTR_CONST
#endif
#if __STDC_VERSION__ >= 202000L
#   define ATTR_NODISCARD [[nodiscard]]
#elif __has_attribute(warn_unused_result)
#   define ATTR_NODISCARD __attribute__ ((warn_unused_result))
#else
#   define ATTR_NODISCARD
#endif
#ifdef ARRGEN_H_TEMP_HAS_ATTRIBUTE
#   undef __has_attribute // to not mess up headers included after this
#endif

#if defined(__GNUC__) || defined(__clang__)
#   define LIKELY(a) __builtin_expect((a), true)
#   define UNLIKELY(a) __builtin_expect((a), false)
#else
#   define LIKELY(a) (a)
#   define UNLIKELY(a) (a)
#endif

#ifdef VERBOSE_DEBUG
#   include <stdio.h>
#   define E_RED_BG            "\e[1;41m" //red background
#   define E_GREEN_BG          "\e[1;42m" //green background. is hard to read...
#   define E_ORANGE_BG         "\e[1;43m" //orange background
#   define E_BLUE_BG           "\e[1;44m" //blue background
#   define E_PURPLE_BG         "\e[1;45m" //purple background
#   define E_RED_FG            "\e[1;31m" //red text
#   define E_GREEN_FG          "\e[1;32m" //green text
#   define E_ORANGE_FG         "\e[1;33m" //orange text
#   define E_BLUE_FG           "\e[1;34m" //blue text
#   define E_PURPLE_FG         "\e[1;35m" //purple text
#   define E_RESET             "\e[0m" //reset terminal formatting to default
#   define DLOG(msg, ...) fprintf(stderr, E_RED_FG "DEBUG:" E_RESET " %s: " msg "\n", __PRETTY_FUNCTION__, ##__VA_ARGS__)
#else
#   define DLOG(msg, ...)
#endif

#ifndef ARRGEN_BUFFER_SIZE
#   define ARRGEN_BUFFER_SIZE 65536U
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
extern unsigned arrgen_pagesize_;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ARRGEN_H_INCLUDED

