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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <drm_fourcc.h>
#include "drm/vs_drm_fourcc.h"

#include "util/common.h"
#include "util/format.h"

#define _fourcc_mod_vs_get_type(val) (((val)&DRM_FORMAT_MOD_VS_TYPE_MASK) >> 53)

#define MAKE_RGB_INFO(rl, ro, gl, go, bl, bo, al, ao) \
	.rgb = { { (rl), (ro) }, { (gl), (go) }, { (bl), (bo) }, { (al), (ao) } }

#define MAKE_YUV_INFO(order, xsub, ysub, chroma_stride) \
	.yuv = { (order), (xsub), (ysub), (chroma_stride) }

static const struct util_format_info format_info_custom[] = {
	/* RGB888-planar */
	{ DRM_FORMAT_RGB888, "RG24", 3, MAKE_RGB_INFO(8, 0, 8, 0, 8, 0, 0, 0) },
	{ DRM_FORMAT_BGR888, "BG24", 3, MAKE_RGB_INFO(8, 0, 8, 0, 8, 0, 0, 0) },
	{ DRM_FORMAT_RGB565_A8, "AR16", 1, MAKE_RGB_INFO(5, 11, 6, 5, 5, 0, 8, 16) },
	{ DRM_FORMAT_BGR565_A8, "AB16", 1, MAKE_RGB_INFO(5, 0, 6, 5, 5, 11, 8, 16) },
	{ DRM_FORMAT_P010, "P010_VU", 2, MAKE_YUV_INFO(YUV_YCrCb, 2, 2, 2) },
	{ DRM_FORMAT_P210, "P210_VU", 2, MAKE_YUV_INFO(YUV_YCrCb, 2, 1, 2) },
	{ DRM_FORMAT_YUV420, "I010_UV", 3, MAKE_YUV_INFO(YUV_YCbCr, 2, 2, 1) },
	{ DRM_FORMAT_YVU420, "I010_VU", 3, MAKE_YUV_INFO(YUV_YCrCb, 2, 2, 1) },
	{ DRM_FORMAT_YUV420_10BIT, "P030_UV", 2, MAKE_YUV_INFO(YUV_YCbCr, 2, 2, 2) },
	{ DRM_FORMAT_P016, "P030_VU", 2, MAKE_YUV_INFO(YUV_YCrCb, 2, 2, 2) },
	/*Y Only*/
	{ DRM_FORMAT_Y0L0, "LUMA_10", 1 },
};

