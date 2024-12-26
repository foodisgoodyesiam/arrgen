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
#include <stdlib.h>
#ifndef ARRGEN_MMAP_SUPPORTED
#   pragma error "ARRGEN_MMAP_SUPPORTED not defined, something's wrong with arrgen.h"
#elif (ARRGEN_MMAP_SUPPORTED == ARRGEN_MMAP_TYPE_POSIX)
#   include <sys/mman.h>
#   include <sys/stat.h>
#   include <unistd.h>
#   include <fcntl.h>
#elif (ARRGEN_MMAP_SUPPORTED == ARRGEN_MMAP_TYPE_WINDOWS)
#   include <windows.h>
#   include <fileapi.h>
#   include <winbase.h>
#   include <memoryapi.h>
#elif (ARRGEN_MMAP_SUPPORTED != ARRGEN_MMAP_TYPE_NONE)
#   pragma error "ARRGEN_MMAP_SUPPORTED has unknown value, something's wrong with arrgen.h"
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
    const char *h_path = pathRelativeToFile(params->c_path, params->h_name);
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
            "%s"
            "#ifndef %s\n"
            "#define %s\n"
            "%s"
            "#ifdef __cplusplus\n"
            "extern \"C\" {\n"
            "#endif // __cplusplus\n"
            "\n",
            (params->constexpr_length ? "#include <stddef.h>\n" : ""),
            include_guard,
            include_guard,
            (params->header_top_text==NULL ? "" : params->header_top_text));
        for (size_t i=0; i<params->num_inputs; i++) {
            fprintf(out,
                (params->constexpr_length ? "constexpr size_t %s = %" PRIu64 "U;\n" : "#define %s %" PRIu64 "U\n"),
                params->inputs[i].length_name,
                (uint64_t)lengths[i]);
        }
        for (size_t i=0; i<params->num_inputs; i++) {
            // TODO hmm, what do I do if the input file name contains a newline
            // TODO use the line pragma for attributes etc...? maybe unnecessary
            fprintf(out,
                "\n"
                "// %s\n"
                "%s"
                "extern%s unsigned char %s[%s];\n",
                params->inputs[i].path_original,
                (params->inputs[i].attributes==NULL ? "" : params->inputs[i].attributes),
                (LIKELY(params->inputs[i].make_const) ? " const" : ""),
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
            myErrorErrno("%s: could not close", h_path);
        ret = true;
        free((void*)include_guard); // totally unnecessary but why not
    }
    free((void*)h_path);
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
                "%sunsigned char %s[%s] = {",
                (input->make_const ? "const " : ""),
                input->array_name,
                input->length_name);
            ssize_t length = writeFileContents(out, input);
            ret = LIKELY(length>=0);
            if (!ret)
                break;
            lengths[i] = (size_t)length;
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
#if (ARRGEN_MMAP_SUPPORTED == ARRGEN_MMAP_TYPE_POSIX)
    int fd = open(input->path_to_open, O_RDONLY);
    if (UNLIKELY(fd<0)) {
        myErrorErrno("%s: could not open", input->path_to_open);
        length = -1;
    } else {
        struct stat stats;
        if (UNLIKELY(fstat(fd, &stats)!=0)) {
            myErrorErrno("%s: could not fstat fd %d", input->path_to_open, fd);
            length = -1;
        } else {
            switch (stats.st_mode & S_IFMT) {
            case S_IFREG: {
                length = stats.st_size;
                // TODO: consider if it should fall back to streaming on mmap failure
                const uint8_t* mem = (const uint8_t*) mmap(NULL, (size_t)length, PROT_READ, MAP_SHARED, fd, 0);
                if (UNLIKELY(close(fd)!=0))
                    myErrorErrno("%s: could not close fd %d", input->path_to_open, fd);
                if (UNLIKELY(mem==MAP_FAILED)) {
                    myErrorErrno("%s: mmap", input->path_to_open);
                    length = -1;
                } else {
                    if (length > arrgen_pagesize_) {
                        // why the frick does madvise not qualify the argument with const...
                        if (UNLIKELY(madvise((void*)mem, (size_t)length, MADV_SEQUENTIAL)))
                            myErrorErrno("%s: could not madvise for %zd bytes at %p", input->path_to_open, length, mem);
                    }
                    ssize_t cur_line_pos = -1;
                    writeArrayContents(out, mem, (size_t)length, &cur_line_pos, input->line_length);
                    if (UNLIKELY(munmap((void*)mem, (size_t)length))!=0)
                        myErrorErrno("%s: munmap", input->path_to_open);
                }
                }; break;
            case S_IFBLK:
                myError("%s: what the heck are you doing?", input->path_to_open);
                length = -1;
                break;
            default: {
                FILE* in = fdopen(fd, "rb");
                if (UNLIKELY(in==NULL)) {
                    myErrorErrno("%s: could not fdopen %d", input->path_to_open, fd);
                    length = -1;
                    if (UNLIKELY(close(fd)!=0))
                        myErrorErrno("%s: could not close fd %d", input->path_to_open, fd);
                } else {
                    length = writeArrayStreamed(out, in, input->path_to_open, input->line_length);
                    if (UNLIKELY(fclose(in)!=0))
                        myErrorErrno("%s: could not fclose", input->path_to_open);
                }
                } break;
            }
        }
    }
