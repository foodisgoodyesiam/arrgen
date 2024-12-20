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
#include "writearray.h"
#include "errors.h"

#ifndef ARRGEN_NUM_REPEATS
#   define ARRGEN_NUM_REPEATS 10U
#endif // ARRGEN_NUM_REPEATS

typedef struct {
    uint16_t offset;
    uint8_t len;
} ByteParams;

static uint8_t base_;
static bool aligned_;
static ByteParams params_[UINT8_MAX];
static char string_bank_[5U*256U*ARRGEN_NUM_REPEATS+1U] ATTR_NONSTRING; // TODO make this number less magic

void initializeLookup(uint8_t base, bool aligned) {
    if (base==base_ && aligned==aligned_)
        return;
    const char* format;
    switch(base) {
    case 8:
        format = aligned ? "0%.3o," : "0%o,";
        break;
    case 10:
        format = aligned ? "%3u," : "%u,";
        break;
    case 16:
        format = aligned ? "0x%.2X," : "0x%X,";
        break;
    default:
        myFatal("unsupported base %u", (unsigned)base);
    }
    base_ = base;
    aligned_ = aligned;
    unsigned cur_pos = 0;
    for (unsigned c=0U; c<UINT8_MAX; c++) {
        params_[c].offset = cur_pos;
        int written_len;
        for (unsigned i = 0U; i<ARRGEN_NUM_REPEATS; i++) {
            written_len = sprintf(&string_bank_[cur_pos], format, c);
            cur_pos += written_len;
        }
        params_[c].len = written_len;
    }
}

// TODO: make it return error information instead of quitting? or add some cleanup functionality to errors.c using global variables... probably I'll do that
void writeArrayContents(FILE* out, const uint8_t *buf, size_t length, ssize_t *cur_line_pos, size_t line_limit) {
    size_t i=0;
    // TODO figure out if I want, or care, to remove the trailing comma with the lookup table implementation
    uint8_t num_to_print;
    if (UNLIKELY(*cur_line_pos < 0)) {
        *cur_line_pos = 0;
        fprintf(out, "\n    ");
    }
    if (line_limit == 0) { // no line limit
        for (; i<length; i+=num_to_print) {
            uint8_t max_num_to_print = LIKELY(ARRGEN_NUM_REPEATS < (length-i)) ? ARRGEN_NUM_REPEATS : length-i;
            for (num_to_print = 1U; num_to_print < max_num_to_print && buf[i+num_to_print]==buf[i]; num_to_print++);
            int cur_printed = fwrite(&string_bank_[params_[buf[i]].offset], params_[buf[i]].len, num_to_print, out);
            if (UNLIKELY(cur_printed != num_to_print))
                myFatalErrno("fwrite");
            *cur_line_pos += num_to_print;
        }
    } else {
        for (; i<length; i+=num_to_print) {
            if (UNLIKELY(*cur_line_pos >= line_limit)) {
                fprintf(out, "\n    ");
                *cur_line_pos = 0;
            }
            uint8_t max_num_to_print = LIKELY(ARRGEN_NUM_REPEATS < (length-i)) ? ARRGEN_NUM_REPEATS : length-i;
            if (UNLIKELY(line_limit-*cur_line_pos < max_num_to_print))
                max_num_to_print = line_limit-*cur_line_pos;
            for (num_to_print = 1U; num_to_print < max_num_to_print && buf[i+num_to_print]==buf[i]; num_to_print++);
            int cur_printed = fwrite(&string_bank_[params_[buf[i]].offset], params_[buf[i]].len, num_to_print, out);
            if (UNLIKELY(cur_printed != num_to_print))
                myFatalErrno("fwrite");
            *cur_line_pos += num_to_print;
        }
    }
}