static const struct util_format_info format_info[] = {
	/* Indexed */
	{ DRM_FORMAT_C8, "C8", 1 },

	/*Y Only*/
	{ DRM_FORMAT_C8, "LUMA_8", 1 },
	{ DRM_FORMAT_Y0L0, "LUMA_10", 1 },

	/* YUV packed */
	{ DRM_FORMAT_UYVY, "UYVY", 1, MAKE_YUV_INFO(YUV_YCbCr | YUV_CY, 2, 2, 2) },
	{ DRM_FORMAT_VYUY, "VYUY", 1, MAKE_YUV_INFO(YUV_YCrCb | YUV_CY, 2, 2, 2) },
	{ DRM_FORMAT_YUYV, "YUYV", 1, MAKE_YUV_INFO(YUV_YCbCr | YUV_YC, 2, 2, 2) },
	{ DRM_FORMAT_YVYU, "YVYU", 1, MAKE_YUV_INFO(YUV_YCrCb | YUV_YC, 2, 2, 2) },
	{ DRM_FORMAT_YUV420_8BIT, "NV12_DEC", 1, MAKE_YUV_INFO(YUV_YCbCr, 2, 2, 1) },
	{ DRM_FORMAT_YUV420_10BIT, "P010_DEC", 1, MAKE_YUV_INFO(YUV_YCbCr, 2, 2, 1) },
	/* For DP YUV */
	{ DRM_FORMAT_Y210, "Y210", 1, MAKE_YUV_INFO(YUV_YCrCb | YUV_YC, 2, 2, 2) },
	/* YUV semi-planar */
	{ DRM_FORMAT_NV12, "NV12", 2, MAKE_YUV_INFO(YUV_YCbCr, 2, 2, 2) },
	{ DRM_FORMAT_NV21, "NV21", 2, MAKE_YUV_INFO(YUV_YCrCb, 2, 2, 2) },
	{ DRM_FORMAT_NV16, "NV16", 2, MAKE_YUV_INFO(YUV_YCbCr, 2, 1, 2) },
	{ DRM_FORMAT_NV61, "NV61", 2, MAKE_YUV_INFO(YUV_YCrCb, 2, 1, 2) },
	{ DRM_FORMAT_P010, "P010_UV", 2, MAKE_YUV_INFO(YUV_YCbCr, 2, 2, 2) },
	{ DRM_FORMAT_P010, "P010_VU", 2, MAKE_YUV_INFO(YUV_YCrCb, 2, 2, 2) },
	{ DRM_FORMAT_P210, "P210_UV", 2, MAKE_YUV_INFO(YUV_YCbCr, 2, 1, 2) },
	{ DRM_FORMAT_P210, "P210_VU", 2, MAKE_YUV_INFO(YUV_YCrCb, 2, 1, 2) },
	{ DRM_FORMAT_YUV420_10BIT, "P030_UV", 2, MAKE_YUV_INFO(YUV_YCbCr, 2, 2, 2) },
	{ DRM_FORMAT_P016, "P030_VU", 2, MAKE_YUV_INFO(YUV_YCrCb, 2, 2, 2) },
	/* YUV planar */
	{ DRM_FORMAT_YUV420, "YU12", 3, MAKE_YUV_INFO(YUV_YCbCr, 2, 2, 1) },
	{ DRM_FORMAT_YVU420, "YV12", 3, MAKE_YUV_INFO(YUV_YCrCb, 2, 2, 1) },
	{ DRM_FORMAT_YUV420, "I010_UV", 3, MAKE_YUV_INFO(YUV_YCbCr, 2, 2, 1) },
	{ DRM_FORMAT_YVU420, "I010_VU", 3, MAKE_YUV_INFO(YUV_YCrCb, 2, 2, 1) },
	{ DRM_FORMAT_YUV444, "YU24", 3, MAKE_YUV_INFO(YUV_YCbCr, 1, 1, 1) },
	/* RGB16 */
	{ DRM_FORMAT_ARGB4444, "AR12", 1, MAKE_RGB_INFO(4, 8, 4, 4, 4, 0, 4, 12) },
	{ DRM_FORMAT_XRGB4444, "XR12", 1, MAKE_RGB_INFO(4, 8, 4, 4, 4, 0, 0, 0) },
	{ DRM_FORMAT_ABGR4444, "AB12", 1, MAKE_RGB_INFO(4, 0, 4, 4, 4, 8, 4, 12) },
	{ DRM_FORMAT_XBGR4444, "XB12", 1, MAKE_RGB_INFO(4, 0, 4, 4, 4, 8, 0, 0) },
	{ DRM_FORMAT_RGBA4444, "RA12", 1, MAKE_RGB_INFO(4, 12, 4, 8, 4, 4, 4, 0) },
	{ DRM_FORMAT_RGBX4444, "RX12", 1, MAKE_RGB_INFO(4, 12, 4, 8, 4, 4, 0, 0) },
	{ DRM_FORMAT_BGRA4444, "BA12", 1, MAKE_RGB_INFO(4, 4, 4, 8, 4, 12, 4, 0) },
	{ DRM_FORMAT_BGRX4444, "BX12", 1, MAKE_RGB_INFO(4, 4, 4, 8, 4, 12, 0, 0) },
	{ DRM_FORMAT_ARGB1555, "AR15", 1, MAKE_RGB_INFO(5, 10, 5, 5, 5, 0, 1, 15) },
	{ DRM_FORMAT_XRGB1555, "XR15", 1, MAKE_RGB_INFO(5, 10, 5, 5, 5, 0, 0, 0) },
	{ DRM_FORMAT_ABGR1555, "AB15", 1, MAKE_RGB_INFO(5, 0, 5, 5, 5, 10, 1, 15) },
	{ DRM_FORMAT_XBGR1555, "XB15", 1, MAKE_RGB_INFO(5, 0, 5, 5, 5, 10, 0, 0) },
	{ DRM_FORMAT_RGBA5551, "RA15", 1, MAKE_RGB_INFO(5, 11, 5, 6, 5, 1, 1, 0) },
	{ DRM_FORMAT_RGBX5551, "RX15", 1, MAKE_RGB_INFO(5, 11, 5, 6, 5, 1, 0, 0) },
	{ DRM_FORMAT_BGRA5551, "BA15", 1, MAKE_RGB_INFO(5, 1, 5, 6, 5, 11, 1, 0) },
	{ DRM_FORMAT_BGRX5551, "BX15", 1, MAKE_RGB_INFO(5, 1, 5, 6, 5, 11, 0, 0) },
	{ DRM_FORMAT_RGB565, "RG16", 1, MAKE_RGB_INFO(5, 11, 6, 5, 5, 0, 0, 0) },
	{ DRM_FORMAT_BGR565, "BG16", 1, MAKE_RGB_INFO(5, 0, 6, 5, 5, 11, 0, 0) },
	/* RGB24 */
	{ DRM_FORMAT_BGR888, "BG24", 1, MAKE_RGB_INFO(8, 0, 8, 8, 8, 16, 0, 0) },
	{ DRM_FORMAT_RGB888, "RG24", 1, MAKE_RGB_INFO(8, 16, 8, 8, 8, 0, 0, 0) },
	{ DRM_FORMAT_RGB565_A8, "AR16", 2, MAKE_RGB_INFO(5, 11, 6, 5, 5, 8, 0, 0) },
	{ DRM_FORMAT_BGR565_A8, "AB16", 2, MAKE_RGB_INFO(5, 0, 6, 5, 5, 11, 0, 0) },
	/* RGB32 */
	{ DRM_FORMAT_ARGB8888, "AR24", 1, MAKE_RGB_INFO(8, 16, 8, 8, 8, 0, 8, 24) },
	{ DRM_FORMAT_XRGB8888, "XR24", 1, MAKE_RGB_INFO(8, 16, 8, 8, 8, 0, 0, 0) },
	{ DRM_FORMAT_ABGR8888, "AB24", 1, MAKE_RGB_INFO(8, 0, 8, 8, 8, 16, 8, 24) },
	{ DRM_FORMAT_XBGR8888, "XB24", 1, MAKE_RGB_INFO(8, 0, 8, 8, 8, 16, 0, 0) },
	{ DRM_FORMAT_RGBA8888, "RA24", 1, MAKE_RGB_INFO(8, 24, 8, 16, 8, 8, 8, 0) },
	{ DRM_FORMAT_RGBX8888, "RX24", 1, MAKE_RGB_INFO(8, 24, 8, 16, 8, 8, 0, 0) },
	{ DRM_FORMAT_BGRA8888, "BA24", 1, MAKE_RGB_INFO(8, 8, 8, 16, 8, 24, 8, 0) },
	{ DRM_FORMAT_BGRX8888, "BX24", 1, MAKE_RGB_INFO(8, 8, 8, 16, 8, 24, 0, 0) },
	{ DRM_FORMAT_ARGB2101010, "AR30", 1, MAKE_RGB_INFO(10, 20, 10, 10, 10, 0, 2, 30) },
	{ DRM_FORMAT_XRGB2101010, "XR30", 1, MAKE_RGB_INFO(10, 20, 10, 10, 10, 0, 0, 0) },
	{ DRM_FORMAT_ABGR2101010, "AB30", 1, MAKE_RGB_INFO(10, 0, 10, 10, 10, 20, 2, 30) },
	{ DRM_FORMAT_XBGR2101010, "XB30", 1, MAKE_RGB_INFO(10, 0, 10, 10, 10, 20, 0, 0) },
	{ DRM_FORMAT_RGBA1010102, "RA30", 1, MAKE_RGB_INFO(10, 22, 10, 12, 10, 2, 2, 0) },
	{ DRM_FORMAT_RGBX1010102, "RX30", 1, MAKE_RGB_INFO(10, 22, 10, 12, 10, 2, 0, 0) },
	{ DRM_FORMAT_BGRA1010102, "BA30", 1, MAKE_RGB_INFO(10, 2, 10, 12, 10, 22, 2, 0) },
	{ DRM_FORMAT_BGRX1010102, "BX30", 1, MAKE_RGB_INFO(10, 2, 10, 12, 10, 22, 0, 0) },
	{ DRM_FORMAT_XRGB16161616F, "XR4H", 1, MAKE_RGB_INFO(16, 32, 16, 16, 16, 0, 0, 0) },
	{ DRM_FORMAT_XBGR16161616F, "XB4H", 1, MAKE_RGB_INFO(16, 0, 16, 16, 16, 32, 0, 0) },
	{ DRM_FORMAT_ARGB16161616F, "AR4H", 1, MAKE_RGB_INFO(16, 32, 16, 16, 16, 0, 16, 48) },
	{ DRM_FORMAT_ABGR16161616F, "AB4H", 1, MAKE_RGB_INFO(16, 0, 16, 16, 16, 32, 16, 48) },
	/* VS USE in writeback, real data in [XYUV2101010] */
	{ DRM_FORMAT_XYUV8888, "YUV444_10BIT", 1, MAKE_RGB_INFO(8, 16, 8, 8, 8, 0, 8, 24) },

};

