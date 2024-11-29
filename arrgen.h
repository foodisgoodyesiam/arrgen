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

#if defined(__has_attribute)
#   if __has_attribute(nonnull)
#       define ATTR_NONNULL __attribute__ ((nonnull))
#   else
#       define ATTR_NONNULL
#   endif
#endif

#if defined(__GNUC__) || defined(__clang__)
#   define LIKELY(a) __builtin_expect((a), true)
#   define UNLIKELY(a) __builtin_expect((a), false)
#else
#   define LIKELY(a) (a)
#   define UNLIKELY(a) (a)
#endif

#endif // ARRGEN_H_INCLUDED

