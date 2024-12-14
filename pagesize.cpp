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

static unsigned constructPagesize();
unsigned arrgen_pagesize_ = constructPagesize();

#if defined(_WIN64) || defined(_WIN32)
#   define WINDOWS_PAGESIZE
#elif defined(__has_include)
#   if __has_include(<unistd.h>)
#       define UNIX_PAGESIZE
#   endif
#elif defined(__linux__) || defined(__APPLE__) || defined(__CYGWIN__)
#   define UNIX_PAGESIZE
#endif // defined

#ifdef WINDOWS_PAGESIZE
#   include <sysinfoapi.h>
static unsigned constructPagesize() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwPageSize;
}

#elif defined(UNIX_PAGESIZE)
#   include <unistd.h>
static unsigned constructPagesize() {
    //return sysconf(_SC_PAGESIZE);
    return getpagesize();
}

#else
// dummy placeholder. the purpose of pagesize is just to know whether to madvise, so if I don't know the OS, this won't be used anyways
static unsigned constructPagesize() {
    return 65536U;
}

#endif




