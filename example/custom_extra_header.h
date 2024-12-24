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
 *
 * This header demonstrates a potential use of the extra header inclusion.
 */

#ifndef CUSTOM_EXTRA_HEADER_H_INCLUDED
#define CUSTOM_EXTRA_HEADER_H_INCLUDED

#ifdef __has_attribute
#   if __has_attribute(aligned)
#       define CUSTOM_ATTRIBUTE_ALIGNED(n) __attribute__ ((aligned (n)))
#   else
#       define CUSTOM_ATTRIBUTE_ALIGNED(n)
#   endif
#endif

#endif // CUSTOM_EXTRA_HEADER_H_INCLUDED
