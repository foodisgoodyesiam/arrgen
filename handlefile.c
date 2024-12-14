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

#define _GNU_SOURCE
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include "arrgen.h"
#ifdef ARRGEN_MMAP_SUPPORTED
#   include <sys/mman.h>
#   include <sys/stat.h>
#   include <unistd.h>
#   include <fcntl.h>
#endif
#include "handlefile.h"
#include "errors.h"
#include "writearray.h"

#if !defined(__GLIBC__) && !defined(__CYGWIN__)
static const char* basename(const char* path)
    ATTR_ACCESS(read_only, 1)
    ATTR_NONNULL;
#   define USE_CUSTOM_BASENAME
#endif

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

static ssize_t writeFileContents(FILE* out, const OutputFileParams *output, size_t index)
    ATTR_ACCESS(read_only, 2)
    ATTR_NONNULL;

bool handleFile(const OutputFileParams* params) {
    size_t lengths[params->num_inputs];
    return writeC(params, lengths) && (!params->create_header || writeH(params, lengths));
}

static bool writeH(const OutputFileParams* params, const size_t lengths[]) {
    DLOG("entering function");
    FILE *out = fopen(params->h_path, "wb");
    bool ret;
    if (UNLIKELY(out==NULL)) {
        myErrorErrno("%s: could not open", params->h_path);
        ret = false;
    } else {
        // TODO: fail gracefully if any fprintf fails
        const char* include_guard = "TODO_BLOOP_INCLUDED";
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
            fprintf(out,
                "// %s\n"
                "extern const unsigned char %s[%s];\n",
                params->inputs[i].path,
                params->inputs[i].array_name,
                params->inputs[i].length_name);
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
            myErrorErrno("%s: could not close", params->h_path);
        ret = true;
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
        // TODO make it figure out the correct include path? or just force the header and c file to be in the same directory
        fprintf(out,
            "#include \"%s\"\n",
            basename(params->h_path));
        for (size_t i=0; i<params->num_inputs; i++) {
            const InputFileParams input = params->inputs[i];
            fprintf(out,
                "const unsigned char %s[%s] = {",
                input.array_name,
                input.length_name);
            ssize_t length = writeFileContents(out, params, i);
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

static ssize_t writeFileContents(FILE* out, const OutputFileParams *output, size_t index) {
    DLOG("entering function");
    const InputFileParams *input = &output->inputs[index];
    ssize_t length;
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
                    writeArrayContents(out, mem, length, &cur_line_pos, output->line_length);
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
                    length = writeArrayStreamed(out, in, input->path, output->line_length);
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

#ifdef USE_CUSTOM_BASENAME
static const char* basename(const char* path) {
    const char* ret = strrchr(path, '/');
    if (ret==nullptr)
        return path;
    ret++;
    return ret;
}
#endif // USE_CUSTOM_BASENAME

