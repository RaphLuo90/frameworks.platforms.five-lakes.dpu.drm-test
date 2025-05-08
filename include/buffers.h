/***************************************************************************
*    Copyright 2012 - 2023 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/

/*
 * DRM based mode setting test program
 * Copyright 2008 Tungsten Graphics
 *   Jakob Bornecrantz <jakob@tungstengraphics.com>
 * Copyright 2008 Intel Corporation
 *   Jesse Barnes <jesse.barnes@intel.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef __BUFFERS_H__
#define __BUFFERS_H__

#include "util/pattern.h"
#include "drmtest.h"

#if CONFIG_VERISILICON_MD5_CHECK
#define MD5_DIGEST_LEN 16
#define MD5_BUFFER_SIZE 2 * MD5_DIGEST_LEN + 1
#endif

struct bo;

struct bo {
	int fd;
	void *ptr;
	size_t size;
	uint64_t offset;
	size_t pitch;
	unsigned handle;

	/* VSI: for wb connector */
	uint32_t width;
	uint32_t height;
};

struct bo *bo_create(int fd, unsigned int format, unsigned int width, unsigned int height,
		     unsigned int handles[4], unsigned int pitches[4], unsigned int offsets[4],
		     enum util_fill_pattern pattern, uint64_t mod);
void bo_destroy(struct bo *bo);

dtest_status bo_dump(pipe_t *pipe, const char *filename, FILE *stream_fp, uint32_t pos_frame,
		     bool save_bin, bool check);

dtest_status dtest_dump_stream_raw_data(void *va_addr[4], pipe_t *pipe, const char *filename,
					FILE *stream_fp, uint32_t pos_frame);

void dtest_extract_filename(const char *filename, char *result);

dtest_status dtest_dump_raw_data(void *va_addr[4], pipe_t *pipe, const char *filename);

uint32_t __get_bpp(uint32_t format, uint64_t mod);

struct bo *bo_create_dumb(int fd, unsigned int width, unsigned int height, unsigned int bpp);

int bo_map(struct bo *bo, void **out);

#if CONFIG_VERISILICON_MD5_CHECK
void vs_fill_sprint(char *str, char *message, ...);

vs_status vs_calcu_md5(const char *filename, uint8_t md5_hash[MD5_DIGEST_LEN]);

dtest_status vs_check_md5(const char *md5_result, const char *golden, char *wb_id);
#endif

void bo_unmap(struct bo *bo);

/* vs bo create */
int vs_bo_create(int fd, unsigned int format, uint64_t mod, unsigned int width, unsigned int height,
		 char (*filename)[RESOURCE_LEN + 1], enum util_fill_pattern pattern,
		 uint64_t modifiers[4], struct bo *pbo[4], drm_vs_bo_param bo_params[4],
		 uint32_t frame_idx);

/* vdp bo create and config */
int vs_vdp_config(struct device *dev, struct pipe_arg *pipes, unsigned int count);

int vs_mask_bo_create(int fd, unsigned int format, uint64_t mod, unsigned int width,
		      unsigned int height, const char *filename, enum util_fill_pattern pattern,
		      struct bo *pbo[4], bool compressed);

#endif
