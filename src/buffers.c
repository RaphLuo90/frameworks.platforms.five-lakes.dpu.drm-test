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
 *	 Jakob Bornecrantz <jakob@tungstengraphics.com>
 * Copyright 2008 Intel Corporation
 *	 Jesse Barnes <jesse.barnes@intel.com>
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

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>

#include <math.h>

#include "type.h"

#include "drm.h"
#include "drm/vs_drm_fourcc.h"
#include "drm/vs_drm.h"

#include "libdrm_macros.h"
#include "xf86drm.h"
#include "util/format.h"
#include "util/log.h"
#include "util_math.h"

#include "buffers.h"
#include "vs_bo_helper.h"
#include "option.h"

#if CONFIG_VERISILICON_MD5_CHECK
#include <openssl/md5.h>
#endif

/* temp for bmp */
#define BIT_RGB 0 /* No compression - straight BGR data */
#define BIT_RLE8 1 /* 8-bit run-length compression */
#define BIT_RLE4 2 /* 4-bit run-length compression */
#define BIT_BITFIELDS 3 /* RGB bitmap with RGB masks */

#define TILESTATUS_BITS 4
#define DEC_HEADER_SIZE 128

#define VS_ABS abs
#define VS_MAX(a, b) ((a) > (b) ? (a) : (b))
#define VS_MIN(a, b) ((a) < (b) ? (a) : (b))

/* align needs to be power of 2 */
#define UP_ALIGN(x, align) ((x + align - 1) & ~(align - 1))

#define fourcc_mod_vs_get_type(val) (((val)&DRM_FORMAT_MOD_VS_TYPE_MASK) >> 53)
#define fourcc_mod_vs_get_tile_mode(val) (uint8_t)((val)&DRM_FORMAT_MOD_VS_DEC_TILE_MODE_MASK)
#define fourcc_mod_vs_is_compressed(val) \
	!!(fourcc_mod_vs_get_type(val) == DRM_FORMAT_MOD_VS_TYPE_COMPRESSED ? 1 : 0)
#define fourcc_mod_vs_is_dec400a(val) \
	!!(fourcc_mod_vs_get_type(val) == DRM_FORMAT_MOD_VS_TYPE_DEC400A ? 1 : 0)
#define fourcc_mod_vs_is_pvric(val) \
	!!(fourcc_mod_vs_get_type(val) == DRM_FORMAT_MOD_VS_TYPE_PVRIC ? 1 : 0)
#define fourcc_mod_vs_is_decnano(val) \
	!!(fourcc_mod_vs_get_type(val) == DRM_FORMAT_MOD_VS_TYPE_DECNANO ? 1 : 0)
#define fourcc_mod_vs_is_etc2(val) \
	!!(fourcc_mod_vs_get_type(val) == DRM_FORMAT_MOD_VS_TYPE_ETC2 ? 1 : 0)
#define fourcc_mod_vs_is_normal(val) !(fourcc_mod_vs_get_type(val) | DRM_FORMAT_MOD_VS_TYPE_NORMAL)

/**** BMP file header structure ****/
typedef struct BMPFILEHEADER {
	unsigned short bfType; /* Magic number for file */
	unsigned int bfSize; /* Size of file */
	unsigned short bfReserved1; /* Reserved */
	unsigned short bfReserved2; /* ... */
	unsigned int bfOffBits; /* Offset to bitmap data */
} BMPFILEHEADER;

/**** BMP file info structure ****/
typedef struct BMPINFOHEADER {
	unsigned int biSize; /* Size of info header */
	int biWidth; /* Width of image */
	int biHeight; /* Height of image */
	unsigned short biPlanes; /* Number of color planes */
	unsigned short biBitCount; /* Number of bits per pixel */
	unsigned int biCompression; /* Type of compression to use */
	unsigned int biSizeImage; /* Size of image data */
	int biXPelsPerMeter; /* X pixels per meter */
	int biYPelsPerMeter; /* Y pixels per meter */
	unsigned int biClrUsed; /* Number of colors used */
	unsigned int biClrImportant; /* Number of important colors */
} BMPINFOHEADER;

/**** Colormap entry structure ****/
typedef struct RGB {
	unsigned char rgbBlue; /* Blue value */
	unsigned char rgbGreen; /* Green value */
	unsigned char rgbRed; /* Red value */
	unsigned char rgbReserved; /* Reserved */
} RGB;

/**** Bitmap information structure ****/
typedef struct _BMPINFO {
	BMPINFOHEADER bmiHeader; /* Image header */
	union {
		RGB bmiColors[256]; /* Image colormap */
		unsigned int mask[3]; /* RGB masks */
	};
} BMPINFO;

typedef struct {
	uint8_t magic[3]; /*"VIV"*/
	uint8_t version;
} VIMG_FILEHEADER;

typedef struct {
	uint32_t format;
	uint32_t tiling;
	uint32_t imageStride;
	uint32_t imageWidth;
	uint32_t imageHeight;
	uint32_t bitsOffset;
} VIMG_V1;

/* -----------------------------------------------------------------------------
 * Buffers management
 */

uint32_t __get_bpp(uint32_t format, uint64_t mod)
{
	uint32_t bpp = 0;
	switch (format) {
	case DRM_FORMAT_C8:
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
	case DRM_FORMAT_YUV444:
		bpp = 8;
		break;

	case DRM_FORMAT_ARGB4444:
	case DRM_FORMAT_XRGB4444:
	case DRM_FORMAT_ABGR4444:
	case DRM_FORMAT_XBGR4444:
	case DRM_FORMAT_RGBA4444:
	case DRM_FORMAT_RGBX4444:
	case DRM_FORMAT_BGRA4444:
	case DRM_FORMAT_BGRX4444:
	case DRM_FORMAT_ARGB1555:
	case DRM_FORMAT_XRGB1555:
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_XBGR1555:
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_RGBX5551:
	case DRM_FORMAT_BGRA5551:
	case DRM_FORMAT_BGRX5551:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
	case DRM_FORMAT_P010:
		bpp = 16;
		break;

	case DRM_FORMAT_BGR888:
	case DRM_FORMAT_RGB888:
		if (fourcc_mod_is_custom_format(mod))
			bpp = 8;
		else
			bpp = 24;
		break;
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_BGRA8888:
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_XRGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_XBGR2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_RGBX1010102:
	case DRM_FORMAT_BGRA1010102:
	case DRM_FORMAT_BGRX1010102:
		bpp = 32;
		break;

	case DRM_FORMAT_XRGB16161616F:
	case DRM_FORMAT_XBGR16161616F:
	case DRM_FORMAT_ARGB16161616F:
	case DRM_FORMAT_ABGR16161616F:
	case DRM_FORMAT_Y210:
		bpp = 64;
		break;

	default:
		DTESTLOGE("unsupported format 0x%08x", format);
		return -1;
	}
	return bpp;
}

struct bo *bo_create_dumb(int fd, unsigned int width, unsigned int height, unsigned int bpp)
{
	struct drm_mode_create_dumb arg;
	struct bo *bo;
	int ret;

	bo = calloc(1, sizeof(*bo));
	if (bo == NULL) {
		fprintf(stderr, "failed to allocate buffer object\n");
		return NULL;
	}

	memset(&arg, 0, sizeof(arg));
	arg.bpp = bpp;
	arg.width = width;
	arg.height = height;

	ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &arg);
	if (ret) {
		fprintf(stderr, "failed to create dumb buffer: %s\n", strerror(errno));
		free(bo);
		return NULL;
	}

	bo->fd = fd;
	bo->handle = arg.handle;
	bo->size = arg.size;
	bo->pitch = arg.pitch;

	/* for wb connector */
	bo->width = width;
	bo->height = height;

	return bo;
}

int bo_map(struct bo *bo, void **out)
{
	struct drm_mode_map_dumb arg;
	void *map;
	int ret;

	memset(&arg, 0, sizeof(arg));
	arg.handle = bo->handle;

	ret = drmIoctl(bo->fd, DRM_IOCTL_MODE_MAP_DUMB, &arg);
	if (ret)
		return ret;

	map = drm_mmap(0, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED, bo->fd, arg.offset);
	if (map == MAP_FAILED)
		return -EINVAL;

	bo->ptr = map;
	*out = map;

	return 0;
}

void bo_unmap(struct bo *bo)
{
	if (!bo->ptr)
		return;

	drm_munmap(bo->ptr, bo->size);
	bo->ptr = NULL;
}

struct bo *bo_create(int fd, unsigned int format, unsigned int width, unsigned int height,
		     unsigned int handles[4], unsigned int pitches[4], unsigned int offsets[4],
		     enum util_fill_pattern pattern, uint64_t mod)
{
	unsigned int virtual_height;
	struct bo *bo;
	unsigned int bpp;
	void *planes[3] = {
		0,
	};
	void *virtual;
	int ret;

	bpp = __get_bpp(format, mod);

	switch (format) {
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
	case DRM_FORMAT_P010:
		virtual_height = height * 3 / 2;
		break;

	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
		virtual_height = height * 2;
		break;

	case DRM_FORMAT_YUV444:
		virtual_height = height * 3;
		break;

	case DRM_FORMAT_RGB888:
		if (fourcc_mod_is_custom_format(mod))
			virtual_height = height * 3;
		else
			virtual_height = height;
		break;
	default:
		virtual_height = height;
		break;
	}
	bo = bo_create_dumb(fd, width, virtual_height, bpp);
	if (!bo)
		return NULL;

	ret = bo_map(bo, &virtual);
	if (ret) {
		fprintf(stderr, "failed to map buffer: %s\n", strerror(-errno));
		bo_destroy(bo);
		return NULL;
	}

	/* just testing a limited # of formats to test single
	 * and multi-planar path.. would be nice to add more..
	 */
	switch (format) {
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
		offsets[0] = 0;
		handles[0] = bo->handle;
		pitches[0] = bo->pitch;

		planes[0] = virtual;
		break;

	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
	case DRM_FORMAT_P010:
		offsets[0] = 0;
		handles[0] = bo->handle;
		pitches[0] = bo->pitch;
		pitches[1] = pitches[0];
		offsets[1] = pitches[0] * height;
		handles[1] = bo->handle;

		planes[0] = virtual;
		planes[1] = virtual + offsets[1];
		break;

	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		offsets[0] = 0;
		handles[0] = bo->handle;
		pitches[0] = bo->pitch;
		pitches[1] = pitches[0] / 2;
		offsets[1] = pitches[0] * height;
		handles[1] = bo->handle;
		pitches[2] = pitches[1];
		offsets[2] = offsets[1] + pitches[1] * height / 2;
		handles[2] = bo->handle;

		planes[0] = virtual;
		planes[1] = virtual + offsets[1];
		planes[2] = virtual + offsets[2];
		break;

	case DRM_FORMAT_C8:
	case DRM_FORMAT_ARGB4444:
	case DRM_FORMAT_XRGB4444:
	case DRM_FORMAT_ABGR4444:
	case DRM_FORMAT_XBGR4444:
	case DRM_FORMAT_RGBA4444:
	case DRM_FORMAT_RGBX4444:
	case DRM_FORMAT_BGRA4444:
	case DRM_FORMAT_BGRX4444:
	case DRM_FORMAT_ARGB1555:
	case DRM_FORMAT_XRGB1555:
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_XBGR1555:
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_RGBX5551:
	case DRM_FORMAT_BGRA5551:
	case DRM_FORMAT_BGRX5551:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_BGRA8888:
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_XRGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_XBGR2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_RGBX1010102:
	case DRM_FORMAT_BGRA1010102:
	case DRM_FORMAT_BGRX1010102:
	case DRM_FORMAT_XRGB16161616F:
	case DRM_FORMAT_XBGR16161616F:
	case DRM_FORMAT_ARGB16161616F:
	case DRM_FORMAT_ABGR16161616F:
	case DRM_FORMAT_Y210:
		offsets[0] = 0;
		handles[0] = bo->handle;
		pitches[0] = bo->pitch;

		planes[0] = virtual;
		break;
	case DRM_FORMAT_YUV444:
		offsets[0] = 0;
		handles[0] = bo->handle;
		pitches[0] = bo->pitch;
		pitches[1] = pitches[0];
		offsets[1] = pitches[0] * height;
		handles[1] = bo->handle;
		pitches[2] = pitches[1];
		offsets[2] = offsets[1] + pitches[1] * height;
		handles[2] = bo->handle;

		planes[0] = virtual;
		planes[1] = virtual + offsets[1];
		planes[2] = virtual + offsets[2];
		break;
	case DRM_FORMAT_RGB888:
	case DRM_FORMAT_BGR888:
		if (fourcc_mod_is_custom_format(mod)) {
			offsets[0] = 0;
			handles[0] = bo->handle;
			pitches[0] = bo->pitch;
			pitches[1] = pitches[0];
			offsets[1] = pitches[0] * height;
			handles[1] = bo->handle;
			pitches[2] = pitches[1];
			offsets[2] = offsets[1] + pitches[1] * height;
			handles[2] = bo->handle;
			planes[0] = virtual;
			planes[1] = virtual + offsets[1];
			planes[2] = virtual + offsets[2];
		} else {
			offsets[0] = 0;
			handles[0] = bo->handle;
			pitches[0] = bo->pitch;
			planes[0] = virtual;
		}
		break;
	}

