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

#include "option.h"

static FILE *fp_src[3] = { NULL, NULL, NULL };

#define fourcc_mod_vs_is_pvric(val) \
	!!(fourcc_mod_vs_get_type(val) == DRM_FORMAT_MOD_VS_TYPE_PVRIC ? 1 : 0)

char *vs_get_layer_source_name(char *filename)
{
	uint8_t size = 0, p = 0;
	uint8_t k;
	char *str = "";
	char *dest = "";
	for (k = 0; k < strlen(filename); k++) {
		if (filename[k] == '.') {
			break;
		}
		p++;
	}
	size = p + 1;
	str = malloc(size);
	memset(str, 0, size);
	memcpy(str, filename, p);

	dest = malloc(size - 9);
	memset(dest, 0, size - 9);

	for (k = 0; k < size - 9; k++) {
		dest[k] = str[k + 9];
	}
	return dest;
}

void vs_sprint(char *str, char *message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	vsprintf(str, message, arguments);
	va_end(arguments);
}

vs_status vs_dump_layer_source(struct bo *plane_bo[4], uint64_t physical[3], uint32_t **va_addr,
			       uint8_t num_planes, drm_vs_bo_param bo_param[4], uint64_t mod,
			       uint32_t format, char *filenme, bool final_plane)
{
	vs_status ret = VS_STATUS_OK;
	uint64_t source = 0;
	uint32_t *val = NULL;
	uint32_t i = 0, j = 0, size = 0;
	static uint32_t source_index = 0;
	char *source_name = "";

	/*for pvric type*/
	bool lossy = 0;
	uint64_t source_addr[3] = { 0 };
	uint32_t *val_addr[3] = { 0 };
	uint32_t tile_cnt = 0;
	uint32_t src_size[4] = { 0 };

	if (!va_addr || !physical[0] || !num_planes || num_planes > 3) {
		printf("%s: invalid input param.\n", __func__);
		return VS_STATUS_FAILED;
	}

	source_name = vs_get_layer_source_name(filenme);

	for (i = 0; i < num_planes; i++) {
		char path_source[256] = "";
		vs_sprint(path_source, "%slayersource_%d_%s_planar%d.txt", LAYER_SOURCE_PATH,
			  source_index, source_name, i);
		if ((fp_src[i] = fopen(path_source, "w")) == NULL) {
			printf("Layer source capture file can't be open.\n");
			ret = VS_STATUS_FAILED;
			goto exit;
		}
	}

	if (fourcc_mod_vs_is_pvric(mod)) {
		if (mod & DRM_FORMAT_MOD_VS_DEC_LOSSY)
			lossy = true;
		else
			lossy = false;
		for (i = 0; i < num_planes; i++) {
			tile_cnt = bo_param[i].header_size;
			if (!lossy) {
				src_size[i] = bo_param[i].header_size + tile_cnt * 256;
			} else if (format == DRM_FORMAT_P010) {
				src_size[i] = bo_param[i].header_size + tile_cnt * 96;
			} else
				src_size[i] = bo_param[i].header_size + tile_cnt * 128;
			val_addr[i] = (uint32_t *)((uint64_t)va_addr[i] + plane_bo[i]->offset -
						   bo_param[i].header_size);
			source_addr[i] =
				(physical[i] + plane_bo[i]->offset - bo_param[i].header_size);
		}
	}

	for (i = 0; i < num_planes; i++) {
		if (fourcc_mod_vs_is_pvric(mod)) {
			source = source_addr[i];
			val = val_addr[i];
			size = src_size[i];
		} else {
			source = physical[i];
			val = va_addr[i];
			size = plane_bo[i]->pitch * plane_bo[i]->height;
		}

		fprintf(fp_src[i], "MEM_INIT layersource_%d_%s_planar%d.txt ADDR:'h", source_index,
			source_name, i);
		fprintf(fp_src[i], "%08x%08x", (uint32_t)(source >> 32), (uint32_t)(source));
		fprintf(fp_src[i], "\n");

		for (j = 0; j < size / 4; j++) {
			fprintf(fp_src[i], "%08x", (*val) & 0xFFFFFFFF);
			fprintf(fp_src[i], "\n");
			val += 1;
		}
		if (size % 4 != 0)
			fprintf(fp_src[i], "\n");
	}

	if (final_plane) {
		for (i = 0; i < num_planes; i++) {
			fclose(fp_src[i]);
			fp_src[i] = NULL;
		}
		source_index++;
	}
	return ret;

exit:
	for (i = 0; i < num_planes; i++) {
		if (fp_src[i])
			fclose(fp_src[i]);
		fp_src[i] = NULL;
	}

	return ret;
}
