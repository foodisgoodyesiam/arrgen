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
#include <inttypes.h>
#include <errno.h>
#include <inttypes.h>
#include <malloc.h>
#ifdef ARRGEN_MMAP_SUPPORTED
#   include <sys/mman.h>
#   include <sys/stat.h>
#   include <unistd.h>
#   include <fcntl.h>
#endif
#include "handlefile.h"
#include "errors.h"
#include "writearray.h"
#include "c_string_stuff.h"

static bool writeH(const OutputFileParams* params, const size_t lengths[])
    ATTR_ACCESS(read_only, 1)
    ATTR_ACCESS(read_only, 2)
    ATTR_NONNULL;

static bool writeC(const OutputFileParams* params, size_t lengths[])
    ATTR_ACCESS(read_only, 1)
    ATTR_ACCESS(write_only, 2)
    ATTR_NONNULL;

static ssize_t writeArrayStreamed(FILE* out, FILE* in, const char* in_path, size_t line_limit)
    ATTR_NONNULL;

static ssize_t writeFileContents(FILE* out, const InputFileParams *input)
    ATTR_ACCESS(read_only, 2)
    ATTR_NONNULL;

bool handleFile(const OutputFileParams* params) {
    size_t lengths[params->num_inputs];
    return writeC(params, lengths) && (!params->create_header || writeH(params, lengths));
}

static bool writeH(const OutputFileParams* params, const size_t lengths[]) {
    DLOG("entering function");
    // this is a clunky way of handling it, but whatever
    char h_path[strlen(params->c_path) + strlen(params->h_name)+2]; // +2 for / and null terminator, in the worst case...? worse than the worst case
    strcpy(h_path, params->c_path);
    char* spot_to_insert_header_name = strchr(h_path, '/');
    if (spot_to_insert_header_name==NULL)
        spot_to_insert_header_name = h_path;
    else
        spot_to_insert_header_name++;
    strcpy(spot_to_insert_header_name, params->h_name);
    DLOG("c_path: %s, h_name: %s, resulting h_path: %s", params->c_path, params->h_name, h_path);
    FILE *out = fopen(h_path, "wb");
    bool ret;
    if (UNLIKELY(out==NULL)) {
        myErrorErrno("%s: could not open", h_path);
        ret = false;
    } else {
        // TODO: fail gracefully if any fprintf fails
        const char *include_guard = createCName(h_path, strlen(h_path), "_INCLUDED");
        int res;
        res = fprintf(out,
            "#ifndef %s\n"
            "#define %s\n"
            "#ifdef __cplusplus\n"
            "#extern \"C\" {\n"
            "#endif // __cplusplus\n"
            "\n",
            include_guard,
            include_guard);
        for (size_t i=0; i<params->num_inputs; i++) {
            fprintf(out,
                "#define %s %" PRIu64 "U\n",
                params->inputs[i].length_name,
                (uint64_t)lengths[i]);
        }
        fprintf(out, "\n");
        for (size_t i=0; i<params->num_inputs; i++) {
            // TODO hmm, what do I do if the input file name contains a newline
            // TODO use the line pragma for attributes etc...? maybe unnecessary
            fprintf(out,
                "// %s\n"
                "extern%s unsigned char %s[%s]",
                params->inputs[i].path,
                (LIKELY(params->inputs[i].make_const) ? " const" : ""),
                params->inputs[i].array_name,
                params->inputs[i].length_name);
            if (params->inputs[i].attributes==NULL)
                fprintf(out, ";\n");
            else {
                fprintf(out,
                    " %s;\n",
                    params->inputs[i].attributes);
            }
        }
        fprintf(out,
            "\n"
            "#ifdef __cplusplus\n"
            "}\n"
            "#endif // __cplusplus\n"
            "#endif // %s\n",
            include_guard);
        if (UNLIKELY(ferror(out))) {
            // TODO
        }
        if (UNLIKELY(fclose(out)!=0))
            myErrorErrno("%s: could not close", h_path);
        ret = true;
        free((void*)include_guard); // totally unnecessary but why not
    }
    DLOG("returning %hhu", ret);
    return (ret);
}