	util_fill_pattern(format, pattern, planes, width, height, pitches[0]);
	bo_unmap(bo);

	return bo;
}

void bo_destroy(struct bo *bo)
{
	struct drm_mode_destroy_dumb arg;
	int ret;

	memset(&arg, 0, sizeof(arg));
	arg.handle = bo->handle;

	ret = drmIoctl(bo->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &arg);
	if (ret)
		fprintf(stderr, "failed to destroy dumb buffer: %s\n", strerror(errno));

	free(bo);
}

/* -----------------------------------------------------------------------------
 * Bmp read-related func
 */

static int _vs_file_seek(void *p, uint32_t offset, int end)
{
	if (end) {
		return fseek((FILE *)p, offset, SEEK_END);
	}
	return fseek((FILE *)p, offset, SEEK_SET);
}

static int _vs_file_read_word(void *p)
{
	FILE *fp = (FILE *)p;
	uint8_t b0, b1;

	b0 = getc(fp);
	b1 = getc(fp);

	return ((b1 << 8) | b0);
}

static int _vs_file_read_dword(void *p)
{
	FILE *fp = (FILE *)p;
	uint8_t b0, b1, b2, b3;

	b0 = getc(fp);
	b1 = getc(fp);
	b2 = getc(fp);
	b3 = getc(fp);

	return ((((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}

static int _vs_file_read_long(void *p)
{
	FILE *fp = (FILE *)p;
	uint8_t b0, b1, b2, b3;

	b0 = getc(fp);
	b1 = getc(fp);
	b2 = getc(fp);
	b3 = getc(fp);

	return ((int)(((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}

/* -----------------------------------------------------------------------------
 * Bmp storage-related func
 */

static int vs_file_write_word(void *p, unsigned short w)
{
	FILE *fp = (FILE *)p;
	putc(w, fp);
	return (putc(w >> 8, fp));
}

static int vs_file_write_dword(void *p, uint32_t dw)
{
	FILE *fp = (FILE *)p;
	putc(dw, fp);
	putc(dw >> 8, fp);
	putc(dw >> 16, fp);
	return (putc(dw >> 24, fp));
}

static int vs_file_write_long(void *p, int l)
{
	FILE *fp = (FILE *)p;
	putc(l, fp);
	putc(l >> 8, fp);
	putc(l >> 16, fp);
	return (putc(l >> 24, fp));
}

static int vs_save_dib_bitmap(const char *filename, BMPINFO *info, unsigned char *bits,
			      uint32_t stride)
{
	FILE *fp = NULL; /* Open file pointer */
	int size = 0; /* Size of file */
	int infosize = 0; /* Size of bitmap info */
	int bitsize = 0; /* Size of bitmap pixels */
	int i = 0, j = 0;
	uint32_t bmpStride = info->bmiHeader.biWidth * ((info->bmiHeader.biBitCount + 7) / 8);

	if (stride < bmpStride)
		return -1;

	fp = fopen(filename, "wb");

	if (!fp) {
		printf("fail open file\n");
		return -1;
	}

	/* Figure out the bitmap size */
	if (info->bmiHeader.biSizeImage == 0) {
		bitsize = info->bmiHeader.biWidth * ((info->bmiHeader.biBitCount + 7) / 8) *
			  VS_ABS(info->bmiHeader.biHeight);
	} else {
		bitsize = info->bmiHeader.biSizeImage;
	}

	/* Figure out the header size */
	infosize = sizeof(BMPINFOHEADER);
	switch (info->bmiHeader.biCompression) {
	case BIT_BITFIELDS:
		infosize += 12; /* Add 3 RGB doubleword masks */
		if (info->bmiHeader.biClrUsed == 0)
			break;
	case BIT_RGB:
		if (info->bmiHeader.biBitCount > 8 && info->bmiHeader.biClrUsed == 0)
			break;
	case BIT_RLE8:
	case BIT_RLE4:
		if (info->bmiHeader.biClrUsed == 0)
			infosize += (1 << info->bmiHeader.biBitCount) * 4;
		else
			infosize += info->bmiHeader.biClrUsed * 4;
		break;
	}

	size = sizeof(BMPFILEHEADER) + infosize + bitsize;

	/* Write the file header, bitmap information, and bitmap pixel data... */
	vs_file_write_word(fp, 0x4D42); /* bfType */
	vs_file_write_dword(fp, size); /* bfSize */
	vs_file_write_word(fp, 0); /* bfReserved1 */
	vs_file_write_word(fp, 0); /* bfReserved2 */
	vs_file_write_dword(fp, 14 + infosize); /* bfOffBits */

	vs_file_write_dword(fp, info->bmiHeader.biSize);
	vs_file_write_long(fp, info->bmiHeader.biWidth);
	vs_file_write_long(fp, abs(info->bmiHeader.biHeight) /*info->bmiHeader.biHeight*/);
	vs_file_write_word(fp, info->bmiHeader.biPlanes);
	vs_file_write_word(fp, info->bmiHeader.biBitCount);
	vs_file_write_dword(fp, info->bmiHeader.biCompression);
	vs_file_write_dword(fp, info->bmiHeader.biSizeImage);
	vs_file_write_long(fp, info->bmiHeader.biXPelsPerMeter);
	vs_file_write_long(fp, info->bmiHeader.biYPelsPerMeter);
	vs_file_write_dword(fp, info->bmiHeader.biClrUsed);
	vs_file_write_dword(fp, info->bmiHeader.biClrImportant);

	if (infosize > 40) {
		int n = (infosize - 40) / 4;
		RGB *temp = info->bmiColors;
		unsigned int *p = (unsigned int *)(temp);

		while (n > 0) {
			vs_file_write_dword(fp, *p);
			p++;
			n--;
		}
	}

	for (i = 0; i < VS_ABS(info->bmiHeader.biHeight); i++) {
		if (info->bmiHeader.biHeight > 0) {
			if (info->bmiHeader.biBitCount == 32) {
				for (j = 0; j < info->bmiHeader.biWidth; j++)
					fwrite(bits + j * 4 + stride * i, 1, 4, fp);
			} else
				fwrite(bits + stride * i, 1, bmpStride, fp);
		} else {
			if (info->bmiHeader.biBitCount == 32) {
				for (j = 0; j < info->bmiHeader.biWidth; j++)
					fwrite(bits + j * 4 +
						       stride * (abs(info->bmiHeader.biHeight) - 1 -
								 i),
					       1, 4, fp);
			} else
				fwrite(bits + stride * (abs(info->bmiHeader.biHeight) - 1 - i), 1,
				       bmpStride, fp);
		}
	}

	fclose(fp);
	return 0;
}

static dtest_status vs_save_dib(void *bits, pipe_t *pipe, uint32_t stride, uint32_t width,
				uint32_t height, const char *bmpFileName)
{
	if (bits && bmpFileName && bmpFileName[0]) {
		BMPINFO bitmap;
		uint32_t bitCount;

		switch (pipe->fourcc) {
		case DRM_FORMAT_RGB888:
		case DRM_FORMAT_BGR888:
			bitCount = 24;
			break;
		case DRM_FORMAT_RGB565:
		case DRM_FORMAT_BGR565:
			bitmap.mask[0] = 0x0000F800;
			bitmap.mask[1] = 0x000007E0;
			bitmap.mask[2] = 0x0000001F;
			bitCount = 16;
			break;
		case DRM_FORMAT_ARGB8888:
		case DRM_FORMAT_XRGB8888:
		case DRM_FORMAT_XRGB16161616F: /*For DP RGB*/
			bitmap.mask[0] = 0x00FF0000;
			bitmap.mask[1] = 0x0000FF00;
			bitmap.mask[2] = 0x000000FF;
			bitCount = 32;
			break;
		case DRM_FORMAT_RGBA8888:
		case DRM_FORMAT_RGBX8888:
			bitmap.mask[0] = 0xFF000000;
			bitmap.mask[1] = 0x00FF0000;
			bitmap.mask[2] = 0x0000FF00;
			bitCount = 32;
			break;
		case DRM_FORMAT_ABGR8888:
		case DRM_FORMAT_XBGR8888:
			bitmap.mask[0] = 0x000000FF;
			bitmap.mask[1] = 0x0000FF00;
			bitmap.mask[2] = 0x00FF0000;
			bitCount = 32;
			break;
		case DRM_FORMAT_BGRA8888:
		case DRM_FORMAT_BGRX8888:
			bitmap.mask[0] = 0x0000FF00;
			bitmap.mask[1] = 0x00FF0000;
			bitmap.mask[2] = 0xFF000000;
			bitCount = 32;
			break;
		case DRM_FORMAT_ARGB2101010:
		case DRM_FORMAT_XRGB2101010:
			bitmap.mask[0] = 0x3FF00000;
			bitmap.mask[1] = 0x000FFC00;
			bitmap.mask[2] = 0x000003FF;
			bitCount = 32;
			break;
		case DRM_FORMAT_RGBA1010102:
		case DRM_FORMAT_RGBX1010102:
			bitmap.mask[0] = 0xFFC00000;
			bitmap.mask[1] = 0x003FF000;
			bitmap.mask[2] = 0x00000FFC;
			bitCount = 32;
			break;
		case DRM_FORMAT_ABGR2101010:
		case DRM_FORMAT_XBGR2101010:
			bitmap.mask[0] = 0x000003FF;
			bitmap.mask[1] = 0x000FFC00;
			bitmap.mask[2] = 0x3FF00000;
			bitCount = 32;
			break;
		case DRM_FORMAT_BGRA1010102:
		case DRM_FORMAT_BGRX1010102:
			bitmap.mask[0] = 0x00000FFC;
			bitmap.mask[1] = 0x003FF000;
			bitmap.mask[2] = 0xFFC00000;
			bitCount = 32;
			break;
		case DRM_FORMAT_ARGB1555:
		case DRM_FORMAT_XRGB1555:
			bitmap.mask[0] = 0x00007C00;
			bitmap.mask[1] = 0x000003E0;
			bitmap.mask[2] = 0x0000001F;
			bitCount = 16;
			break;
		case DRM_FORMAT_ARGB4444:
		case DRM_FORMAT_XRGB4444:
			bitmap.mask[0] = 0x00000F00;
			bitmap.mask[1] = 0x000000F0;
			bitmap.mask[2] = 0x0000000F;
			bitCount = 16;
			break;
		default:
			/* can not save and display */
			DTESTLOGI("Unsupported format %s", pipe->format_str);
			return DTEST_FAILURE;
		}

		/* Fill in the BITMAPINFOHEADER information. */
		bitmap.bmiHeader.biSize = sizeof(bitmap.bmiHeader);
		bitmap.bmiHeader.biWidth = width;
		bitmap.bmiHeader.biHeight = -(int)height;
		bitmap.bmiHeader.biPlanes = 1;
		bitmap.bmiHeader.biBitCount = bitCount;

		/* BIT_BITFIELDS is only valid when used 16 and 32 bpp bitmaps, which color table
		 * consists of three DWORD color maks that specify the red, green and blue components,
		 * respectively, of each pixel.
		 */
		if (pipe->fourcc == DRM_FORMAT_RGB888 || pipe->fourcc == DRM_FORMAT_BGR888)
			bitmap.bmiHeader.biCompression = BIT_RGB;
		else
			bitmap.bmiHeader.biCompression = BIT_BITFIELDS;

		bitmap.bmiHeader.biSizeImage = 0;
		bitmap.bmiHeader.biXPelsPerMeter = 0;
		bitmap.bmiHeader.biYPelsPerMeter = 0;
		bitmap.bmiHeader.biClrUsed = 0;
		bitmap.bmiHeader.biClrImportant = 0;

		if (vs_save_dib_bitmap(bmpFileName, (BMPINFO *)&bitmap, bits, stride) == 0) {
			DTESTLOGI("Scusses save bmp result.");
			return DTEST_SUCCESS;
		} else {
			DTESTLOGW("Save bmp fail!");
			return DTEST_FAILURE;
		}
	}
	return DTEST_FAILURE;
}

static int _fill_fbc_filename(char *filename, char *dest, char *dest_uv, uint32_t num_planes)
{
	char *suffix = NULL;
	uint32_t file_len = 0, suffix_len = 0, append_len = 3;

	file_len = strlen(filename);
	suffix = strrchr(filename, '.');
	if (suffix == NULL)
		return -1;

	suffix_len = strlen(suffix);
	if (file_len <= suffix_len)
		return -1;

	strncpy(dest, filename, file_len);
	if (num_planes > 1) {
		strncpy(dest_uv, filename, file_len - suffix_len);
		dest_uv[file_len - suffix_len] = '\0';
		strncat(dest_uv, "_uv", append_len);
		strncat(dest_uv, suffix, suffix_len);
	}

	return 0;
}

static int _load_fbc_area(char *filename, void *va_addr, uint32_t src_size, uint32_t header_size,
			  uint64_t offset)
{
	FILE *file;
	uint8_t *base_addr = NULL;
	int ret = 0;
	size_t cnt;

	file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "file [%s] open failed\n", filename);
		ret = errno;
		return ret;
	}

	base_addr = (uint8_t *)((uint64_t)va_addr + offset - header_size);

	cnt = fread(base_addr, 1, src_size, file);
	(void)cnt;

	fclose(file);

	return 0;
}

static int _load_bmp_area(char *file_name, uint8_t *buf, drm_vs_bo_param bo_param,
			  struct bo *plane_bo, uint32_t format, uint32_t *lut)
{
	int ret = 0;
	uint32_t buf_width = bo_param.width;
	uint32_t buf_height = bo_param.height;
	uint32_t buf_stride = plane_bo->pitch;

	uint32_t i = 0, j = 0;
	uint32_t offset = 0, bpp = 0;
	uint32_t img_width = 0, img_height = 0, img_stride = 0;
	uint32_t width = 0, height = 0, stride = 0;
	uint32_t h = 0;
	void *fp = NULL;
	uint8_t *img_buf = NULL;
	/* TODO: temporarily only for tile height = 1 */
	uint32_t tile_height = 1;

	if (!file_name || !buf)
		return -1;

	fp = fopen(file_name, "rb");
	if (!fp) {
		printf("Can not find %s\n", file_name);
		return -1;
	}

	_vs_file_read_word(fp);
	_vs_file_read_dword(fp);
	_vs_file_read_word(fp);
	_vs_file_read_word(fp);
	offset = _vs_file_read_dword(fp);
	_vs_file_read_dword(fp);
	img_width = _vs_file_read_long(fp);
	h = _vs_file_read_long(fp);
	img_height = VS_ABS(h);
	_vs_file_read_word(fp);
	bpp = _vs_file_read_word(fp);
	img_stride = (img_width * bpp / 8 + 3) & (~3);

	width = VS_MIN(buf_width, img_width);
	height = VS_MIN(buf_height, img_height);
	stride = VS_MIN(buf_stride, img_stride);

	/* biCompression. */
	_vs_file_read_dword(fp);
	/* biSizeImage. */
	_vs_file_read_dword(fp);
	/* biXPelsPerMeter*/
	_vs_file_read_long(fp);
	/* biYPelsPerMeter*/
	_vs_file_read_long(fp);

	/* biClrUsed. */
	_vs_file_read_dword(fp);
	/* biClrImportant. */
	_vs_file_read_dword(fp);

	/* Check LUT. */
	if (lut) {
		uint32_t info_size = offset - 14;

		if (info_size > 40) {
			uint32_t n = (info_size - 40) / 4;
			uint32_t *p = lut;

			while (n-- > 0)
				*p++ = _vs_file_read_dword(fp);
		}
	}

	if (!_vs_file_seek(fp, offset, false)) {
		uint8_t *bits = NULL;
		uint32_t pos = 0;
		uint8_t *img_bits = NULL;

		if (h > 0) {
			if (tile_height > 1) {
				/*
				 * Use tmp buffer to make life easier for
				 * tiled and bottom-to-top stored bmp file.
				 */
				uint32_t size = img_stride * img_height;

				img_buf = malloc(size);

				for (i = 0; i < img_height; i++) {
					if (fread(img_buf + (img_height - i - 1) * img_stride, 1,
						  img_stride, (FILE *)fp) < img_stride) {
						ret = -1;
						goto exit;
					}
				}
			} else if (img_height > buf_height)
				_vs_file_seek(fp,
					      ftell((FILE *)fp) +
						      (img_height - buf_height) * img_stride,
					      false);
		}

		for (i = 0; i < height; i += tile_height) {
			if (img_buf) {
				/* h >= 0 && tile_height > 1 */
				bits = (uint8_t *)buf + i * buf_stride;
				img_bits = img_buf + i * img_stride;

				memcpy(bits, img_bits, img_stride * tile_height);
			} else {
				/* h < 0 || tile_height == 1 */
				bits = (h > 0) ? (uint8_t *)buf +
							 (height - i - tile_height) * buf_stride :
						       (uint8_t *)buf + i * buf_stride;

				pos = ftell((FILE *)fp);

				if (bpp == 24) {
					for (j = 0; j < width * tile_height; j++) {
						if (fread(bits + j * 4, 1, 3, fp) < 3) {
							ret = -1;
							goto exit;
						}
					}
				} else {
					if (fread(bits, 1, stride * tile_height, (FILE *)fp) <
					    stride * tile_height) {
						ret = -1;
						goto exit;
					}
				}

				/* Move to the next position. */
				_vs_file_seek(fp, pos + img_stride * tile_height, false);
			}
		}
	} else
		ret = -1;

exit:
	if (img_buf)
		free(img_buf);

	if (fp)
		fclose((FILE *)fp);

	return ret;
}

static int _load_dec_raw_area(char (*file_name)[RESOURCE_LEN + 10], uint8_t **plane_addr,
			      drm_vs_bo_param *bo_param, struct bo *plane_bo[4], uint32_t format,
			      uint64_t mod)
{
	int ret = 0;
	const struct util_format_info *info = NULL;
	void *fp = NULL, *ts_fp[3] = { NULL, NULL, NULL };
	uint32_t i = 0, j = 0, plane_num = 0;
	uint8_t *p = NULL;
	int h_tile[3] = { 1, 1, 1 };

	info = util_format_info_find(format);
	if (!info) {
		printf("can't get format info\n");
		return -1;
	}
	plane_num = info->num_planes;
	drm_vs_get_tile_height(format, mod, h_tile);

	if (file_name == NULL || *plane_addr == NULL) {
		ret = -1;
		goto exit;
	}

	if ((fp = fopen(file_name[0], "rb")) == NULL) {
		printf("Can not find %s\n", file_name[0]);
		ret = -1;
		goto exit;
	}

	for (i = 0; i < plane_num; i++) {
		if ((ts_fp[i] = fopen(file_name[i + 1], "rb")) == NULL) {
			printf("Can not find %s\n", file_name[i + 1]);
			ret = -1;
			goto exit;
		}
	}

	for (i = 0; i < plane_num; i++) {
		uint32_t pos_line = 0, pos_plane = 0, ts_buf_height = 0;
		p = plane_addr[i];
		pos_plane = ftell((FILE *)fp);
		ts_buf_height = bo_param[i].ts_buf_size / (bo_param[i].width * bo_param[i].bpp / 8);

		for (j = 0; j < bo_param[i].height - ts_buf_height; j += h_tile[i]) {
			pos_line = ftell((FILE *)fp);

			fread(p, 1, bo_param[i].width * h_tile[i] * bo_param[i].bpp / 8,
			      (FILE *)fp);
			p += plane_bo[i]->pitch * h_tile[i];
			_vs_file_seek(
				fp, pos_line + bo_param[i].width * h_tile[i] * bo_param[i].bpp / 8,
				false);
		}

		/*  read tile status buffer behind image plane buffer for DEC400 */
		fread(p, 1, bo_param[i].ts_buf_size, (FILE *)ts_fp[i]);

		_vs_file_seek(fp,
			      pos_plane + bo_param[i].width * (bo_param[i].height - ts_buf_height) *
						  bo_param[i].bpp / 8,
			      false);
	}

exit:
	if (fp)
		fclose((FILE *)fp);

	for (i = 0; i < plane_num; i++) {
		if (ts_fp[i])
			fclose((FILE *)ts_fp[i]);
	}

	return ret;
}

static int _load_dec_raw_area_from_bin(char (*file_name)[RESOURCE_LEN + 10], uint8_t **plane_addr,
				       drm_vs_bo_param *bo_param, struct bo *plane_bo[4],
				       uint32_t format, uint64_t mod)
{
	int ret = 0;
	const struct util_format_info *info = NULL;
	void *fp = NULL;
	uint32_t i = 0, j = 0, plane_num = 0;
	uint8_t *p = NULL;
	int h_tile[3] = { 1, 1, 1 };
	uint64_t modifiers[4] = { 0 };

	info = util_format_info_find(format);
	plane_num = info->num_planes;
	drm_vs_get_tile_height(format, mod, h_tile);

	if (file_name == NULL || *plane_addr == NULL) {
		ret = -1;
		goto exit;
	}

	if ((fp = fopen(file_name[0], "rb")) == NULL) {
		printf("Can not find %s\n", file_name[0]);
		ret = -1;
		goto exit;
	}

	_vs_file_seek(fp, 0, true);
	uint32_t bfsize = ftell((FILE *)fp);
	if (bfsize % 64 != 0) {
		printf("DEC400: ERROR!! Invalid bin file size(%d)!!\n", bfsize);
		printf("DEC400: bin file size should align to 64 bytes!!\n");
		ret = -1;
		goto exit;
	}
	_vs_file_seek(fp, 0, false);

	if (vs_mod_config(format, mod, plane_num, modifiers) < 0) {
		ret = -1;
		goto exit;
	}

	for (i = 0; i < plane_num; i++) {
		uint32_t pos_line = 0, pos_plane = 0, ts_buf_height = 0;
		uint32_t tile_num = 0, pos_ts_base = 0, data_size = 0, data_tile_size = 0;
		uint32_t ts_buf_offset = 0, ts_size = 0, ts_size_memory = 0;
		double ts_size_double = 0.f;
		uint8_t ts_bits = TILESTATUS_BITS;
		uint32_t header_size = DEC_HEADER_SIZE;

		if (dec_mod_is_fc(mod))
			header_size = DEC_HEADER_SIZE + DEC_HEADER_SIZE * dec_mod_get_fc_size(mod);

		p = plane_addr[i];
		pos_plane = ftell((FILE *)fp);
		ts_buf_height = bo_param[i].ts_buf_size / (bo_param[i].width * bo_param[i].bpp / 8);

		data_size = bo_param[i].width * (bo_param[i].height - ts_buf_height) *
			    bo_param[i].bpp / 8;
		data_tile_size = vs_get_dec_tile_size(fourcc_mod_vs_get_tile_mode(modifiers[i]),
						      bo_param[i].bpp);
		pos_ts_base = pos_plane + header_size;
		ts_buf_offset = pos_ts_base % data_tile_size;
		tile_num = data_size / data_tile_size;
		ts_size_double = tile_num * ts_bits / 8;
		ts_size = (uint32_t)ceil(ts_size_double);
		ts_size_memory = UP_ALIGN(ts_buf_offset + ts_size, data_tile_size) - ts_buf_offset;
#if CONFIG_VERISILICON_DEC400_CONFORMANCE_TEST
		printf("DEC400: ts_size_memory: %d  bo_param[i].ts_buf_size: %d \n", ts_size_memory,
		       bo_param[i].ts_buf_size);
		printf("DEC400: Header start: %d  ts start: %d data start: %d \n", pos_plane,
		       pos_plane + header_size, pos_plane + header_size + ts_size_memory);
		printf("DEC400: bpp: %d  data_size: %d data_tile_size: %d \n", bo_param[i].bpp,
		       data_size, data_tile_size);
#endif
		/* read data buffer for DEC400 */
		_vs_file_seek(fp, pos_plane + header_size + ts_size_memory, false);
		for (j = 0; j < bo_param[i].height - ts_buf_height; j += h_tile[i]) {
			pos_line = ftell((FILE *)fp);
			fread(p, 1, bo_param[i].width * h_tile[i] * bo_param[i].bpp / 8,
			      (FILE *)fp);
			p += plane_bo[i]->pitch * h_tile[i];
			_vs_file_seek(
				fp, pos_line + bo_param[i].width * h_tile[i] * bo_param[i].bpp / 8,
				false);
		}

		/* read tile status buffer for DEC400 */
		if (dec_mod_is_fc(mod)) {
			/* fast clear enable, read header buffer for DEC400 */
			_vs_file_seek(fp, pos_plane, false);
			fread(p, 1, ts_size_memory + header_size, (FILE *)fp);
		} else {
			_vs_file_seek(fp, pos_plane + header_size, false);
			fread(p, 1, ts_size_memory, (FILE *)fp);
		}

		_vs_file_seek(fp, pos_plane + header_size + ts_size_memory + data_size, false);
	}

exit:
	if (fp)
		fclose((FILE *)fp);

	return ret;
}

static int _load_dec400a_raw_area(char (*file_name)[RESOURCE_LEN + 10], uint8_t **plane_addr,
				  drm_vs_bo_param *bo_param, struct bo *plane_bo[4],
				  uint32_t format)
{
	int ret = 0;
	void *fp = NULL;
	uint8_t *p = NULL;
	uint64_t size = 0;

	if (file_name == NULL || *plane_addr == NULL) {
		ret = -1;
		goto exit;
	}

	if ((fp = fopen(file_name[0], "rb")) == NULL) {
		printf("Can not find %s\n", file_name[0]);
		ret = -1;
		goto exit;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	rewind(fp);

	p = plane_addr[0];

	fread(p, 1, size, (FILE *)fp);

exit:
	if (fp)
		fclose((FILE *)fp);

	return ret;
}

static int _load_decnano_etc2_raw_area(char (*file_name)[RESOURCE_LEN + 10], uint8_t **plane_addr,
				       drm_vs_bo_param *bo_param, struct bo *plane_bo[4],
				       uint32_t format, uint32_t head_size)
{
	int j, ret = 0;
	void *fp = NULL;
	uint32_t fp_pos = 0, pos_line = 0;
	uint8_t *p = plane_addr[0];

	if (file_name == NULL || *plane_addr == NULL) {
		ret = -1;
		goto exit;
	}

	if ((fp = fopen(file_name[0], "rb")) == NULL) {
		printf("Can not find %s\n", file_name[0]);
		ret = -1;
		goto exit;
	}

	/* For pkm resource. */
	fp_pos = ftell((FILE *)fp);
	_vs_file_seek(fp, fp_pos + head_size, false);

	for (j = 0; j < bo_param[0].height; j++) {
		pos_line = ftell((FILE *)fp);

		fread(p, 1, bo_param[0].width * bo_param[0].bpp / 8, (FILE *)fp);
		p += plane_bo[0]->pitch;
		_vs_file_seek(fp, pos_line + bo_param[0].width * bo_param[0].bpp / 8, false);
	}

exit:
	if (fp)
		fclose((FILE *)fp);

	return ret;
}

static int _load_raw_area(char *file_name, uint8_t **plane_addr, drm_vs_bo_param *bo_param,
			  struct bo *plane_bo[4], uint32_t format, uint64_t mod)
{
	int ret = 0;
	const struct util_format_info *info = NULL;
	void *fp = NULL;
	uint32_t i = 0, j = 0, plane_num = 0;
	uint8_t *p = NULL;
	int h_tile[3] = { 1, 1, 1 };

	info = util_format_mod_info_find(format, mod);
	if (!info) {
		printf("failed to get format info");
		ret = -1;
		goto exit;
	}
	plane_num = info->num_planes;
	drm_vs_get_tile_height(format, mod, h_tile);

	if (file_name == NULL || *plane_addr == NULL) {
		ret = -1;
		goto exit;
	}

	if ((fp = fopen(file_name, "rb")) == NULL) {
		printf("Can not find %s\n", file_name);
		ret = -1;
		goto exit;
	}

	for (i = 0; i < plane_num; i++) {
		uint32_t pos_line = 0, pos_plane = 0;
		p = plane_addr[i];
		pos_plane = ftell((FILE *)fp);

		for (j = 0; j < bo_param[i].height; j += h_tile[i]) {
			pos_line = ftell((FILE *)fp);

			fread(p, 1, bo_param[i].width * h_tile[i] * bo_param[i].bpp / 8,
			      (FILE *)fp);
			p += plane_bo[i]->pitch * h_tile[i];
			_vs_file_seek(
				fp, pos_line + bo_param[i].width * h_tile[i] * bo_param[i].bpp / 8,
				false);
		}

		_vs_file_seek(fp,
			      pos_plane +
				      bo_param[i].width * bo_param[i].height * bo_param[i].bpp / 8,
			      false);
	}

exit:
	if (fp)
		fclose((FILE *)fp);

	return ret;
}

static int set_stream_offset(uint32_t width, uint32_t height, enum util_yuv_order yuv_order,
			     uint32_t frame_idx, uint32_t *offset)
{
	int ret = 0;

	switch (yuv_order) {
	case YUV_YCbCr:
	case YUV_YCrCb:
		*offset = frame_idx * (width * height + (width * height) / 2);
		break;
	default:
		*offset = 0;
		printf("Can not find yuv_order\n");
		ret = -1;
		goto exit;
	}

exit:
	return ret;
}

static int _load_stream_area(char *file_name, uint8_t **plane_addr, drm_vs_bo_param *bo_param,
			     struct bo *plane_bo[4], uint32_t format, uint64_t mod,
			     uint32_t frame_idx)
{
	int ret = 0;
	const struct util_format_info *info = NULL;
	void *fp = NULL;
	uint32_t i = 0, j = 0, plane_num = 0, offset = 0;
	uint8_t *p = NULL;
	int h_tile[3] = { 1, 1, 1 };

	info = util_format_mod_info_find(format, mod);
	if (!info) {
		printf("util_format_mod_info_find failed.\n");
		ret = -1;
		goto exit;
	}
	plane_num = info->num_planes;
	drm_vs_get_tile_height(format, mod, h_tile);

	if (file_name == NULL || *plane_addr == NULL) {
		ret = -1;
		goto exit;
	}
	if ((fp = fopen(file_name, "rb")) == NULL) {
		printf("Can not find %s\n", file_name);
		ret = -1;
		goto exit;
	}

	/* get offset from yuv file */
	if (set_stream_offset(bo_param[0].width, bo_param[0].height, info->yuv.order, frame_idx,
			      &offset)) {
		printf("Can not set stream offset\n");
		ret = -1;
		goto exit;
	}

	_vs_file_seek(fp, offset, false);

	/* load all streams data to plane_addr */
	for (i = 0; i < plane_num; i++) {
		uint32_t pos_line = 0, pos_plane = 0;
		p = plane_addr[i];
		pos_plane = ftell((FILE *)fp);

		for (j = 0; j < bo_param[i].height; j += h_tile[i]) {
			pos_line = ftell((FILE *)fp);

			fread(p, 1, bo_param[i].width * h_tile[i] * bo_param[i].bpp / 8,
			      (FILE *)fp);
			p += plane_bo[i]->pitch * h_tile[i];
			_vs_file_seek(
				fp, pos_line + bo_param[i].width * h_tile[i] * bo_param[i].bpp / 8,
				false);
		}

		_vs_file_seek(fp,
			      pos_plane +
				      bo_param[i].width * bo_param[i].height * bo_param[i].bpp / 8,
			      false);
	}

exit:
	if (fp)
		fclose((FILE *)fp);

	return ret;
}

static int _load_vimg_area(char *file_name, uint8_t **plane_addr, drm_vs_bo_param *bo_param,
			   struct bo *plane_bo[4], uint32_t format, uint64_t mod)
{
	int ret = 0;
	void *fp = NULL;
	const struct util_format_info *info = NULL;
	VIMG_FILEHEADER head = { { 0, 0, 0 }, 0 };
	VIMG_V1 img = { 0, 0, 0, 0, 0, 0 };
	uint32_t i = 0, j = 0, n = 0;
	uint32_t plane_num = 0;
	/* TODO: temporarily only for tile height = 1 */
	uint32_t tile_height = 1;

	/*	if (!is_rgb(format))
 *		tile_height = 2;
 */
	info = util_format_mod_info_find(format, mod);
	if (!info) {
		printf("failed to get format info");
		ret = -1;
		goto exit;
	}
	plane_num = info->num_planes;

	if ((fp = fopen(file_name, "rb")) == NULL) {
		printf("Can not find %s\n", file_name);
		ret = -1;
		goto exit;
	}

	n = fread(&head, 1, sizeof(head), (FILE *)fp);
	if (n < sizeof(head)) {
		ret = -1;
		goto exit;
	}

	if (strncmp((char *)head.magic, "VIV", 3)) {
		ret = -1;
		goto exit;
	}

	if (head.version == 1) {
		img.format = _vs_file_read_dword(fp);
		img.tiling = _vs_file_read_dword(fp);
		img.imageStride = _vs_file_read_dword(fp);
		img.imageWidth = _vs_file_read_dword(fp);
		img.imageHeight = _vs_file_read_dword(fp);
		img.bitsOffset = _vs_file_read_dword(fp);
	} else {
		ret = -1;
		goto exit;
	}

	_vs_file_seek(fp, img.bitsOffset, false);

	for (j = 0; j < plane_num; j++) {
		uint32_t width = 0, height = 0;
		uint32_t pos_line = 0, pos_plane = 0;
		uint8_t *p = plane_addr[j];

		width = bo_param[j].width;
		height = bo_param[j].height;

		pos_plane = ftell((FILE *)fp);

		for (i = 0; i < height; i += tile_height) {
			pos_line = ftell((FILE *)fp);

			fread(p, 1, width * tile_height * bo_param[j].bpp / 8, fp);
			p += plane_bo[j]->pitch * tile_height;
			_vs_file_seek(fp,
				      pos_line +
					      bo_param[j].width * tile_height * bo_param[j].bpp / 8,
				      false);
		}

		_vs_file_seek(fp,
			      pos_plane +
				      bo_param[j].width * bo_param[j].height * bo_param[j].bpp / 8,
			      false);
	}

exit:
	if (fp)
		fclose((FILE *)fp);

	return ret;
}

static int _load_tga_area(char *file_name, uint8_t **plane_addr, drm_vs_bo_param *bo_param,
			  struct bo *plane_bo[4], uint32_t format)
{
	int ret = 0;
	void *fp = NULL;
	uint8_t *p = NULL;
	uint32_t bpps[3] = { 0 };
	uint32_t img_width = 0, img_height = 0, pos_line = 0, j = 0;
	uint8_t channelCount = 3;
	uint8_t tgaHeader[18];

	if (file_name == NULL || *plane_addr == NULL) {
		ret = -1;
		goto exit;
	}

	if ((fp = fopen(file_name, "rb")) == NULL) {
		printf("Can not find %s\n", file_name);
		ret = -1;
		goto exit;
	}

	fread(tgaHeader, 1, 18, fp);

	img_width = ((tgaHeader[13] << 8) | tgaHeader[12]);
	img_height = ((tgaHeader[15] << 8) | tgaHeader[14]);
	bpps[0] = tgaHeader[16];
	channelCount = (uint8_t)(bpps[0] / 8);

	p = plane_addr[0];
	for (j = 0; j < img_height; j++) {
		pos_line = ftell((FILE *)fp);

		fread(p, 1, img_width * channelCount, fp);
		p += plane_bo[0]->pitch;
		_vs_file_seek(fp, pos_line + img_width * channelCount, false);
	}

exit:
	if (fp)
		fclose((FILE *)fp);

	return ret;
}

static int _load_source(const char *filename, void *addr, uint32_t size)
{
	FILE *file;
	int ret = 0;
	size_t cnt;

	file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "file [%s] open failed\n", filename);
		ret = errno;
		return ret;
	}

	cnt = fread(addr, 1, size, file);
	(void)cnt;

	fclose(file);

	return 0;
}

static int vs_load_pvric_mask(const char *filename, uint32_t format, bool lossy,
			      uint32_t header_size, uint32_t offset, struct bo *bo, void *va_addr,
			      bool compressed)
{
	uint32_t tile_cnt = header_size;
	uint32_t src_size, ret = 0;

	if (!compressed)
		src_size = bo->size - 256;
	else {
		if (!lossy)
			src_size = header_size + tile_cnt * 256;
		else if (format == DRM_FORMAT_P010)
			src_size = header_size + tile_cnt * 96;
		else
			src_size = header_size + tile_cnt * 128;
	}

	ret = _load_source(filename, (uint8_t *)((uint64_t)va_addr + offset - header_size),
			   src_size);
	if (ret)
		fprintf(stderr, "load image %s failed with %d\n", filename, ret);
	return ret;
}

static int get_filename_ext_pos(const char *filename)
{
	uint32_t len = 0, ext_pos = 0, ext_len = 0;

	len = strlen(filename);
	ext_pos = len - 1;
	ext_len = 0;

	while (ext_pos) {
		if (filename[ext_pos] == '.')
			break;

		ext_len++;
		ext_pos--;
	}

	if (ext_pos == 0)
		return -1;

	if (ext_len > 4) {
		printf("File format not supported.\n");
		return -1;
	}

	return ext_pos;
}

static int vs_plane_fill(char (*filename)[RESOURCE_LEN + 1], uint32_t format, uint64_t mod,
			 void *va_addr[3], drm_vs_bo_param bo_param[4], struct bo *plane_bo[4],
			 uint64_t physical[3], uint32_t frame_idx)
{
	const struct util_format_info *info = NULL;
	char file_src[4][RESOURCE_LEN + 10];
	uint32_t tile_cnt = 0, src_size = 0, ext_pos = 0, head_size = 0;
	uint8_t i, num_planes = 0;
	int ret = 0;
	bool lossy;
	char tmp_name[RESOURCE_LEN + 10];
	info = util_format_mod_info_find(format, mod);

	if (!info) {
		fprintf(stderr, "can't get format info\n");
		return -1;
	}
	num_planes = info->num_planes;

	for (i = 0; i < 4; i++)
		memset(file_src + i, '\0', sizeof(char) * (RESOURCE_LEN + 4));

	if (fourcc_mod_vs_is_normal(mod)) {
		sprintf(file_src[0], "resource/%s", filename[0]);

		ext_pos = get_filename_ext_pos(file_src[0]);
		if (ext_pos <= 0)
			return -1;

		if (!strcmp(&file_src[0][ext_pos], ".bmp")) {
			ret = _load_bmp_area(file_src[0], (uint8_t *)va_addr[0], bo_param[0],
					     plane_bo[0], format, NULL);
			if (ret) {
				printf("upload bmp resource file to buffer failed. \n");
				return ret;
			}
		} else if (!strcmp(&file_src[0][ext_pos], ".raw") ||
			   !strcmp(&file_src[0][ext_pos], ".bin")) {
			ret = _load_raw_area(file_src[0], (uint8_t **)va_addr, bo_param, plane_bo,
					     format, mod);
			if (ret) {
				printf("upload raw resource file to buffer failed. \n");
				return ret;
			}
		} else if (!strcmp(&file_src[0][ext_pos], ".yuv")) {
			ret = _load_stream_area(file_src[0], (uint8_t **)va_addr, bo_param,
						plane_bo, format, mod, frame_idx);
			if (ret) {
				printf("upload stream resource file to buffer failed. \n");
				return ret;
			}
		} else if (!strcmp(&file_src[0][ext_pos], ".vimg")) {
			ret = _load_vimg_area(file_src[0], (uint8_t **)va_addr, bo_param, plane_bo,
					      format, mod);
			if (ret) {
				printf("upload vimg resource file to buffer failed. \n");
				return ret;
			}
		} else if (!strcmp(&file_src[0][ext_pos], ".tga")) {
			ret = _load_tga_area(file_src[0], (uint8_t **)va_addr, bo_param, plane_bo,
					     format);
			if (ret) {
				printf("upload tga resource file to buffer failed. \n");
				return ret;
			}
		} else {
			printf("File format is not supported for now.\n");
			return -1;
		}
#if CONFIG_VERISILICON_DEBUG
		vs_dump_layer_source(plane_bo, physical, (uint32_t **)va_addr, num_planes, bo_param,
				     mod, format, file_src[0], FINAL_PLANE);
#endif
	} else if (fourcc_mod_vs_is_pvric(mod)) {
		if (filename[0] == NULL) {
			fprintf(stderr, "Invalid filename for fbc fill\n");
			return -1;
		}

		sprintf(tmp_name, "resource/%s", filename[0]);
		ret = _fill_fbc_filename(tmp_name, file_src[0], file_src[1], num_planes);
		if (ret)
			return ret;

		if (mod & DRM_FORMAT_MOD_VS_DEC_LOSSY)
			lossy = true;
		else
			lossy = false;

		for (i = 0; i < num_planes; i++) {
			tile_cnt = bo_param[i].header_size;
			if (!lossy)
				src_size = bo_param[i].header_size + tile_cnt * 256;
			else if (format == DRM_FORMAT_P010)
				src_size = bo_param[i].header_size + tile_cnt * 96;
			else
				src_size = bo_param[i].header_size + tile_cnt * 128;

			ret = _load_fbc_area(file_src[i], va_addr[i], src_size,
					     bo_param[i].header_size, plane_bo[i]->offset);
			if (ret)
				return ret;
		}
#if CONFIG_VERISILICON_DEBUG
		vs_dump_layer_source(plane_bo, physical, (uint32_t **)va_addr, num_planes, bo_param,
				     mod, format, file_src[0], FINAL_PLANE);
#endif
	} else if (fourcc_mod_vs_is_compressed(mod)) {
		if (filename[0] == NULL) {
			fprintf(stderr, "Invalid filename for fbc fill\n");
			return -1;
		}

		sprintf(file_src[0], "resource/%s", filename[0]);

		ext_pos = get_filename_ext_pos(file_src[0]);
		if (ext_pos <= 0)
			return -1;

		if (!strcmp(&file_src[0][ext_pos], ".bin")) {
#if CONFIG_VERISILICON_DEC400_CONFORMANCE_TEST
			printf("DEC400: decompression start!! read compressed bin file.\n");
#endif
			ret = _load_dec_raw_area_from_bin(file_src, (uint8_t **)va_addr, bo_param,
							  plane_bo, format, mod);

		} else if (!strcmp(&file_src[0][ext_pos], ".raw")) {
			/* for tile status buffer filename */
			for (i = 0; i < num_planes; i++) {
				sprintf(file_src[i + 1], "resource/%s", filename[i + 1]);
			}

			ret = _load_dec_raw_area(file_src, (uint8_t **)va_addr, bo_param, plane_bo,
						 format, mod);
		} else {
			printf("DEC400: DEC400 File format is not supported for now.\n");
			ret = -1;
		}

		if (ret) {
			printf("upload dec raw resource file to buffer failed. \n");
			return ret;
		}

#if CONFIG_VERISILICON_DEBUG
		vs_dump_layer_source(plane_bo, physical, (uint32_t **)va_addr, num_planes, bo_param,
				     mod, format, file_src[0], FINAL_PLANE);
#endif

	} else if (fourcc_mod_vs_is_dec400a(mod)) {
		if (filename[0] == NULL) {
			fprintf(stderr, "Invalid filename for fbc fill\n");
			return -1;
		}

		snprintf(file_src[0], sizeof(file_src[0]), "resource/%s", filename[0]);

		ret = _load_dec400a_raw_area(file_src, (uint8_t **)va_addr, bo_param, plane_bo,
					     format);
		if (ret) {
			printf("upload dec400A raw resource file to buffer failed.\n");
			return ret;
		}
#if CONFIG_VERISILICON_DEBUG
		vs_dump_layer_source(plane_bo, physical, (uint32_t **)va_addr, num_planes, bo_param,
				     mod, format, file_src[0], FINAL_PLANE);
#endif
	} else if (fourcc_mod_vs_is_decnano(mod) || fourcc_mod_vs_is_etc2(mod)) {
		sprintf(file_src[0], "resource/%s", filename[0]);

		ext_pos = get_filename_ext_pos(file_src[0]);
		if (ext_pos <= 0)
			return -1;

		if (!strcmp(&file_src[0][ext_pos], ".pkm"))
			head_size = 16;

		ret = _load_decnano_etc2_raw_area(file_src, (uint8_t **)va_addr, bo_param, plane_bo,
						  format, head_size);
		if (ret) {
			printf("upload decnano/etc2 raw resource file to buffer failed.\n");
			return ret;
		}
#if CONFIG_VERISILICON_DEBUG
		vs_dump_layer_source(plane_bo, physical, (uint32_t **)va_addr, num_planes, bo_param,
				     mod, format, file_src[0], FINAL_PLANE);
#endif
	}
	return 0;
}

/* -----------------------------------------------------------------------------
 * dump post wb content
 */

/* remap yuv444 packed format to 3 planer format */
static int __modify_wb_yuv444_10bit(struct bo *bo, drm_vs_bo_param *bo_param)
{
	uint8_t *y_plane = NULL, *u_plane = NULL, *v_plane = NULL;
	uint32_t cur_pixel = 0, row = 0, col = 0;
	uint32_t dest_width = bo->width;
	uint32_t dest_height = bo->height;
	int ret = DTEST_SUCCESS;
	uint32_t *temp_buf = (uint32_t *)malloc(bo->size);
	if (!temp_buf) {
		printf("%s: failed to create temp buf\n", __func__);
		return DTEST_FAILURE;
	}
	memcpy(temp_buf, bo->ptr, bo->size);
	y_plane = (uint8_t *)bo->ptr;
	u_plane = (uint8_t *)bo->ptr + dest_width * dest_height;
	v_plane = (uint8_t *)bo->ptr + dest_width * dest_height * 2;
	for (row = 0; row < dest_height; row++) {
		for (col = 0; col < dest_width; col++) {
			cur_pixel = *(temp_buf + row * dest_width + col);
			*(y_plane++) = (cur_pixel >> 22) & 0xFF;
			*(u_plane++) = (cur_pixel >> 12) & 0xFF;
			*(v_plane++) = (cur_pixel >> 2) & 0xFF;
		}
	}
	free(temp_buf);

	/* modify bo pitch to width * 3, when num_planes is still equal to 1.
	 * so ensure that the dumped bo size is w * h * 3.
	 */
	bo_param->bpp = 24;
	bo->pitch = dest_width * 24 / 8;

	return ret;
}

/* remap dp rgb888 48-bit format to xrgb8888 32-bit format */
static void __modify_dp_rgb888(struct bo *bo)
{
	uint64_t currentPixel = 0, *ptr64 = NULL;
	uint32_t row, col, *temp_buf = NULL;

	temp_buf = (uint32_t *)calloc(1, bo->size);
	if (!temp_buf) {
		printf("calloc for temp buffer failed.\n");
		return;
	}
	ptr64 = (uint64_t *)bo->ptr;
	for (row = 0; row < bo->height; row++) {
		for (col = 0; col < bo->width; col++) {
			currentPixel = *(ptr64 + row * bo->width + col);
			*(temp_buf + row * bo->width + col) =
				(((currentPixel >> 40) & 0xFF) << 16 |
				 ((currentPixel >> 24) & 0xFF) << 8 | ((currentPixel >> 8) & 0xFF));
		}
	}

	memcpy(bo->ptr, temp_buf, bo->size);
	bo->pitch /= 2;

	free(temp_buf);
	return;
}

static int _write_dec_raw_area(char (*file_name)[RESOURCE_LEN + 4], uint8_t **plane_addr,
			       drm_vs_bo_param bo_param[4], struct bo *plane_bo[4],
			       uint32_t num_planes, uint32_t format, uint64_t mod)
{
	int ret = 0;
	void *fp = NULL, *ts_fp[3] = { NULL, NULL, NULL };
	uint32_t i = 0, j = 0;
	uint8_t *p = NULL;
	int h_tile[3] = { 1, 1, 1 };

	if (file_name == NULL || *plane_addr == NULL) {
		ret = -1;
		goto exit;
	}

	if ((fp = fopen(file_name[0], "wb")) == NULL) {
		printf("Can not find %s\n", file_name[0]);
		ret = -1;
		goto exit;
	}

	for (i = 0; i < num_planes; i++) {
		if ((ts_fp[i] = fopen(file_name[i + 1], "wb")) == NULL) {
			printf("Can not find %s\n", file_name[i + 1]);
			ret = -1;
			goto exit;
		}
	}

	drm_vs_get_tile_height(format, mod, h_tile);

	for (i = 0; i < num_planes; i++) {
		uint32_t pos_line = 0, pos_plane = 0, ts_buf_height = 0;
		p = plane_addr[i];
		pos_plane = ftell((FILE *)fp);
		ts_buf_height = bo_param[i].ts_buf_size / (bo_param[i].width * bo_param[i].bpp / 8);

		for (j = 0; j < bo_param[i].height - ts_buf_height; j += h_tile[i]) {
			pos_line = ftell((FILE *)fp);

			fwrite(p, 1, bo_param[i].width * h_tile[i] * bo_param[i].bpp / 8,
			       (FILE *)fp);
			p += plane_bo[i]->pitch * h_tile[i];
			_vs_file_seek(
				fp, pos_line + bo_param[i].width * h_tile[i] * bo_param[i].bpp / 8,
				false);
		}

		/* write tile status buffer behind image plane buffer for DEC400 */
		fwrite(p, 1, bo_param[i].ts_buf_size, (FILE *)ts_fp[i]);

		_vs_file_seek(fp,
			      pos_plane + bo_param[i].width * (bo_param[i].height - ts_buf_height) *
						  bo_param[i].bpp / 8,
			      false);
	}

exit:
	if (fp)
		fclose((FILE *)fp);

	for (i = 0; i < num_planes; i++) {
		if (ts_fp[i])
			fclose((FILE *)ts_fp[i]);
	}

	return ret;
}

static int _write_fbc_raw_area(char *filename, void *va_addr, uint32_t src_size,
			       uint32_t header_size, uint64_t offset)
{
	FILE *file = NULL;
	uint8_t *base_addr = NULL;
	int ret = 0;
	size_t cnt;

	if (filename == NULL || va_addr == NULL) {
		ret = -1;
		goto exit;
	}

	if ((file = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "file [%s] open failed\n", filename);
		ret = -1;
		goto exit;
	}

	base_addr = (uint8_t *)((uint64_t)va_addr + offset - header_size);

	cnt = fwrite(base_addr, 1, src_size, file);
	(void)cnt;

	fclose(file);

	return 0;

exit:
	if (file)
		fclose(file);
	return ret;
}

static int _write_dec_raw_area_to_bin(char (*file_name)[RESOURCE_LEN + 4], uint8_t **plane_addr,
				      drm_vs_bo_param bo_param[4], struct bo *plane_bo[4],
				      uint32_t num_planes, uint32_t format, uint64_t mod)
{
	int ret = 0;
	void *fp = NULL;
	uint32_t i = 0, j = 0;
	uint8_t *p = NULL;
	int h_tile[3] = { 1, 1, 1 };
	uint64_t modifiers[4] = { 0 };

	if (file_name == NULL || *plane_addr == NULL) {
		ret = -1;
		goto exit;
	}

	if ((fp = fopen(file_name[0], "wb")) == NULL) {
		printf("Can not find %s\n", file_name[0]);
		ret = -1;
		goto exit;
	}

	drm_vs_get_tile_height(format, mod, h_tile);

	if (vs_mod_config(format, mod, num_planes, modifiers) < 0) {
		ret = -1;
		goto exit;
	}

	for (i = 0; i < num_planes; i++) {
		uint32_t pos_line = 0, pos_plane = 0, ts_buf_height = 0;
		uint32_t tile_num = 0, pos_ts_base = 0, data_size = 0, data_tile_size = 0;
		uint32_t ts_buf_offset = 0, ts_size = 0, ts_size_memory = 0;
		double ts_size_double = 0.f;
		uint8_t ts_bits = TILESTATUS_BITS;
		uint32_t header_size = DEC_HEADER_SIZE;

		if (dec_mod_is_fc(mod))
			header_size = DEC_HEADER_SIZE + DEC_HEADER_SIZE * dec_mod_get_fc_size(mod);

		p = plane_addr[i];
		pos_plane = ftell((FILE *)fp);
		ts_buf_height = bo_param[i].ts_buf_size / (bo_param[i].width * bo_param[i].bpp / 8);

		data_size = bo_param[i].width * (bo_param[i].height - ts_buf_height) *
			    bo_param[i].bpp / 8;
		data_tile_size = vs_get_dec_tile_size(fourcc_mod_vs_get_tile_mode(modifiers[i]),
						      bo_param[i].bpp);
		pos_ts_base = pos_plane + header_size;
		ts_buf_offset = pos_ts_base % data_tile_size;
		tile_num = data_size / data_tile_size;
		ts_size_double = tile_num * ts_bits / 8;
		ts_size = (uint32_t)ceil(ts_size_double);
		ts_size_memory = UP_ALIGN(ts_buf_offset + ts_size, data_tile_size) - ts_buf_offset;
#if CONFIG_VERISILICON_DEC400_CONFORMANCE_TEST
		printf("DEC400: ts_size_memory: %d  bo_param[i].ts_buf_size: %d \n", ts_size_memory,
		       bo_param[i].ts_buf_size);
		printf("DEC400: Header start: %d  ts start: %d data start: %d \n", pos_plane,
		       pos_plane + header_size, pos_plane + header_size + ts_size_memory);
		printf("DEC400: bpp: %d  data_size: %d data_tile_size: %d \n", bo_param[i].bpp,
		       data_size, data_tile_size);
#endif

		/* write header buffer for DEC400, the fast clear is not
		   supported for dec400 compression for now, just write 0 */
		for (j = 0; j < header_size; j++) {
			fputc(0x0, (FILE *)fp);
		}

		_vs_file_seek(fp, pos_plane + header_size + ts_size_memory, false);
		/* write data buffer for DEC400 */
		for (j = 0; j < bo_param[i].height - ts_buf_height; j += h_tile[i]) {
			pos_line = ftell((FILE *)fp);
			fwrite(p, 1, bo_param[i].width * h_tile[i] * bo_param[i].bpp / 8,
			       (FILE *)fp);
			p += plane_bo[i]->pitch * h_tile[i];
			_vs_file_seek(
				fp, pos_line + bo_param[i].width * h_tile[i] * bo_param[i].bpp / 8,
				false);
		}

		/* write tile status buffer behind image plane buffer for DEC400 */
		_vs_file_seek(fp, pos_plane + header_size, false);
		fwrite(p, 1, ts_size_memory, (FILE *)fp);

		_vs_file_seek(fp, pos_plane + header_size + ts_size_memory + data_size, false);
	}

exit:
	if (fp)
		fclose((FILE *)fp);

	return ret;
}

static dtest_status __write_raw_area(const char *filename, uint8_t **plane_addr, struct bo *bo[4],
				     drm_vs_bo_param bo_param[4], uint32_t num_planes,
				     uint32_t format, uint64_t mod)
{
	uint32_t i, j;
	FILE *fp = NULL;
	uint8_t *p = NULL;
	int ret = 0;
	int h_tile[3] = { 1, 1, 1 };

	fp = fopen(filename, "wb");
	if (NULL == fp) {
		DTESTLOGE("Open %s fail", filename);
		ret = DTEST_FAILURE;
		goto exit;
	}

	drm_vs_get_tile_height(format, mod, h_tile);

	for (i = 0; i < num_planes; i++) {
		uint32_t pos_line = 0, pos_plane = 0;
		p = (uint8_t *)plane_addr[i];
		pos_plane = ftell((FILE *)fp);

		for (j = 0; j < bo_param[i].height; j += h_tile[i]) {
			pos_line = ftell((FILE *)fp);

			fwrite(p, 1, bo_param[i].width * h_tile[i] * bo_param[i].bpp / 8,
			       (FILE *)fp);
			p += bo[i]->pitch * h_tile[i];
			_vs_file_seek(
				fp, pos_line + bo_param[i].width * h_tile[i] * bo_param[i].bpp / 8,
				false);
		}

		_vs_file_seek(fp,
			      pos_plane +
				      bo_param[i].width * bo_param[i].bpp / 8 * bo_param[i].height,
			      false);
	}

exit:
	if (fp)
		fclose(fp);
	return ret;
}

static dtest_status __write_stream_raw_area(const char *filename, FILE *stream_fp,
					    uint32_t pos_frame, uint8_t **plane_addr,
					    struct bo *bo[4], drm_vs_bo_param bo_param[4],
					    uint32_t num_planes, uint32_t format, uint64_t mod)
{
	uint32_t i, j;
	FILE *fp = NULL;
	uint8_t *p = NULL;
	int ret = 0;
	int h_tile[3] = { 1, 1, 1 };
	bool dump_frame = true;
	uint32_t pos_line = 0;
	if (stream_fp) {
#ifndef CONFIG_VERISILICON_STREAM_DEBUG
		dump_frame = false;
#endif
	}

	if (dump_frame) {
		fp = fopen(filename, "wb");
		if (NULL == fp) {
			DTESTLOGE("Open %s fail", filename);
			ret = DTEST_FAILURE;
			goto exit;
		}
	}
	drm_vs_get_tile_height(format, mod, h_tile);

	for (i = 0; i < num_planes; i++) {
		if (fp) {
			pos_line = 0;
		}

		p = (uint8_t *)plane_addr[i];

		for (j = 0; j < bo_param[i].height; j += h_tile[i]) {
			if (fp)
				pos_line = ftell((FILE *)fp);

			if (fp)
				fwrite(p, 1, bo_param[i].width * h_tile[i] * bo_param[i].bpp / 8,
				       (FILE *)fp);
			if (stream_fp)
				fwrite(p, 1, bo_param[i].width * h_tile[i] * bo_param[i].bpp / 8,
				       (FILE *)stream_fp);

			p += bo[i]->pitch * h_tile[i];

			if (fp) {
				_vs_file_seek(fp,
					      pos_line + bo_param[i].width * h_tile[i] *
								 bo_param[i].bpp / 8,
					      false);
			}

			if (stream_fp)
				_vs_file_seek(stream_fp,
					      pos_frame + pos_line +
						      bo_param[i].width * h_tile[i] *
							      bo_param[i].bpp / 8,
					      false);
			if (!fp)
				pos_line += bo_param[i].width * h_tile[i] * bo_param[i].bpp / 8;
		}
	}

exit:
	if (fp)
		fclose(fp);
	return ret;
}

dtest_status dtest_dump_stream_raw_data(void *va_addr[4], pipe_t *pipe, const char *filename,
					FILE *stream_fp, uint32_t pos_frame)
{
	dtest_status ret = DTEST_SUCCESS;
	struct bo **bo = pipe->out_bo;
	drm_vs_bo_param *bo_param = pipe->bo_param;
	uint32_t num_planes = pipe->num_planes;

	ret = __write_stream_raw_area(filename, stream_fp, pos_frame, (uint8_t **)va_addr, bo,
				      bo_param, num_planes, pipe->fourcc, pipe->wb_modifier);

	if (ret != DTEST_SUCCESS) {
		DTESTLOGI("Dump raw file %s fail.", filename);
		return ret;
	}

	return ret;
}

void dtest_extract_filename(const char *filename, char *result)
{
	const char *dot = strrchr(filename, '.');

	if (dot != NULL) {
		size_t len = dot - filename;
		strncpy(result, filename, len);
		result[len] = '\0';
	} else {
		strcpy(result, filename);
		printf("extract_filename fail.");
	}
}

dtest_status dtest_dump_raw_data(void *va_addr[4], pipe_t *pipe, const char *filename)
{
	struct bo **bo = pipe->out_bo;
	dtest_status ret = DTEST_SUCCESS;
	uint32_t num_planes = pipe->num_planes, ext_pos = 0;
	uint64_t mod = pipe->wb_modifier;
	char file_src[4][RESOURCE_LEN + 4] = { 0 };
	drm_vs_bo_param *bo_param = pipe->bo_param;
	bool lossy;
	uint8_t i;
	uint32_t tile_cnt = 0, src_size = 0;
	char temp_filename[256];

#if 1 // for debug
	uint32_t n;
	DTESTLOGI("Raw num_planes=%u, vdisplay=%u, hdislay=%u", num_planes, pipe->wb_h, pipe->wb_w);
	for (n = 0; n < num_planes; n++) {
		DTESTLOGI("Raw raw_height[%u]=%u, width：%u, stride: %ld, bpp: %u", n,
			  bo[n]->height, bo[n]->width, bo[n]->pitch, bo_param[n].bpp);
	}
#endif

	if (fourcc_mod_vs_is_normal(mod)) {
		if (pipe->wb_yuv444_10bit) {
			ret = __modify_wb_yuv444_10bit(bo[0], &bo_param[0]);
			if (ret != DTEST_SUCCESS) {
				DTESTLOGE("modify yuv444 10bit for %s fail.", filename);
				return ret;
			}
		}

		ret = __write_raw_area(filename, (uint8_t **)va_addr, bo, bo_param, num_planes,
				       pipe->fourcc, mod);
		if (ret != DTEST_SUCCESS) {
			DTESTLOGI("Dump raw file %s fail.", filename);
			return ret;
		}
	} else if (fourcc_mod_vs_is_compressed(mod)) {
		if (filename == NULL) {
			fprintf(stderr, "Invalid filename for fbc fill\n");
			return -1;
		}

		sprintf(file_src[0], "%s", filename);

		ext_pos = get_filename_ext_pos(file_src[0]);
		if (ext_pos <= 0)
			return -1;

		if (!strcmp(&file_src[0][ext_pos], ".bin")) {
#if CONFIG_VERISILICON_DEC400_CONFORMANCE_TEST
			printf("DEC400: compression complete!! start dump compressed BIN file %s.\n",
			       filename);
#endif
			ret = _write_dec_raw_area_to_bin(file_src, (uint8_t **)va_addr, bo_param,
							 bo, num_planes, pipe->fourcc, mod);
		} else if (!strcmp(&file_src[0][ext_pos], ".raw")) {
			/* for tile status buffer filename */
			if (num_planes == 1) {
				sprintf(file_src[1], "%s.ts", filename);
			} else if (num_planes == 2) {
				sprintf(file_src[1], "%s.y.ts", filename);
				sprintf(file_src[2], "%s.uv.ts", filename);
			} else if (num_planes == 3) {
				sprintf(file_src[1], "%s.y.ts", filename);
				sprintf(file_src[2], "%s.u.ts", filename);
				sprintf(file_src[3], "%s.v.ts", filename);
			}

			ret = _write_dec_raw_area(file_src, (uint8_t **)va_addr, bo_param, bo,
						  num_planes, pipe->fourcc, mod);

		} else {
			printf("DEC400: DEC400 File format is not supported for now.\n");
			ret = -1;
		}

		if (ret) {
			printf("dump dec raw resource file to buffer failed. \n");
			return ret;
		}
	} else if (fourcc_mod_vs_is_pvric(mod)) {
		if (filename == NULL) {
			fprintf(stderr, "Invalid filename for pvric fill\n");
			return -1;
		}

		if (mod & DRM_FORMAT_MOD_VS_DEC_LOSSY)
			lossy = true;
		else
			lossy = false;

		dtest_extract_filename(filename, temp_filename);

		if (num_planes == 1) {
			sprintf(file_src[0], "%s", filename);
		} else if (num_planes == 2) {
			sprintf(file_src[0], "%s", filename);
			sprintf(file_src[1], "%s_uv.raw", temp_filename);
		}

		for (i = 0; i < num_planes; i++) {
			tile_cnt = bo_param[i].header_size;
			if (!lossy)
				src_size = bo_param[i].header_size + tile_cnt * 256;
			else if (pipe->fourcc == DRM_FORMAT_P010)
				src_size = bo_param[i].header_size + tile_cnt * 96;
			else
				src_size = bo_param[i].header_size + tile_cnt * 128;

			ret = _write_fbc_raw_area(file_src[i], va_addr[i], src_size,
						  bo_param[i].header_size, bo[i]->offset);
			if (ret) {
				printf("dump pvric raw resource file to buffer failed. \n");
				return ret;
			}
		}
	}

	DTESTLOGI("Dump raw file %s success.", filename);
	return ret;
}

#if CONFIG_VERISILICON_MD5_CHECK
void vs_fill_sprint(char *str, char *message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	vsprintf(str, message, arguments);
	va_end(arguments);
}

vs_status vs_calcu_md5(const char *filename, uint8_t md5_hash[MD5_DIGEST_LEN])
{
	vs_status ret = VS_STATUS_OK;
	unsigned char buffer[1024];
	size_t bytesRead = 0;

	FILE *file = fopen(filename, "rb");
	if (file == NULL) {
		printf("file to calculate md5 can't be open.\n");
		ret = VS_STATUS_FAILED;
		return ret;
	}

	MD5_CTX ctx;
	MD5_Init(&ctx);

	while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
		MD5_Update(&ctx, buffer, bytesRead);
	}

	MD5_Final(md5_hash, &ctx);

	if (file)
		fclose(file);
	file = NULL;

	return ret;
}

dtest_status vs_check_md5(const char *md5_result, const char *golden, char *wb_id)
{
	if (!md5_result || !golden)
		return DTEST_FAILURE;
	if (strcmp(md5_result, golden)) {
		printf("Fail: the wb%s result: %s cannot match the golden: %s!\n", wb_id,
		       md5_result, golden);
		return DTEST_FAILURE;
	}
	printf("Pass: the wb%s result: %s match the golden: %s!\n", wb_id, md5_result, golden);
	return DTEST_SUCCESS;
}
#endif
dtest_status bo_dump(pipe_t *pipe, const char *filename, FILE *stream_fp, uint32_t pos_frame,
		     bool save_bin, bool check)
{
	dtest_status status;
	void *va_addr[4] = { NULL };
	struct bo **bo = pipe->out_bo;
	char fullname[512] = { 0 };
	bool is_linear = false, is_rgb888 = false;
	uint32_t i = 0;
	int ret = 0;
	bool save_raw = false;
#if CONFIG_VERISILICON_MD5_CHECK
	uint8_t md5_hash[MD5_DIGEST_LEN];
#endif

	if (!bo || !filename) {
		DTESTLOGE("Invalid parameter.");
		return DTEST_FAILURE;
	}
	memset(fullname, 0, sizeof(fullname));

	for (i = 0; i < pipe->num_planes; i++) {
		ret = bo_map(bo[i], &va_addr[i]);
		if (ret) {
			DTESTLOGE("failed to map buffer: %s", strerror(-errno));
			bo_destroy(bo[i]);
			return -1;
		}
	}

	/* save rgb888 24bpp format in raw form for better review */
	is_rgb888 = (pipe->fourcc == DRM_FORMAT_RGB888) || (pipe->fourcc == DRM_FORMAT_BGR888);

	is_linear = (pipe->wb_modifier & DRM_FORMAT_MOD_VS_NORM_MODE_MASK) ==
				    DRM_FORMAT_MOD_VS_LINEAR ?
				  true :
				  false;
	save_raw = (fourcc_mod_is_custom_format(pipe->wb_modifier)) | is_rgb888 | (!is_linear) |
		   (pipe->wb_yuv444_10bit) | (!is_rgb(pipe->fourcc));

	if (pipe->fourcc == DRM_FORMAT_XRGB16161616F)
		__modify_dp_rgb888(bo[0]);

	if (pipe->stream) {
		char rawname[512] = { 0 };
		memset(rawname, 0, sizeof(rawname));
		snprintf(rawname, sizeof(rawname), "%s.raw", filename);

		status = dtest_dump_stream_raw_data(va_addr, pipe, rawname, stream_fp, pos_frame);
	} else {
#if CONFIG_VERISILICON_DEC400_CONFORMANCE_TEST
		save_bin = 1;
#endif
		if (!save_bin) {
			if (save_raw) {
				snprintf(fullname, sizeof(fullname), "%s.raw", filename);
				status = dtest_dump_raw_data(va_addr, pipe, fullname);
			} else {
				snprintf(fullname, sizeof(fullname), "%s.bmp", filename);
				status = vs_save_dib(va_addr[0], pipe, bo[0]->pitch, bo[0]->width,
						     bo[0]->height, fullname);
			}
		} else {
			snprintf(fullname, sizeof(fullname), "%s.bin", filename);
			status = dtest_dump_raw_data(va_addr, pipe, fullname);
		}
	}
#if CONFIG_VERISILICON_MD5_CHECK
	if (check) {
		status = vs_calcu_md5(fullname, md5_hash);
		pipe->md5_result = malloc(MD5_BUFFER_SIZE);
		memset(pipe->md5_result, 0, MD5_BUFFER_SIZE);
		for (int i = 0; i < MD5_DIGEST_LEN; i++) {
			vs_fill_sprint(&pipe->md5_result[i * 2], "%02x", md5_hash[i]);
		}
		status = vs_check_md5(pipe->md5_result, pipe->md5_golden, pipe->cons[0]);
	}
#endif
	for (i = 0; i < pipe->num_planes; i++) {
		bo_unmap(bo[i]);
	}
	return status;
}

static int _query_gem_physical(int fd, uint32_t handle, uint64_t *physical)
{
	struct drm_vs_gem_query_info info = { 0 };
	int ret = 0;
	info.handle = handle;
	info.type = VS_GEM_QUERY_HANDLE;
	ret = drmIoctl(fd, DRM_IOCTL_VS_GEM_QUERY, &info);
	if (!ret)
		*physical = info.data;
	return ret;
}

/* -----------------------------------------------------------------------------
 * vs mask bo create
 */
int vs_mask_bo_create(int fd, unsigned int format, uint64_t mod, unsigned int width,
		      unsigned int height, const char *filename, enum util_fill_pattern pattern,
		      struct bo *pbo[4], bool compressed)
{
	drm_vs_bo_param bo_param = { 0 };
	struct bo *bo;
	int ret = 0;
	uint32_t offset = 0;
	void *va_addr = NULL;
	uint64_t physical = 0;

	if (!filename) {
		fprintf(stderr, "No mask file provided.\n");
		return -1;
	}

	bo_param.width = width;
	bo_param.height = height;
	bo_param.bpp = 8;
	drm_vs_get_align_size(&bo_param.width, &bo_param.height, format, mod);
	drm_vs_calibrate_bo_size(&bo_param, mod, format);
	bo = bo_create_dumb(fd, bo_param.width, bo_param.height, bo_param.bpp);
	if (!bo) {
		fprintf(stderr, "failed to create dumb: %s\n", strerror(-errno));
		return -1;
	}
	ret = bo_map(bo, &va_addr);
	if (ret) {
		fprintf(stderr, "failed to map buffer: %s\n", strerror(-errno));
		bo_destroy(bo);
		return -1;
	}

	if (compressed) {
		ret = _query_gem_physical(fd, bo->handle, &physical);
		if (ret) {
			fprintf(stderr, "Query buffer pysical fail.\n");
			goto final;
		}
		offset = ((uint64_t)ALIGN(physical + bo_param.header_size, 256)) - physical;
	}

	bo->offset = offset;
	ret = vs_load_pvric_mask(filename, format, !!(mod & DRM_FORMAT_MOD_VS_DEC_LOSSY),
				 bo_param.header_size, offset, bo, va_addr, compressed);
	if (ret)
		fprintf(stderr, "load image %s failed with %d\n", filename, ret);

	pbo[0] = bo;
	return 0;

final:
	bo_unmap(bo);
	if (pbo) {
		memcpy(*pbo, bo, sizeof(*bo));
	}
	free(bo);
	return ret;
}

static void vs_bo_init(unsigned int format, uint64_t mod, drm_vs_bo_param bo_param,
		       struct bo *plane_bo)
{
	uint8_t *raw_buffer = (uint8_t *)plane_bo->ptr;
	uint8_t tile_mode = fourcc_mod_vs_get_tile_mode(mod);
	uint32_t i;

	memset(plane_bo->ptr, 0, plane_bo->size);

	/* fill in X channel with 0xFF for XRGB8_SuperTileX or 0x11 for XRGB2101010_SuperTileX, only works on 0x20000007 */
	if (tile_mode == DRM_FORMAT_MOD_VS_DEC_TILE_8X8_SUPERTILE_X ||
	    tile_mode == DRM_FORMAT_MOD_VS_TILE_8X8_SUPERTILE_X) {
		if (format == DRM_FORMAT_XRGB8888 || format == DRM_FORMAT_XBGR8888) {
			for (i = 0; i < (bo_param.width * bo_param.height); i += 4) {
				raw_buffer[4 * (i + 0) + 3] = 0xFF;
				raw_buffer[4 * (i + 1) + 3] = 0xFF;
				raw_buffer[4 * (i + 2) + 3] = 0xFF;
				raw_buffer[4 * (i + 3) + 3] = 0xFF;
			}
		} else if (format == DRM_FORMAT_RGBX8888 || format == DRM_FORMAT_BGRX8888) {
			for (i = 0; i < bo_param.width * bo_param.height; i += 4) {
				raw_buffer[4 * (i + 0)] = 0xFF;
				raw_buffer[4 * (i + 1)] = 0xFF;
				raw_buffer[4 * (i + 2)] = 0xFF;
				raw_buffer[4 * (i + 3)] = 0xFF;
			}
		} else if (format == DRM_FORMAT_XRGB2101010 || format == DRM_FORMAT_XBGR2101010) {
			for (i = 0; i < bo_param.width * bo_param.height; i += 4) {
				raw_buffer[4 * (i + 0) + 3] |= 0xc0;
				raw_buffer[4 * (i + 1) + 3] |= 0xc0;
				raw_buffer[4 * (i + 2) + 3] |= 0xc0;
				raw_buffer[4 * (i + 3) + 3] |= 0xc0;
			}
		} else if (format == DRM_FORMAT_RGBX1010102 || format == DRM_FORMAT_BGRX1010102) {
			for (i = 0; i < bo_param.width * bo_param.height; i += 4) {
				raw_buffer[4 * (i + 0)] |= 0x03;
				raw_buffer[4 * (i + 1)] |= 0x03;
				raw_buffer[4 * (i + 2)] |= 0x03;
				raw_buffer[4 * (i + 3)] |= 0x03;
			}
		}
	}
}

/* -----------------------------------------------------------------------------
 * vdp bo create and config
 */

int vs_vdp_config(struct device *dev, struct pipe_arg *pipes, unsigned int count)
{
	uint32_t *vdp_buffer_ptr = NULL;
	uint32_t display_offset = 0;
	uint32_t src_w, src_h, num_planes, width_height, i;
	const struct util_format_info *info = NULL;
	drm_vs_bo_param bo_param[4] = { 0 };
	struct bo *plane_bo[4] = { 0 };
	uint64_t physical[3] = { 0 };

	for (i = 0; i < count; i++) {
		struct pipe_arg *pipe = &pipes[i];

		src_w = pipe->wb_w;
		src_h = pipe->wb_h;
		drm_vs_get_align_size(&src_w, &src_h, pipe->fourcc, pipe->wb_modifier);

		drm_vs_bo_config(src_w, src_h, pipe->fourcc, pipe->wb_modifier, bo_param);
		info = util_format_mod_info_find(pipe->fourcc, pipe->wb_modifier);
		if (!info) {
			DTESTLOGE("failed to get format info");
			return -1;
		}
		num_planes = info->num_planes;
		pipe->num_planes = num_planes;

		plane_bo[0] = bo_create_dumb(dev->fd, bo_param[0].width, bo_param[0].height,
					     bo_param[0].bpp * num_planes);
		if (!plane_bo[0]) {
			DTESTLOGE("failed to create dumb: %s", strerror(-errno));
			return -1;
		}

		plane_bo[0]->offset = 0;
		_query_gem_physical(dev->fd, plane_bo[0]->handle, &physical[0]);

		for (i = 0; i < num_planes; i++) {
			pipe->out_bo[i] = plane_bo[i];

			if (pipe->bo_param != NULL)
				pipe->bo_param[i] = bo_param[i];
		}

		vdp_buffer_ptr = dev->vdp_buffer_logic;
		display_offset = 0x100 * i;
		width_height = ((src_w << 16) & 0xFFFF0000) | (src_h & 0x0000FFFF);
		*(vdp_buffer_ptr + 2 + display_offset) = width_height;
		*(vdp_buffer_ptr + 4 + display_offset) = physical[0];
		*(vdp_buffer_ptr + 5 + display_offset) = src_w * 4;
		*(vdp_buffer_ptr + 1 + display_offset) = 0xa0011;
	}

	return 0;
}

/* -----------------------------------------------------------------------------
 * vs bo create
 */
int vs_bo_create(int fd, unsigned int format, uint64_t mod, unsigned int width, unsigned int height,
		 char (*filename)[RESOURCE_LEN + 1], enum util_fill_pattern pattern,
		 uint64_t modifiers[4], struct bo *pbo[4], drm_vs_bo_param bo_params[4],
		 uint32_t frame_idx)
{
	uint32_t src_w, src_h, num_planes, i;
	const struct util_format_info *info = NULL;
	drm_vs_bo_param bo_param[4] = { 0 };
	struct drm_vs_pvric_offset fbc_offsets = { 0 };
	struct bo *plane_bo[4] = { 0 };
	void *va_addr[3] = { 0 };
	int ret = 0;
	uint64_t physical[3] = { 0 };

	src_w = width;
	src_h = height;
	drm_vs_get_align_size(&src_w, &src_h, format, mod);

	drm_vs_bo_config(src_w, src_h, format, mod, bo_param);
	info = util_format_mod_info_find(format, mod);
	if (!info) {
		DTESTLOGE("failed to get format info");
		return -1;
	}
	num_planes = info->num_planes;

	for (i = 0; i < num_planes; i++) {
		if (!bo_param[i].width)
			continue;
		plane_bo[i] =
			bo_create_dumb(fd, bo_param[i].width, bo_param[i].height, bo_param[i].bpp);
		if (!plane_bo[i]) {
			DTESTLOGE("failed to create dumb: %s", strerror(-errno));
			return -1;
		}

		modifiers[i] = mod;
		plane_bo[i]->offset = 0;
		_query_gem_physical(fd, plane_bo[i]->handle, &physical[i]);
	}

	if (fourcc_mod_vs_is_pvric(mod)) {
		fbc_offsets.format = format;

		for (i = 0; i < num_planes; i++) {
			fbc_offsets.handles[i] = plane_bo[i]->handle;
			fbc_offsets.header_size[i] = bo_param[i].header_size;
		}
		ret = drmIoctl(fd, DRM_IOCTL_VS_GET_FBC_OFFSET, &fbc_offsets);
		if (ret) {
			DTESTLOGE("failed to get fbc offset: %s", strerror(-errno));
			return -1;
		}

		for (i = 0; i < num_planes; i++) {
			plane_bo[i]->offset = fbc_offsets.offsets[i];
		}
	}

	if (fourcc_mod_vs_is_dec400a(mod)) {
		for (i = 0; i < num_planes; i++) {
			plane_bo[i]->offset = bo_param[i].ts_buf_size;
		}
	}

	for (i = 0; i < num_planes; i++) {
		ret = bo_map(plane_bo[i], &va_addr[i]);
		if (ret) {
			DTESTLOGE("failed to map buffer: %s", strerror(-errno));
			bo_destroy(plane_bo[i]);
			return -1;
		}
		vs_bo_init(format, mod, bo_param[i], plane_bo[i]);
	}

	if (filename && strcmp(filename[0], "defaultPattern")) {
		ret = vs_plane_fill(filename, format, mod, va_addr, bo_param, plane_bo, physical,
				    frame_idx);
		if (ret)
			DTESTLOGW("load image %s failed with %d", filename[0], ret);
	} else {
		if (filename) {
			DTESTLOGI("Plane resource is not specified, will use default pattern fill");
			util_fill_pattern(format, pattern, va_addr, src_w, src_h,
					  plane_bo[0]->pitch);
		}
	}

	for (i = 0; i < num_planes; i++) {
		bo_unmap(plane_bo[i]);
		pbo[i] = plane_bo[i];

		if (bo_params != NULL)
			bo_params[i] = bo_param[i];
	}

	return 0;
}