#elif (ARRGEN_MMAP_SUPPORTED == ARRGEN_MMAP_TYPE_WINDOWS)
    // this is a lot of overhead...
    // TODO switch to the W version (ANSI = 8-bit characters, W=UTF-16)
    // the 260-character limit can be avoided using \\?\ but that turns off expansion of . and .., figure out way around that...
    // TODO investigate whether OpenFileMappingA can be used instead of the double-handle, would it simplify? would it have the same effect/level of control?
    // TODO fall back on regular file I/O if the map fails? investigate when/whether this can happen in Windows, are there an equivalent of named pipes?
    // TODO try this with not-locally-downloaded dropbox files or the like
    HANDLE handle = CreateFileA(
        input->path_to_open,
        GENERIC_READ, // I only want to read the file
        FILE_SHARE_READ, // don't let other people do anything to the file while I have it open, except reading it
        NULL, // doesn't matter because I won't be creating any child processes
        OPEN_EXISTING, // don't create it if it doesn't exist
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, // don't open it unbuffered, don't change it to hidden, and a bunch of other weird misc stuff. But hint to Windows that I'll read the file sequentially, sorta like madvise. (Can't figure out if it matters for memory-mapped I/O.)
        NULL // parameters to give the newly created file, doesn't matter because I'm not creating a file
        );
    if (LIKELY(handle!=INVALID_HANDLE_VALUE)) {
        // TODO decide whether to map with exclusive access or not. I think I don't care because writes/deletes should be prevented by the FILE_SHARE_READ.
        HANDLE mapping_handle = CreateFileMappingA(
            handle,
            NULL, // doesn't matter because I won't be creating any child processes
            PAGE_READONLY,
            0, // figure out the file size from the handle I gave you
            0, // figure out the file size from the handle I gave you
            NULL // why would I want to name this handle?
        );
        if (LIKELY(mapping_handle!=INVALID_HANDLE_VALUE)) {
            const uint8_t *mem = (const uint8_t*)MapViewOfFile(
                mapping_handle,
                FILE_MAP_READ,
                0, // start at the beginning of the file
                0, // start at the beginning of the file (don't get the difference between these two)
                0  // map the entire file
            );
            if (LIKELY(mem!=NULL)) {
                // TODO: investigate, can I find the size of the file from the handle? instead of from the view? (should I?)
                MEMORY_BASIC_INFORMATION info;
                size_t info_buffer_bytes_written = VirtualQuery(mem, &info, sizeof(MEMORY_BASIC_INFORMATION));
                if (UNLIKELY(info_buffer_bytes_written==0))
                    myFatalWindowsError("%s: VirtualQuery", input->path_to_open);
                length = info.RegionSize;
                ssize_t cur_line_pos = -1;
                writeArrayContents(out, mem, (size_t)length, &cur_line_pos, input->line_length);
            } else
                myFatalWindowsError("%s: MapViewOfFile", input->path_to_open);
            if (UNLIKELY(!UnmapViewOfFile(mem)))
                myErrorWindowsError("%s: UnmapViewOfFile", input->path_to_open);
        } else
            myFatalWindowsError("%s: CreateFileMappingA", input->path_to_open);
        if (UNLIKELY(!CloseHandle(mapping_handle)))
            myErrorWindowsError("%s: CloseHandle (memory-mapping handle)", input->path_to_open);
        if (UNLIKELY(!CloseHandle(handle)))
            myErrorWindowsError("%s: CloseHandle", input->path_to_open);
    } else
        myFatalWindowsError("%s: CreateFileA", input->path_to_open);
#else // ARRGEN_MMAP_TYPE_NONE
    FILE* in = fopen(input->path_to_open, "rb");
    if (UNLIKELY(in==NULL)) {
        myErrorErrno("%s: could not fopen", input->path_to_open);
        length = -1;
    } else {
        length = writeArrayStreamed(out, in, input->path_to_open, input->line_length);
        if (UNLIKELY(fclose(in)!=0))
            myErrorErrno("%s: could not fclose", input->path_to_open);
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
    return (error==0 ? (ssize_t)total_length : -1);
}


