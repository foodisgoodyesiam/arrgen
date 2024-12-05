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
#define _GNU_SOURCE
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if defined(__CYGWIN__) || defined(_WIN64) || defined(_WIN32)
#   define PATH_MAX 1000 //should be 260 but whatever
#elif defined(__APPLE__)
#   include <sys/syslimits.h>
// TODO: figure out why defined(__has_include) didn't work here on ToughGuy, but did on Midna
#elif defined __linux__
#   include <linux/limits.h>
#else
#   warning "could not detect PATH_MAX, defaulting to 4096"
#   define PATH_MAX 4096
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

void initializeLookup(uint8_t base, bool aligned);

void writeArrayContentsDec(const uint8_t *buf, size_t length) ATTR_ACCESS(read_only (1, 2));

#endif // ARRGEN_H_INCLUDED