uint32_t util_format_fourcc(const char *name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(format_info); i++)
		if (!strcmp(format_info[i].name, name))
			return format_info[i].format;

	return 0;
}

const struct util_format_info *util_format_info_find(uint32_t format)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(format_info); i++)
		if (format_info[i].format == format)
			return &format_info[i];

	return NULL;
}

const struct util_format_info *util_format_mod_info_find(uint32_t format, uint64_t mod)
{
	unsigned int i;

	if (fourcc_mod_is_custom_format(mod)) {
		for (i = 0; i < ARRAY_SIZE(format_info_custom); i++)
			if (format_info_custom[i].format == format)
				return &format_info_custom[i];
	} else {
		for (i = 0; i < ARRAY_SIZE(format_info); i++)
			if (format_info[i].format == format)
				return &format_info[i];
	}

	return NULL;
}

bool is_rgb(uint32_t format)
{
	bool ret;
	switch (format) {
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
	case DRM_FORMAT_YUV444:
	case DRM_FORMAT_P010:
	case DRM_FORMAT_P210:
	case DRM_FORMAT_Y210:
	case DRM_FORMAT_YUV420_10BIT:
	case DRM_FORMAT_P016:

		ret = false;
		break;
	default:
		ret = true;
		break;
	}
	return ret;
}