static bool writeC(const OutputFileParams* params, size_t lengths[]) {
    DLOG("entering function");
    FILE *out = fopen(params->c_path, "wb"); // CLRF is icky
    bool ret;
    if (UNLIKELY(out==NULL)) {
        myErrorErrno("%s: could not open", params->c_path);
        ret = false;
    } else {
        fprintf(out,
            "#include \"%s\"\n",
            params->h_name);
        for (size_t i=0; i<params->num_inputs; i++) {
            const InputFileParams *input = &params->inputs[i];
            fprintf(out,
                "const unsigned char %s[%s] = {",
                input->array_name,
                input->length_name);
            ssize_t length = writeFileContents(out, input);
            ret = LIKELY(length>=0);
            if (!ret)
                break;
            lengths[i] = length;
            fprintf(out, "};\n");
        }
        if (UNLIKELY(ferror(out))) {
            // TODO
        }
        if (UNLIKELY(fclose(out)!=0))
            myErrorErrno("%s: could not close", params->c_path);
    }
    DLOG("returning %hhu", ret);
    return (ret);
}

static ssize_t writeFileContents(FILE* out, const InputFileParams *input) {
    DLOG("entering function");
    ssize_t length;
    initializeLookup(input->base, input->aligned);
    // following a no-early-return policy here because of the various unwinding necessary
#ifdef ARRGEN_MMAP_SUPPORTED
    int fd = open(input->path, O_RDONLY);
    if (UNLIKELY(fd<0)) {
        myErrorErrno("%s: could not open", input->path);
        length = -1;
    } else {
        struct stat stats;
        if (UNLIKELY(fstat(fd, &stats)!=0)) {
            myErrorErrno("%s: could not fstat fd %d", input->path, fd);
            length = -1;
        } else {
            switch (stats.st_mode & S_IFMT) {
            case S_IFREG: {
                length = stats.st_size;
                // TODO: consider if it should fall back to streaming on mmap failure
                const uint8_t* mem = (const uint8_t*) mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
                if (UNLIKELY(close(fd)!=0))
                    myErrorErrno("%s: could not close fd %d", input->path, fd);
                if (UNLIKELY(mem==MAP_FAILED)) {
                    myErrorErrno("%s: mmap", input->path);
                    length = -1;
                } else {
                    if (length > arrgen_pagesize_) {
                        // why the frick does madvise not qualify the argument with const...
                        if (UNLIKELY(madvise((void*)mem, length, MADV_SEQUENTIAL)))
                            myErrorErrno("%s: could not madvise for %zd bytes at %p", input->path, length, mem);
                    }
                    ssize_t cur_line_pos = -1;
                    writeArrayContents(out, mem, length, &cur_line_pos, input->line_length);
                    if (UNLIKELY(munmap((void*)mem, length))!=0)
                        myErrorErrno("%s: munmap", input->path);
                }
                }; break;
            case S_IFBLK:
                myError("%s: what the heck are you doing?", input->path);
                length = -1;
                break;
            default: {
                FILE* in = fdopen(fd, "rb");
                if (UNLIKELY(in==NULL)) {
                    myErrorErrno("%s: could not fdopen %d", input->path, fd);
                    length = -1;
                    if (UNLIKELY(close(fd)!=0))
                        myErrorErrno("%s: could not close fd %d", input->path, fd);
                } else {
                    length = writeArrayStreamed(out, in, input->path, input->line_length);
                    if (UNLIKELY(fclose(in)!=0))
                        myErrorErrno("%s: could not fclose", input->path);
                }
                } break;
            }
        }
    }
#else // MMAP_SUPPORTED
    FILE* in = fopen(input->path, "rb");
    if (UNLIKELY(in==NULL)) {
        myErrorErrno("%s: could not fopen", input->path);
        length = -1;
    } else {
        length = writeArrayStreamed(out, in, input->path);
        if (UNLIKELY(fclose(in)!=0))
            myErrorErrno("%s: could not fclose", input->path);
    }
#endif // ARRGEN_MMAP_SUPPORTED
    DLOG("returning %zd", length);
    return (length);
}

static ssize_t writeArrayStreamed(FILE* out, FILE* in, const char* in_path, size_t line_limit) {
    DLOG("entering function: %p, %p, %s", out, in, in_path);
    size_t num_read = ARRGEN_BUFFER_SIZE, total_length;
    static uint8_t buf[ARRGEN_BUFFER_SIZE];
    int error = 0;
    ssize_t cur_line_pos = -1;
    for (total_length=0U; num_read==ARRGEN_BUFFER_SIZE; total_length+=num_read) {
        num_read = fread(buf, 1, ARRGEN_BUFFER_SIZE, in);
        if (UNLIKELY(num_read != ARRGEN_BUFFER_SIZE)) {
            error = errno;
            if (!LIKELY(feof(in)))
                myError("%s: read: %s", in_path, strerror(error));
        }
        DLOG("%s: num_read = %zu\ttotal_length=%zu", in_path, num_read, total_length);
        writeArrayContents(out, buf, num_read, &cur_line_pos, line_limit);
    }
    return (error==0 ? total_length : -1);
}


