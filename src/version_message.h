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

#ifndef VERSION_MESSAGE_H_INCLUDED
#define VERSION_MESSAGE_H_INCLUDED

#ifdef __has_include
#   if __has_include("../gen_src/build_version_message.h")
#       include "../gen_src/build_version_message.h"
#   endif
#endif

#ifndef NDEBUG
#   ifdef VERBOSE_DEBUG
#       define ARRGEN_DEBUG_VERSION_MESSAGE "Debug build with verbose debug logging and assertions\n"
#   else
#       define ARRGEN_DEBUG_VERSION_MESSAGE "Built with debug settings and assertions\n"
#   endif
#else
#   define ARRGEN_DEBUG_VERSION_MESSAGE
#endif

#ifndef ARRGEN_VERSION_MESSAGE_GIT
#   define ARRGEN_VERSION_MESSAGE_GIT "Compiled without git commit info, not yet implemented for this platform?\n"
#endif

#define ARRGEN_VERSION_MESSAGE ARRGEN_VERSION_MESSAGE_GIT ARRGEN_DEBUG_VERSION_MESSAGE

#endif // VERSION_MESSAGE_H_INCLUDED
