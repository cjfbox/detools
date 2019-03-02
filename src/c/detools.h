/**
 * BSD 2-Clause License
 *
 * Copyright (c) 2019, Erik Moqvist
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DETOOLS_H
#define DETOOLS_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <lzma.h>

/**
 * Read callback.
 *
 * @param[in] arg_p User data passed to detools_apply_patch_init().
 * @param[out] buf_p Buffer to read into.
 * @param[in] size Number of bytes to read.
 *
 * @return zero(0) or negative error code.
 */
typedef int (*detools_read_t)(void *arg_p, uint8_t *buf_p, size_t size);

/**
 * Write callback.
 *
 * @param[in] arg_p User data passed to detools_apply_patch_init().
 * @param[in] buf_p Buffer to write.
 * @param[in] size Number of bytes to write.
 *
 * @return zero(0) or negative error code.
 */
typedef int (*detools_write_t)(void *arg_p, const uint8_t *buf_p, size_t size);

/**
 * Seek from current position callback.
 *
 * @param[in] arg_p User data passed to detools_apply_patch_init().
 * @param[in] offset Offset to seek to from current position.
 *
 * @return zero(0) or negative error code.
 */
typedef int (*detools_seek_t)(void *arg_p, int offset);

struct detools_apply_patch_patch_reader_none_t {
    struct {
        const uint8_t *buf_p;
        size_t size;
        size_t offset;
    } chunk;
};

struct detools_apply_patch_patch_reader_lzma_t {
    lzma_stream stream;
    uint8_t *next_in_p;
    uint8_t *next_out_p;
};

struct detools_apply_patch_patch_reader_t {
    struct {
        const uint8_t *buf_p;
        size_t size;
        size_t offset;
    } chunk;
    union {
        struct detools_apply_patch_patch_reader_none_t none;
        struct detools_apply_patch_patch_reader_lzma_t lzma;
    } compression;
    int (*decompress)(struct detools_apply_patch_patch_reader_t *self_p,
                      uint8_t *buf_p,
                      size_t size);
};

/**
 * The apply patch data structure.
 */
struct detools_apply_patch_t {
    detools_read_t from_read;
    detools_seek_t from_seek;
    detools_write_t to_write;
    void *arg_p;
    int patch_type;
    int compression;
    int to_pos;
    int to_size;
    int state;
    int chunk_pos;
    int chunk_size;
    struct detools_apply_patch_patch_reader_t patch_reader;
};

/**
 * Apply given patch with file names.
 *
 * @param[in] from_p Source file name.
 * @param[in] patch_p Patch file name.
 * @param[in] to_p Destination file name.
 *
 * @return zero(0) or negative error code.
 */
int detools_apply_patch_filenames(const char *from_p,
                                  const char *patch_p,
                                  const char *to_p);

/**
 * Apply given patch with callbacks.
 *
 * @param[in] from_read Source callback.
 * @param[in] patch_read Patch callback.
 * @param[in] to_write Destination callback.
 * @param[in] arg_p Argument passed to callbacks.
 *
 * @return zero(0) or negative error code.
 */
int detools_apply_patch_callbacks(detools_read_t from_read,
                                  detools_read_t patch_read,
                                  detools_write_t to_write,
                                  void *arg_p);

/**
 * Initialize given apply patch object.
 *
 * @param[out] self_p Patcher object to initialize.
 * @param[in] from_read Callback to read from-data.
 * @param[in] from_seek Callback to seek from current position in from-data.
 * @param[in] to_write Destination callback.
 * @param[in] arg_p Argument passed to the callbacks.
 *
 * @return zero(0) or negative error code.
 */
int detools_apply_patch_init(struct detools_apply_patch_t *self_p,
                             detools_read_t from_read,
                             detools_seek_t from_seek,
                             detools_write_t to_write,
                             void *arg_p);

/**
 * Feed data into given patcher and at the same time generate patched
 * output, ready to be written to disk/flash.
 *
 * NOTE: The minimum patch chunk size needed to produce any to-bytes
 *       depends on the patch compression algorithm and settings.
 *
 * @param[in,out] self_p Patcher object.
 * @param[in] patch_p Next chunk of the patch.
 * @param[in,out] size Patch buffer size.
 *
 * @return Number of consumed patch bytes, or negative error code.
 */
int detools_apply_patch_process(struct detools_apply_patch_t *self_p,
                                const uint8_t *patch_p,
                                size_t size);

/**
 * Call once after all data has been processed to finalize the
 * patching.
 *
 * @param[in,out] self_p Patcher object.
 *
 * @return Zero(0) if the patch was applied successfully, or negative
 *         error code.
 */
int detools_apply_patch_finalize(struct detools_apply_patch_t *self_p);

#endif