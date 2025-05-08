/***************************************************************************
*	 Copyright 2012 - 2023 Vivante Corporation, Santa Clara, California.
*	 All Rights Reserved.
*
*	 Permission is hereby granted, free of charge, to any person obtaining
*	 a copy of this software and associated documentation files (the
*	 'Software'), to deal in the Software without restriction, including
*	 without limitation the rights to use, copy, modify, merge, publish,
*	 distribute, sub license, and/or sell copies of the Software, and to
*	 permit persons to whom the Software is furnished to do so, subject
*	 to the following conditions:
*
*	 The above copyright notice and this permission notice (including the
*	 next paragraph) shall be included in all copies or substantial
*	 portions of the Software.
*
*	 THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*	 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*	 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*	 IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*	 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*	 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*	 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <linux/media-bus-format.h>
#include <unistd.h>

#include "drm/vs_drm_fourcc.h"
#include "drm/vs_drm.h"
#include "util/format.h"
#include "util/common.h"
#include "buffers.h"
#include "vs_bo_helper.h"
#include "drmtest_helper.h"
#include "map_funcs/prop_map_funcs.h"
#include "viv_dc_functions.h"

#define BIT(n) (1 << (n))

#define ARRAY_ENUM_MATCH_FAIL(status)                             \
	if (status == -1) {                                       \
		printf("This array list not find match enum.\n"); \
		return -1;                                        \
	}

#define BLOB_PROP_GLOBAL_DISABLE(obj)                                                  \
	do {                                                                           \
		cJSON *global_disable_obj = cJSON_GetObjectItem(obj, "globalDisable"); \
		if (global_disable_obj) {                                              \
			if (global_disable_obj->valueint)                              \
				return 0;                                              \
		}                                                                      \
	} while (0);

struct enum_str_match_list {
	uint32_t type;
	const char *name;
};

typedef struct _obj_mask {
	char file[256];
	uint32_t format;
	uint64_t mod;
	enum util_fill_pattern pattern;
	uint32_t width;
	uint32_t height;
} obj_mask;

static int string_to_enum(const struct enum_str_match_list *list, uint32_t size, char *str)
{
	uint32_t i = 0;

	for (i = 0; i < size; i++) {
		if (!strcmp(list[i].name, str))
			return list[i].type;
	}

	return -1;
}

static int32_t get_uint16_list(cJSON *obj, uint16_t *list)
{
	int32_t i;
	int32_t size = cJSON_GetArraySize(obj);
	cJSON *item;
	for (i = 0; i < size; i++) {
		item = cJSON_GetArrayItem(obj, i);
		list[i] = (uint16_t)item->valueint;
	}
	return size;
}

static int32_t get_uint32_list(cJSON *obj, uint32_t *list)
{
	int32_t i;
	int32_t size = cJSON_GetArraySize(obj);
	cJSON *item;
	for (i = 0; i < size; i++) {
		item = cJSON_GetArrayItem(obj, i);
		list[i] = (uint32_t)item->valuedouble;
	}
	return size;
}

static int32_t get_int_list(cJSON *obj, int *list)
{
	int32_t i;
	int32_t size = cJSON_GetArraySize(obj);
	cJSON *item;
	for (i = 0; i < size; i++) {
		item = cJSON_GetArrayItem(obj, i);
		list[i] = item->valueint;
	}
	return size;
}

/*
static int32_t parse_pattern(const char *pattern)
{
	if(strcmp(pattern, "UTIL_PATTERN_TILES") == 0) {
		return UTIL_PATTERN_TILES;
	}
	else if(strcmp(pattern, "UTIL_PATTERN_PLAIN") == 0) {
		return UTIL_PATTERN_PLAIN;
	}
	else if(strcmp(pattern, "UTIL_PATTERN_SMPTE") == 0) {
		return UTIL_PATTERN_SMPTE;
	}
	else if(strcmp(pattern, "UTIL_PATTERN_GRADIENT") == 0) {
		return UTIL_PATTERN_GRADIENT;
	}
	else {
		fprintf(stderr, "Can't find the pattern %s\n", pattern);
		return -1;
	}
}

static int32_t parse_tile(const char *name)
{
#define TILECMP(tilename) \
	do { \
		if(strcmp(#tilename, name) == 0) \
			return DRM_FORMAT_MOD_VS_DEC_##tilename; \
	} while(0);

	TILECMP(TILE_8X8_XMAJOR)
	TILECMP(TILE_8X8_YMAJOR)
	TILECMP(TILE_16X4)
	TILECMP(TILE_8X4)
	TILECMP(TILE_4X8)
	TILECMP(RASTER_16X4)
	TILECMP(TILE_64X4)
	TILECMP(TILE_32X4)
	TILECMP(RASTER_256X1)
	TILECMP(RASTER_128X1)
	TILECMP(RASTER_64X4)
	TILECMP(RASTER_256X2)
	TILECMP(RASTER_128X2)
	TILECMP(RASTER_128X4)
	TILECMP(RASTER_64X1)
	TILECMP(TILE_16X8)
	TILECMP(TILE_8X16)
	TILECMP(RASTER_512X1)
	TILECMP(RASTER_32X4)
	TILECMP(RASTER_64X2)
	TILECMP(RASTER_32X2)
	TILECMP(RASTER_32X1)
	TILECMP(RASTER_16X1)
	TILECMP(TILE_128X4)
	TILECMP(TILE_256X4)
	TILECMP(TILE_512X4)
	TILECMP(TILE_16X16)
	TILECMP(TILE_32X16)
	TILECMP(TILE_64X16)
	TILECMP(TILE_128X8)
	TILECMP(TILE_8X4_S)
	TILECMP(TILE_16X4_S)
	TILECMP(TILE_32X4_S)
	TILECMP(TILE_16X4_LSB)
	TILECMP(TILE_32X4_LSB)
	TILECMP(TILE_32X8)
	TILECMP(TILE_8X8)
	TILECMP(TILE_32X2)

	fprintf(stderr, "Can't find tile %s\n", name);
	return -1;
}*/

static void fill_random_dither_seed(cJSON *obj, struct drm_vs_random_dither_seed *seed)
{
	cJSON *hash_seed_x_enable_obj = cJSON_GetObjectItem(obj, "hash_seed_x_enable");
	cJSON *hash_seed_y_enable_obj = cJSON_GetObjectItem(obj, "hash_seed_y_enable");
	cJSON *permut_seed1_enable_obj = cJSON_GetObjectItem(obj, "permut_seed1_enable");
	cJSON *permut_seed2_enable_obj = cJSON_GetObjectItem(obj, "permut_seed2_enable");
	cJSON *hash_seed_x_obj = cJSON_GetObjectItem(obj, "hash_seed_x");
	cJSON *hash_seed_y_obj = cJSON_GetObjectItem(obj, "hash_seed_y");
	cJSON *permut_seed1_obj = cJSON_GetObjectItem(obj, "permut_seed1");
	cJSON *permut_seed2_obj = cJSON_GetObjectItem(obj, "permut_seed2");

	if (hash_seed_x_enable_obj)
		seed->hash_seed_x_enable = (bool)hash_seed_x_enable_obj->valueint;
	if (hash_seed_y_enable_obj)
		seed->hash_seed_y_enable = (bool)hash_seed_y_enable_obj->valueint;
	if (permut_seed1_enable_obj)
		seed->permut_seed1_enable = (bool)permut_seed1_enable_obj->valueint;
	if (permut_seed2_enable_obj)
		seed->permut_seed2_enable = (bool)permut_seed2_enable_obj->valueint;
	if (hash_seed_x_obj)
		get_uint32_list(hash_seed_x_obj, seed->hash_seed_x);
	if (hash_seed_y_obj)
		get_uint32_list(hash_seed_y_obj, seed->hash_seed_y);
	if (permut_seed1_obj)
		get_uint32_list(permut_seed1_obj, seed->permut_seed1);
	if (permut_seed2_obj)
		get_uint32_list(permut_seed2_obj, seed->permut_seed2);
}

static int fill_obj_mask(cJSON *obj, obj_mask *mask, bool lossy, bool *compressed)
{
	cJSON *file_obj = cJSON_GetObjectItem(obj, "file");
	cJSON *width_obj = cJSON_GetObjectItem(obj, "width");
	cJSON *height_obj = cJSON_GetObjectItem(obj, "height");
	cJSON *tile_obj = cJSON_GetObjectItem(obj, "tileMode");
	uint32_t fourcc = DRM_FORMAT_C8;
	uint8_t tile_mode = DRM_FORMAT_MOD_VS_LINEAR;
	uint8_t dec_type = DRM_FORMAT_MOD_VS_TYPE_NORMAL;
	uint64_t modifier =
		fourcc_mod_vs_code(dec_type, tile_mode); /* DEC/PVRIC NONE, tile LINEAR */
	char rcd_src[RESOURCE_LEN + 1] = { 0 };
	*compressed = false;

	if (tile_obj) {
		tile_mode = dtest_get_obj_normal_tilemode(obj, "tileMode");

		if (tile_mode == DRM_FORMAT_MOD_VS_TILE_8X8) {
			tile_mode = DRM_FORMAT_MOD_VS_DEC_TILE_8X8;
			modifier = fourcc_mod_vs_pvric_code(tile_mode, lossy);
			*compressed = true;
		}
	}

	if (file_obj) {
		size_t file_len = strlen(file_obj->valuestring);

		if (file_len > 0 && file_len < (RESOURCE_LEN + 1))
			strncpy((char *)rcd_src, file_obj->valuestring, file_len);

		if (file_len > sizeof(mask->file)) {
			fprintf(stderr, "The filename is longer than 256\n");
			return -1;
		}
		memset(mask->file, 0, sizeof(mask->file));
		sprintf(mask->file, "resource/%s", rcd_src);
	} else
		strcpy((char *)rcd_src, "defaultPattern");

	mask->format = fourcc;
	if (!height_obj) {
		printf("not found height config.\n");
		return 0;
	} else
		mask->height = height_obj->valueint;
	if (!width_obj) {
		printf("not found width config.\n");
		return 0;
	} else
		mask->width = width_obj->valueint;
	mask->pattern = UTIL_PATTERN_SMPTE;
	mask->mod = modifier;

	return 0;
}

static uint32_t fill_drm_mask(cJSON *obj, struct device *dev)
{
	obj_mask mask;
	unsigned int fb_id = 0;
	uint32_t handles[4] = { 0 }, pitches[4] = { 0 }, offsets[4] = { 0 };

	uint64_t modifiers[4] = { 0, 0, 0, 0 };
	struct bo *crtc_bo[4];
	uint32_t ret = 0;
	uint32_t align_w = 0, align_h = 0;
	bool compressed = false;

	fill_obj_mask(obj, &mask, false, &compressed);

	ret = vs_mask_bo_create(dev->fd, mask.format, mask.mod, mask.width, mask.height, mask.file,
				mask.pattern, crtc_bo, compressed);
	if (ret) {
		printf("failed to create bo for mask framebuffer\n");
	}

	align_w = mask.width;
	align_h = mask.height;
	drm_vs_get_align_size(&align_w, &align_h, mask.format, mask.mod);

	handles[0] = crtc_bo[0]->handle;
	pitches[0] = crtc_bo[0]->pitch;
	offsets[0] = crtc_bo[0]->offset;
	modifiers[0] = mask.mod;

	if (drmModeAddFB2WithModifiers(dev->fd, align_w, align_h, mask.format, handles, pitches,
				       offsets, modifiers, &fb_id, DRM_MODE_FB_MODIFIERS)) {
		printf("failed to add framebuffer for mask\n");
	}

	return fb_id;
}

static void fill_rect(cJSON *obj, struct drm_vs_rect *react)
{
	cJSON *x_obj = cJSON_GetObjectItem(obj, "x");
	cJSON *y_obj = cJSON_GetObjectItem(obj, "y");
	cJSON *w_obj = cJSON_GetObjectItem(obj, "w");
	cJSON *h_obj = cJSON_GetObjectItem(obj, "h");

	if (x_obj)
		react->x = x_obj->valueint;
	if (y_obj)
		react->y = y_obj->valueint;
	if (w_obj)
		react->w = w_obj->valueint;
	if (h_obj)
		react->h = h_obj->valueint;
}

static uint32_t value_in_range(uint32_t value, uint32_t min, uint32_t max)
{
	if (value >= min && value <= max)
		return value;

	return 0;
}

static void fill_position(cJSON *obj, struct drm_vs_position *pos)
{
	cJSON *x_obj = cJSON_GetObjectItem(obj, "x");
	cJSON *y_obj = cJSON_GetObjectItem(obj, "y");

	if (x_obj)
		pos->x = x_obj->valueint;
	if (y_obj)
		pos->y = y_obj->valueint;
}

uint64_t MAPAPI map_ext_layer_fb(cJSON *obj, struct device *dev)
{
	char format_str[20] = { 0 };
	unsigned int fourcc = 0;
	unsigned int fb_id = 0;
	uint32_t width = 0, height = 0, num_planes = 0, i = 0;
	uint32_t handles[4] = { 0 }, pitches[4] = { 0 }, offsets[4] = { 0 };
	uint8_t dec_type = DRM_FORMAT_MOD_VS_TYPE_NORMAL;
	uint8_t tile_mode = DRM_FORMAT_MOD_VS_LINEAR;
	uint64_t modifier =
		fourcc_mod_vs_code(dec_type, tile_mode); /* DEC/PVRIC NONE, tile LINEAR */
	uint64_t modifiers[4] = { 0 };
	struct bo *plane_bo[4];
	char file_src[4][RESOURCE_LEN + 1] = { "defaultPattern" };
	const struct util_format_info *info = NULL;

	fourcc = dtest_get_obj_format(obj, format_str);
	width = dtest_get_obj_int(obj, "width");
	height = dtest_get_obj_int(obj, "height");
	modifier = dtest_get_obj_dec_fb_modifier(obj, "decType");

	cJSON *obj_file_src = cJSON_GetObjectItem(obj, "resource");
	if (obj_file_src) {
		size_t len = strlen(obj_file_src->valuestring);

		if (len > 0 && len < (RESOURCE_LEN + 1))
			strncpy(file_src[0], obj_file_src->valuestring, len);
	}

	info = util_format_info_find(fourcc);
	if (info == NULL) {
		printf("Extend layer framebuffer with format %u has no matched format info\n",
		       fourcc);
		return 0;
	}

	num_planes = info->num_planes;

	if (vs_bo_create(dev->fd, fourcc, modifier, width, height, file_src, UTIL_PATTERN_SMPTE,
			 modifiers, plane_bo, NULL, 0)) {
		printf("failed to create bo for extend layer framebuffer\n");
		return 0;
	}

	for (i = 0; i < num_planes; i++) {
		handles[i] = plane_bo[i]->handle;
		pitches[i] = plane_bo[i]->pitch;
		offsets[i] = plane_bo[i]->offset;
	}

	if (drmModeAddFB2WithModifiers(dev->fd, width, height, fourcc, handles, pitches, offsets,
				       modifiers, &fb_id, DRM_MODE_FB_MODIFIERS)) {
		printf("failed to add framebuffer for extend layer\n");
		return 0;
	}

	return fb_id;
}

/*     momenta dev        */
uint64_t MAPAPI map_multi_extend_layer(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	cJSON *ext_x_obj = NULL, *ext_y_obj = NULL, *ext_w_obj = NULL, *ext_h_obj = NULL;
	cJSON *width_obj = NULL, *height_obj = NULL;
	struct drm_vs_ex_layer ex_layer = { 0 };
	char format_str[20] = { 0 };
	unsigned int fourcc = 0;
	uint64_t modifier;
	uint32_t width[VS_MAX_EXT_LAYER_CNT] = { 0 }, height[VS_MAX_EXT_LAYER_CNT] = { 0 };
	const struct util_format_info *info = NULL;
	char file_src[3][4][RESOURCE_LEN + 1] = { { "defaultPattern" },
						  { "defaultPattern" },
						  { "defaultPattern" } };
	char *tileStatus[3][3] = { { "tileStatusEx0", "tileStatus1Ex0", "tileStatus2Ex0" },
				   { "tileStatusEx1", "tileStatus1Ex1", "tileStatus2Ex1" },
				   { "tileStatusEx2", "tileStatus1Ex2", "tileStatus2Ex2" } };
	uint32_t i = 0, plane_num = 0;

	BLOB_PROP_GLOBAL_DISABLE(obj);

	fourcc = dtest_get_obj_format(obj, format_str);
	modifier = dtest_get_obj_dec_fb_modifier(obj, "decType");
	info = util_format_info_find(fourcc);
	if (info == NULL) {
		printf("Multi extend layer framebuffer with format %u has no matched format info\n",
		       fourcc);
		return 0;
	}
	ex_layer.fd = dev->fd;
	ex_layer.num = 0;
	ex_layer.fourcc = fourcc;
	ex_layer.modifier = modifier;

	ext_x_obj = cJSON_GetObjectItem(obj, "layerOutEx0X");
	ext_y_obj = cJSON_GetObjectItem(obj, "layerOutEx0Y");
	ext_w_obj = cJSON_GetObjectItem(obj, "layerOutEx0Width");
	ext_h_obj = cJSON_GetObjectItem(obj, "layerOutEx0Height");
	width_obj = cJSON_GetObjectItem(obj, "width_Ex0");
	height_obj = cJSON_GetObjectItem(obj, "height_Ex0");
	if (ext_x_obj && ext_y_obj && ext_w_obj && ext_h_obj && width_obj && height_obj) {
		ex_layer.out_rect[0].x = ext_x_obj->valueint;
		ex_layer.out_rect[0].y = ext_y_obj->valueint;
		ex_layer.out_rect[0].w = ext_w_obj->valueint;
		ex_layer.out_rect[0].h = ext_h_obj->valueint;
		width[0] = width_obj->valueint;
		height[0] = height_obj->valueint;
		ex_layer.crop_rect[0].x = dtest_get_obj_int(obj, "cropStartEx0X");
		ex_layer.crop_rect[0].y = dtest_get_obj_int(obj, "cropStartEx0Y");
		ex_layer.num++;

		dtest_get_obj_str(obj, "resourceEx0", file_src[0][0]);
		if (fourcc_mod_vs_is_compressed(modifier)) {
			info = util_format_info_find(fourcc);
			plane_num = info->num_planes;

			for (i = 0; i < plane_num; i++)
				dtest_get_obj_str(obj, tileStatus[0][i], file_src[0][i + 1]);
		}
		ex_layer.fb_id[0] = ext_fb_create(dev, file_src[0], fourcc, modifier,
						  info->num_planes, width[0], height[0]);
	}

	ext_x_obj = cJSON_GetObjectItem(obj, "layerOutEx1X");
	ext_y_obj = cJSON_GetObjectItem(obj, "layerOutEx1Y");
	ext_w_obj = cJSON_GetObjectItem(obj, "layerOutEx1Width");
	ext_h_obj = cJSON_GetObjectItem(obj, "layerOutEx1Height");
	width_obj = cJSON_GetObjectItem(obj, "width_Ex1");
	height_obj = cJSON_GetObjectItem(obj, "height_Ex1");
	if (ext_x_obj && ext_y_obj && ext_w_obj && ext_h_obj && width_obj && height_obj) {
		ex_layer.out_rect[1].x = ext_x_obj->valueint;
		ex_layer.out_rect[1].y = ext_y_obj->valueint;
		ex_layer.out_rect[1].w = ext_w_obj->valueint;
		ex_layer.out_rect[1].h = ext_h_obj->valueint;
		width[1] = width_obj->valueint;
		height[1] = height_obj->valueint;
		ex_layer.crop_rect[1].x = dtest_get_obj_int(obj, "cropStartEx1X");
		ex_layer.crop_rect[1].y = dtest_get_obj_int(obj, "cropStartEx1Y");
		ex_layer.num++;

		dtest_get_obj_str(obj, "resourceEx1", file_src[1][0]);
		if (fourcc_mod_vs_is_compressed(modifier)) {
			info = util_format_info_find(fourcc);
			plane_num = info->num_planes;

			for (i = 0; i < plane_num; i++)
				dtest_get_obj_str(obj, tileStatus[1][i], file_src[1][i + 1]);
		}
		ex_layer.fb_id[1] = ext_fb_create(dev, file_src[1], fourcc, modifier,
						  info->num_planes, width[1], height[1]);
	}

	ext_x_obj = cJSON_GetObjectItem(obj, "layerOutEx2X");
	ext_y_obj = cJSON_GetObjectItem(obj, "layerOutEx2Y");
	ext_w_obj = cJSON_GetObjectItem(obj, "layerOutEx2Width");
	ext_h_obj = cJSON_GetObjectItem(obj, "layerOutEx2Height");
	width_obj = cJSON_GetObjectItem(obj, "width_Ex2");
	height_obj = cJSON_GetObjectItem(obj, "height_Ex2");
	if (ext_x_obj && ext_y_obj && ext_w_obj && ext_h_obj && width_obj && height_obj) {
		ex_layer.out_rect[2].x = ext_x_obj->valueint;
		ex_layer.out_rect[2].y = ext_y_obj->valueint;
		ex_layer.out_rect[2].w = ext_w_obj->valueint;
		ex_layer.out_rect[2].h = ext_h_obj->valueint;
		width[2] = width_obj->valueint;
		height[2] = height_obj->valueint;
		ex_layer.crop_rect[2].x = dtest_get_obj_int(obj, "cropStartEx2X");
		ex_layer.crop_rect[2].y = dtest_get_obj_int(obj, "cropStartEx2Y");
		ex_layer.num++;

		dtest_get_obj_str(obj, "resourceEx2", file_src[2][0]);
		if (fourcc_mod_vs_is_compressed(modifier)) {
			info = util_format_info_find(fourcc);
			plane_num = info->num_planes;

			for (i = 0; i < plane_num; i++)
				dtest_get_obj_str(obj, tileStatus[2][i], file_src[2][i + 1]);
		}
		ex_layer.fb_id[2] = ext_fb_create(dev, file_src[2], fourcc, modifier,
						  info->num_planes, width[2], height[2]);
	}

	drmModeCreatePropertyBlob(dev->fd, &ex_layer, sizeof(struct drm_vs_ex_layer), &blob_id);
	return blob_id;
}
uint64_t MAPAPI map_dma_config(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	cJSON *roi_x_obj = NULL, *roi_y_obj = NULL, *roi_w_obj = NULL, *roi_h_obj = NULL;
	cJSON *dma_mode_obj = NULL;
	struct drm_vs_dma dma;

	BLOB_PROP_GLOBAL_DISABLE(obj);

	dma_mode_obj = cJSON_GetObjectItem(obj, "dmaMode");

	if (!dma_mode_obj) {
		printf("not found dmaMode config.\n");
		return 0;
	}
	if (!strcmp(dma_mode_obj->valuestring, "oneROI"))
		dma.mode = VS_DMA_ONE_ROI;
	else if (!strcmp(dma_mode_obj->valuestring, "twoROI"))
		dma.mode = VS_DMA_TWO_ROI;
	else if (!strcmp(dma_mode_obj->valuestring, "skipROI"))
		dma.mode = VS_DMA_SKIP_ROI;
	else if (!strcmp(dma_mode_obj->valuestring, "extendLayer"))
		dma.mode = VS_DMA_EXT_LAYER;
	else if (!strcmp(dma_mode_obj->valuestring, "extendLayerEx"))
		dma.mode = VS_DMA_EXT_LAYER_EX;

	if (dma.mode == VS_DMA_ONE_ROI || dma.mode == VS_DMA_TWO_ROI ||
	    dma.mode == VS_DMA_EXT_LAYER || dma.mode == VS_DMA_SKIP_ROI ||
	    dma.mode == VS_DMA_EXT_LAYER_EX) {
		if (dma.mode != VS_DMA_EXT_LAYER) {
			roi_x_obj = cJSON_GetObjectItem(obj, "roiX");
			roi_y_obj = cJSON_GetObjectItem(obj, "roiY");
			roi_w_obj = cJSON_GetObjectItem(obj, "roiWidth");
			roi_h_obj = cJSON_GetObjectItem(obj, "roiHeight");
			if (roi_x_obj)
				dma.in_rect[0].x = roi_x_obj->valueint;
			if (roi_y_obj)
				dma.in_rect[0].y = roi_y_obj->valueint;
			if (roi_w_obj)
				dma.in_rect[0].w = roi_w_obj->valueint;
			if (roi_h_obj)
				dma.in_rect[0].h = roi_h_obj->valueint;
		}

		roi_x_obj = cJSON_GetObjectItem(obj, "layerOutX");
		roi_y_obj = cJSON_GetObjectItem(obj, "layerOutY");
		roi_w_obj = cJSON_GetObjectItem(obj, "layerOutWidth");
		roi_h_obj = cJSON_GetObjectItem(obj, "layerOutHeight");
		if (roi_x_obj)
			dma.out_rect[0].x = roi_x_obj->valueint;
		if (roi_y_obj)
			dma.out_rect[0].y = roi_y_obj->valueint;
		if (roi_w_obj)
			dma.out_rect[0].w = roi_w_obj->valueint;
		if (roi_h_obj)
			dma.out_rect[0].h = roi_h_obj->valueint;

		if (dma.mode == VS_DMA_TWO_ROI || dma.mode == VS_DMA_EXT_LAYER_EX ||
		    dma.mode == VS_DMA_EXT_LAYER) {
			if (dma.mode != VS_DMA_EXT_LAYER) {
				roi_x_obj = cJSON_GetObjectItem(obj, "roiExX");
				roi_y_obj = cJSON_GetObjectItem(obj, "roiExY");
				roi_w_obj = cJSON_GetObjectItem(obj, "roiExWidth");
				roi_h_obj = cJSON_GetObjectItem(obj, "roiExHeight");
				if (roi_x_obj)
					dma.in_rect[1].x = roi_x_obj->valueint;
				if (roi_y_obj)
					dma.in_rect[1].y = roi_y_obj->valueint;
				if (roi_w_obj)
					dma.in_rect[1].w = roi_w_obj->valueint;
				if (roi_h_obj)
					dma.in_rect[1].h = roi_h_obj->valueint;
			}

			roi_x_obj = cJSON_GetObjectItem(obj, "layerOutExX");
			roi_y_obj = cJSON_GetObjectItem(obj, "layerOutExY");
			roi_w_obj = cJSON_GetObjectItem(obj, "layerOutExWidth");
			roi_h_obj = cJSON_GetObjectItem(obj, "layerOutExHeight");
			if (roi_x_obj)
				dma.out_rect[1].x = roi_x_obj->valueint;
			if (roi_y_obj)
				dma.out_rect[1].y = roi_y_obj->valueint;
			if (roi_w_obj)
				dma.out_rect[1].w = roi_w_obj->valueint;
			if (roi_h_obj)
				dma.out_rect[1].h = roi_h_obj->valueint;
		}
	}

	drmModeCreatePropertyBlob(dev->fd, &dma, sizeof(struct drm_vs_dma), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_line_padding(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_line_padding line_padding = { 0 };
	cJSON *mode_obj = cJSON_GetObjectItem(obj, "mode");
	cJSON *a_obj = cJSON_GetObjectItem(obj, "a");
	cJSON *r_obj = cJSON_GetObjectItem(obj, "r");
	cJSON *g_obj = cJSON_GetObjectItem(obj, "g");
	cJSON *b_obj = cJSON_GetObjectItem(obj, "b");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (!mode_obj) {
		printf("not found mode config for line padding.\n");
		return 0;
	}

	if (!strcmp(mode_obj->valuestring, "1TO1"))
		line_padding.mode = VS_DMA_LINE_PADDING_1TO1;
	else if (!strcmp(mode_obj->valuestring, "2TO1"))
		line_padding.mode = VS_DMA_LINE_PADDING_2TO1;
	else if (!strcmp(mode_obj->valuestring, "3TO1"))
		line_padding.mode = VS_DMA_LINE_PADDING_3TO1;
	else if (!strcmp(mode_obj->valuestring, "3TO2"))
		line_padding.mode = VS_DMA_LINE_PADDING_3TO2;
	else if (!strcmp(mode_obj->valuestring, "4TO1"))
		line_padding.mode = VS_DMA_LINE_PADDING_4TO1;
	else if (!strcmp(mode_obj->valuestring, "4TO3"))
		line_padding.mode = VS_DMA_LINE_PADDING_4TO3;
	else if (!strcmp(mode_obj->valuestring, "5TO2"))
		line_padding.mode = VS_DMA_LINE_PADDING_5TO2;
	else if (!strcmp(mode_obj->valuestring, "5TO3"))
		line_padding.mode = VS_DMA_LINE_PADDING_5TO3;
	else if (!strcmp(mode_obj->valuestring, "8TO5"))
		line_padding.mode = VS_DMA_LINE_PADDING_8TO5;

	if (a_obj)
		line_padding.color.a = a_obj->valueint;
	if (r_obj)
		line_padding.color.r = r_obj->valueint;
	if (g_obj)
		line_padding.color.g = g_obj->valueint;
	if (b_obj)
		line_padding.color.b = b_obj->valueint;

	drmModeCreatePropertyBlob(dev->fd, &line_padding, sizeof(struct drm_vs_line_padding),
				  &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_up_sample(cJSON *obj, struct device *dev)
{
	uint64_t up_sample = 0;
	uint32_t h_phase = 0, v_phase = 0;
	cJSON *h_phase_obj = NULL, *v_phase_obj = NULL;

	h_phase_obj = cJSON_GetObjectItem(obj, "hPhase");
	v_phase_obj = cJSON_GetObjectItem(obj, "vPhase");

	if (h_phase_obj)
		h_phase = h_phase_obj->valueint & 0xFF;
	if (v_phase_obj)
		v_phase = v_phase_obj->valueint & 0xFF;

	up_sample = ((v_phase << 8) | h_phase) & 0xFFFF;

	return up_sample;
}

uint64_t MAPAPI map_y2r_csc(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_y2r_config y2r_config;

	BLOB_PROP_GLOBAL_DISABLE(obj);

	cJSON *y2r_mode_obj = cJSON_GetObjectItem(obj, "mode");
	cJSON *y2r_gamut_obj = cJSON_GetObjectItem(obj, "gamut");
	cJSON *y2r_coef_obj = cJSON_GetObjectItem(obj, "coef");

	if (y2r_mode_obj) {
		if (!strcmp(y2r_mode_obj->valuestring, "USR"))
			y2r_config.mode = VS_CSC_CM_USR;
		else if (!strcmp(y2r_mode_obj->valuestring, "L2L"))
			y2r_config.mode = VS_CSC_CM_L2L;
		else if (!strcmp(y2r_mode_obj->valuestring, "L2F"))
			y2r_config.mode = VS_CSC_CM_L2F;
		else if (!strcmp(y2r_mode_obj->valuestring, "F2L"))
			y2r_config.mode = VS_CSC_CM_F2L;
		else if (!strcmp(y2r_mode_obj->valuestring, "F2F"))
			y2r_config.mode = VS_CSC_CM_F2F;
		else
			y2r_config.mode = VS_CSC_CM_F2F;
	}
	if (y2r_gamut_obj) {
		if (!strcmp(y2r_gamut_obj->valuestring, "BT601"))
			y2r_config.gamut = VS_CSC_CG_601;
		else if (!strcmp(y2r_gamut_obj->valuestring, "BT709"))
			y2r_config.gamut = VS_CSC_CG_709;
		else if (!strcmp(y2r_gamut_obj->valuestring, "BT2020"))
			y2r_config.gamut = VS_CSC_CG_2020;
		else if (!strcmp(y2r_gamut_obj->valuestring, "DCIP3"))
			y2r_config.gamut = VS_CSC_CG_P3;
		else if (!strcmp(y2r_gamut_obj->valuestring, "SRGB"))
			y2r_config.gamut = VS_CSC_CG_SRGB;
		else
			y2r_config.gamut = VS_CSC_CG_601;
	}
	if (y2r_coef_obj)
		get_int_list(y2r_coef_obj, (int *)y2r_config.coef);

	drmModeCreatePropertyBlob(dev->fd, &y2r_config, sizeof(struct drm_vs_y2r_config), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_ltm_gamma(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_ltm_xgamma ltm_xgamma;

	BLOB_PROP_GLOBAL_DISABLE(obj);

	uint32_t in_bit = dtest_get_obj_int(obj, "in_bit");
	uint32_t out_bit = dtest_get_obj_int(obj, "out_bit");
	float exp = (float)dtest_get_obj_int(obj, "expPreMult10") / 10.0;

	drm_vs_init_gamma_curve(in_bit, out_bit, VS_LTM_XGAMMA_COEF_NUM - 1, exp, ltm_xgamma.coef);

	drmModeCreatePropertyBlob(dev->fd, &ltm_xgamma, sizeof(struct drm_vs_ltm_xgamma), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_ltm_degamma(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_ltm_xgamma ltm_xgamma;

	BLOB_PROP_GLOBAL_DISABLE(obj);

	uint32_t in_bit = dtest_get_obj_int(obj, "in_bit");
	uint32_t out_bit = dtest_get_obj_int(obj, "out_bit");
	float exp = (float)dtest_get_obj_int(obj, "expPreMult10") / 10.0;

	drm_vs_init_degamma_curve(in_bit, out_bit, VS_LTM_XGAMMA_COEF_NUM - 1, exp,
				  ltm_xgamma.coef);

	drmModeCreatePropertyBlob(dev->fd, &ltm_xgamma, sizeof(struct drm_vs_ltm_xgamma), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_ltm_luma(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	uint32_t temp = 0;
	struct drm_vs_ltm_luma ltm_luma = { 0 };
	cJSON *mode_obj = cJSON_GetObjectItem(obj, "mode");
	cJSON *coef_obj = cJSON_GetObjectItem(obj, "coef");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	const struct enum_str_match_list luma_cal_mode_list[] = {
		{ VS_LTM_LUMA_GRAY, "GRAY" },
		{ VS_LTM_LUMA_LIGHTNESS, "LIGHTNESS" },
		{ VS_LTM_LUMA_MIXED, "MIXED" },
	};

	if (mode_obj) {
		temp = string_to_enum(luma_cal_mode_list, ARRAY_SIZE(luma_cal_mode_list),
				      mode_obj->valuestring);
		ARRAY_ENUM_MATCH_FAIL(temp);
		ltm_luma.mode = temp;
	} else
		ltm_luma.mode = VS_LTM_LUMA_GRAY; /* LTM mode default GRAY */

	get_uint16_list(coef_obj, ltm_luma.coef);

	drmModeCreatePropertyBlob(dev->fd, &ltm_luma, sizeof(struct drm_vs_ltm_luma), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_ltm_freq_decomp(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_ltm_freq_decomp ltm_freq_decomp;
	cJSON *coef_obj = cJSON_GetObjectItem(obj, "coef");
	uint32_t coef_size = cJSON_GetArraySize(coef_obj);

	BLOB_PROP_GLOBAL_DISABLE(obj);

	ltm_freq_decomp.decomp_enable = (bool)dtest_get_obj_int(obj, "decomp_Enable");

	get_uint16_list(coef_obj, ltm_freq_decomp.coef);

	ltm_freq_decomp.norm = drm_vs_get_ltm_norm(ltm_freq_decomp.coef, coef_size);

	drmModeCreatePropertyBlob(dev->fd, &ltm_freq_decomp, sizeof(struct drm_vs_ltm_freq_decomp),
				  &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_ltm_luma_adj(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_1d_lut ltm_luma_adj = { 0 };
	cJSON *data_obj = cJSON_GetObjectItem(obj, "data");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	ltm_luma_adj.entry_cnt = (uint32_t)dtest_get_obj_int(obj, "entry_cnt");
	get_uint32_list(data_obj, ltm_luma_adj.data);

	drmModeCreatePropertyBlob(dev->fd, &ltm_luma_adj, sizeof(struct drm_vs_1d_lut), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_ltm_grid(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_ltm_grid ltm_grid = { 0 };

	BLOB_PROP_GLOBAL_DISABLE(obj);

	ltm_grid.width = (uint16_t)dtest_get_obj_int(obj, "width");
	ltm_grid.height = (uint16_t)dtest_get_obj_int(obj, "height");
	ltm_grid.depth = (uint16_t)dtest_get_obj_int(obj, "depth");

	drmModeCreatePropertyBlob(dev->fd, &ltm_grid, sizeof(struct drm_vs_ltm_grid), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_ltm_af_filter(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_ltm_af_filter ltm_af_filter;
	cJSON *slope_obj = cJSON_GetObjectItem(obj, "slope");
	cJSON *bias_obj = cJSON_GetObjectItem(obj, "bias");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	ltm_af_filter.weight = (uint16_t)dtest_get_obj_int(obj, "weight");
	get_uint32_list(slope_obj, ltm_af_filter.slope);
	get_uint32_list(bias_obj, ltm_af_filter.bias);

	drmModeCreatePropertyBlob(dev->fd, &ltm_af_filter, sizeof(struct drm_vs_ltm_af_filter),
				  &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_ltm_af_slice(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_ltm_af_slice ltm_af_slice = { 0 };
	cJSON *start_pos_obj = cJSON_GetObjectItem(obj, "weight");
	cJSON *scale_obj = cJSON_GetObjectItem(obj, "scale");
	cJSON *scale_half_obj = cJSON_GetObjectItem(obj, "scale_half");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	get_uint16_list(start_pos_obj, ltm_af_slice.start_pos);
	get_uint16_list(scale_obj, ltm_af_slice.scale);
	get_uint16_list(scale_half_obj, ltm_af_slice.scale_half);

	drmModeCreatePropertyBlob(dev->fd, &ltm_af_slice, sizeof(struct drm_vs_ltm_af_slice),
				  &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_ltm_af_trans(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_ltm_af_trans ltm_af_trans = { 0 };
	cJSON *scale_obj = cJSON_GetObjectItem(obj, "scale");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	ltm_af_trans.slope_bit = (uint8_t)dtest_get_obj_int(obj, "slope_bit");
	ltm_af_trans.bias_bit = (uint8_t)dtest_get_obj_int(obj, "bias_bit");
	get_uint16_list(scale_obj, ltm_af_trans.scale);

	drmModeCreatePropertyBlob(dev->fd, &ltm_af_trans, sizeof(struct drm_vs_ltm_af_trans),
				  &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_ltm_tone_adj(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	uint32_t temp = 0;
	struct drm_vs_ltm_tone_adj ltm_tone_adj;
	cJSON *luma_from_obj = cJSON_GetObjectItem(obj, "luma_from");
	cJSON *entry_cnt_obj = cJSON_GetObjectItem(obj, "entry_cnt");
	cJSON *data_obj = cJSON_GetObjectItem(obj, "data");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	const struct enum_str_match_list luma_from_list[] = {
		{ 0, "LumaCalOut" },
		{ 1, "LocalAffineTransOut" },
	};

	if (luma_from_obj) {
		temp = string_to_enum(luma_from_list, ARRAY_SIZE(luma_from_list),
				      luma_from_obj->valuestring);
		ARRAY_ENUM_MATCH_FAIL(temp);
		ltm_tone_adj.luma_from = temp;
	} else
		ltm_tone_adj.luma_from = 1; /* LUMA default from Local Affine Transform Out */

	if (entry_cnt_obj)
		ltm_tone_adj.entry_cnt = (uint32_t)entry_cnt_obj->valueint;
	get_uint32_list(data_obj, ltm_tone_adj.data);

	drmModeCreatePropertyBlob(dev->fd, &ltm_tone_adj, sizeof(struct drm_vs_ltm_tone_adj),
				  &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_ltm_color(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_ltm_color ltm_color;
	cJSON *gain_obj = cJSON_GetObjectItem(obj, "gain");
	cJSON *luma_obj = cJSON_GetObjectItem(obj, "luma");
	cJSON *satu_obj = cJSON_GetObjectItem(obj, "satu");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	ltm_color.satu_ctrl = (bool)dtest_get_obj_int(obj, "satu_ctrl");
	ltm_color.luma_thresh = (uint16_t)dtest_get_obj_int(obj, "luma_thresh");
	get_uint16_list(gain_obj, ltm_color.gain);
	get_uint16_list(luma_obj, ltm_color.luma);
	get_uint16_list(satu_obj, ltm_color.satu);

	drmModeCreatePropertyBlob(dev->fd, &ltm_color, sizeof(struct drm_vs_ltm_color), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_ltm_dither(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_ltm_dither ltm_dither;
	cJSON *table_low_obj = cJSON_GetObjectItem(obj, "table_low");
	cJSON *table_high_obj = cJSON_GetObjectItem(obj, "table_high");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	ltm_dither.dither_enable = (bool)dtest_get_obj_int(obj, "ditherEnable");
	get_uint32_list(table_low_obj, ltm_dither.table_low);
	get_uint32_list(table_high_obj, ltm_dither.table_high);

	drmModeCreatePropertyBlob(dev->fd, &ltm_dither, sizeof(struct drm_vs_ltm_dither), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_ltm_luma_ave_set(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_ltm_luma_ave ltm_luma_set = { 0 };

	BLOB_PROP_GLOBAL_DISABLE(obj);

	ltm_luma_set.margin_x = (uint16_t)dtest_get_obj_int(obj, "margin_x");
	ltm_luma_set.margin_y = (uint16_t)dtest_get_obj_int(obj, "margin_y");
	ltm_luma_set.pixel_norm = (uint16_t)dtest_get_obj_int(obj, "pixel_norm");

	drmModeCreatePropertyBlob(dev->fd, &ltm_luma_set, sizeof(struct drm_vs_ltm_luma_ave),
				  &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_ltm_hist_cd_set(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_ltm_cd_set ltm_cd_set;
	cJSON *coef_obj = cJSON_GetObjectItem(obj, "coef");
	cJSON *thresh_obj = cJSON_GetObjectItem(obj, "thresh");
	cJSON *slope_obj = cJSON_GetObjectItem(obj, "slope");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	ltm_cd_set.enable = (bool)dtest_get_obj_int(obj, "ltm_cd_set_Enable");
	ltm_cd_set.overlap = (bool)dtest_get_obj_int(obj, "overlap");
	ltm_cd_set.min_wgt = (uint32_t)dtest_get_obj_int(obj, "min_wgt");
	ltm_cd_set.filt_norm = (uint32_t)dtest_get_obj_int(obj, "filt_norm");

	get_uint32_list(coef_obj, ltm_cd_set.coef);
	get_uint32_list(thresh_obj, ltm_cd_set.thresh);
	get_uint32_list(slope_obj, ltm_cd_set.slope);

	drmModeCreatePropertyBlob(dev->fd, &ltm_cd_set, sizeof(struct drm_vs_ltm_cd_set), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_ltm_local_hist_set(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_ltm_hist_set ltm_hist_set = { 0 };
	cJSON *start_pos_obj = cJSON_GetObjectItem(obj, "start_pos");
	cJSON *grid_scale_obj = cJSON_GetObjectItem(obj, "grid_scale");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	ltm_hist_set.overlap = (bool)dtest_get_obj_int(obj, "overlap");
	ltm_hist_set.grid_depth = (uint32_t)dtest_get_obj_int(obj, "grid_depth");

	get_uint32_list(start_pos_obj, ltm_hist_set.start_pos);
	get_uint32_list(grid_scale_obj, ltm_hist_set.grid_scale);

	drmModeCreatePropertyBlob(dev->fd, &ltm_hist_set, sizeof(struct drm_vs_ltm_hist_set),
				  &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_ltm_down_scaler(cJSON *obj, struct device *dev)
{
#define VS_LTM_DS_FIXED_BIT 21
	uint32_t blob_id = 0;
	uint32_t crop_w = 0, crop_h = 0;
	struct drm_vs_ltm_ds ltm_ds;

	BLOB_PROP_GLOBAL_DISABLE(obj);

	ltm_ds.crop_l = dtest_get_obj_int(obj, "crop_left");
	ltm_ds.crop_r = dtest_get_obj_int(obj, "crop_right");
	ltm_ds.crop_t = dtest_get_obj_int(obj, "crop_top");
	ltm_ds.crop_b = dtest_get_obj_int(obj, "crop_bottom");
	ltm_ds.output.w = dtest_get_obj_int(obj, "output_width");
	ltm_ds.output.h = dtest_get_obj_int(obj, "output_height");

	crop_w = ltm_ds.crop_r - ltm_ds.crop_l;
	crop_h = ltm_ds.crop_b - ltm_ds.crop_t;

	ltm_ds.h_norm = (uint32_t)((1 << VS_LTM_DS_FIXED_BIT) / (float)crop_w + 0.5f);
	ltm_ds.v_norm = (uint32_t)((1 << VS_LTM_DS_FIXED_BIT) / (float)crop_h + 0.5f);

	drmModeCreatePropertyBlob(dev->fd, &ltm_ds, sizeof(struct drm_vs_ltm_ds), &blob_id);
	return blob_id;

#undef VS_LTM_FREQ_NORM_FRAC_BIT
}

static uint64_t _map_drm_ltm_vcmd(cJSON *obj, struct device *dev)
{
	uint32_t i = 0, j = 0, temp1 = 0, temp2 = 0;
	uint32_t blob_id = 0;
	bool enable;
	uint32_t fr = dev->frame_id;
	uint32_t mode = 0, vid_mode = 0, Strength = 0, layerWidth = 0, layerHeight = 0;
	uint32_t gridWidth = 0, gridHeight = 0;
	uint32_t grid_num[2];
	struct drm_vs_ltm ltm;
	struct bo *ltm_hist_bo1 = NULL, *ltm_hist_bo2 = NULL;
	struct bo *ltm_hist_bo = NULL;
	uint32_t *hist_arr;
	static uint32_t hist_buf_first[144][64] = { 0 };
	static uint32_t cdf_curve_last[144][65] = { 0 };
	cJSON *Grid_num_obj = NULL;
	cJSON *vid_setting_obj = NULL;
	uint16_t spatial_kernal[6] = { 164, 101, 60, 23, 14, 3 };
	double sensitivity = 0.4, iir_max_wgt = 1.0;

	BLOB_PROP_GLOBAL_DISABLE(obj);

	enable = (bool)dtest_get_obj_int(obj, "ltm_enable");
	if (enable) {
		Grid_num_obj = cJSON_GetObjectItem(obj, "Grid_num");
		get_uint32_list(Grid_num_obj, grid_num);

		mode = (uint32_t)dtest_get_obj_int(obj, "Mode");
		vid_mode = (uint32_t)dtest_get_obj_int(obj, "VidMode");
		Strength = (uint32_t)dtest_get_obj_int(obj, "Strength");
		layerWidth = (uint32_t)dtest_get_obj_int(obj, "width");
		layerHeight = (uint32_t)dtest_get_obj_int(obj, "height");
		gridWidth = grid_num[0];
		gridHeight = grid_num[1];

		if ((vid_setting_obj = cJSON_GetObjectItem(obj, "VidSetting"))) {
			cJSON *spatial_coef_obj =
				cJSON_GetObjectItem(vid_setting_obj, "Spatial_Coef");
			get_uint16_list(spatial_coef_obj, spatial_kernal);
			sensitivity = dtest_get_obj_double(vid_setting_obj, "Sensity");
			iir_max_wgt = dtest_get_obj_double(vid_setting_obj, "IIR_Wgt_max");
		}

		if (dev->ltm_hist_bo1 == NULL && dev->ltm_hist_bo2 == NULL) {
			ltm_hist_bo1 = bo_create_dumb(dev->fd, 144, 32, 32);
			if (ltm_hist_bo1 == NULL)
				return 0;
			else
				dev->ltm_hist_bo1 = ltm_hist_bo1;

			ltm_hist_bo2 = bo_create_dumb(dev->fd, 144, 32, 32);
			if (ltm_hist_bo2 == NULL)
				return 0;
			else
				dev->ltm_hist_bo2 = ltm_hist_bo2;
		}

		/*query the exe frame count*/
		struct drm_vs_ctx arg = { .type = VS_CTX_QUERY };
		uint32_t frm_exe_count = 0;
		arg.handle = 1; /*the default ctx id, TODO: need refine*/
		drmIoctl(dev->fd, DRM_IOCTL_VS_SET_CTX, &arg);
		frm_exe_count = arg.frm_exe_count;

		if (frm_exe_count == fr && fr != 0) {
			printf("  [%s] update ltm!!\n", __func__);
			if ((fr - 1) % 2 == 0)
				ltm_hist_bo = dev->ltm_hist_bo1;
			else
				ltm_hist_bo = dev->ltm_hist_bo2;
			if (!ltm_hist_bo)
				return 0;
			if (bo_map(ltm_hist_bo, (void **)&hist_arr)) {
				fprintf(stderr, "failed to map buffer: %s\n", strerror(-errno));
				bo_destroy(ltm_hist_bo);
				return 0;
			}
			/***update histogram***/
			for (i = 0; i < 144; i++) {
				for (j = 0; j < 32; j++) {
					temp1 = hist_arr[i * 32 + j] & 0xffff;
					temp2 = hist_arr[i * 32 + j] >> 16;
					hist_buf_first[i][2 * j] = temp1;
					hist_buf_first[i][2 * j + 1] = temp2;
				}
			}
			bo_unmap(ltm_hist_bo);

			if (dev->dpu_algorithm->funcs->dpu_algo_ltm) {
				if (dev->dpu_algorithm->funcs->dpu_algo_ltm(
					    fr, mode, vid_mode, Strength, layerWidth, layerHeight,
					    gridWidth, gridHeight, sensitivity, iir_max_wgt,
					    spatial_kernal, hist_buf_first, cdf_curve_last,
					    &ltm) != 0) {
					printf("LTM algo function calculate faield!\n");
					/*TBD*/
				}
			} else {
				printf("Not found the LTM algo function!\n");
				return 0;
			}
		} else {
			printf("  [%s] config default!!\n", __func__);
			if (dev->dpu_algorithm->funcs->dpu_algo_ltm) {
				if (dev->dpu_algorithm->funcs->dpu_algo_ltm(
					    0, mode, vid_mode, Strength, layerWidth, layerHeight,
					    gridWidth, gridHeight, sensitivity, iir_max_wgt,
					    spatial_kernal, NULL, NULL, &ltm) != 0) {
					printf("LTM algo function calculate faield!\n");
					/*TBD*/
				}
			} else {
				printf("Not found the LTM algo function!\n");
				return 0;
			}
		}

		/*set hist get handle*/
		ltm.ltm_hist_get.enable = true;
		ltm.ltm_hist_get.fd = dev->fd;
		if (!dev->ltm_hist_bo1 || !dev->ltm_hist_bo2)
			return 0;
		if (fr % 2 == 0)
			ltm.ltm_hist_get.hist_bo_handle = dev->ltm_hist_bo1->handle;
		else
			ltm.ltm_hist_get.hist_bo_handle = dev->ltm_hist_bo2->handle;

		drmModeCreatePropertyBlob(dev->fd, &ltm, sizeof(struct drm_vs_ltm), &blob_id);
	}
	return blob_id;
}

static uint64_t _map_drm_ltm(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0, i = 0, j = 0, temp1 = 0, temp2 = 0;
	bool enable;
	uint32_t fr = dev->frame_id;
	uint32_t mode = 0, vid_mode = 0, Strength = 0, layerWidth = 0, layerHeight = 0;
	uint32_t gridWidth = 0, gridHeight = 0;
	uint32_t grid_num[2];
	static uint32_t hist_buf_first[144][64] = { 0 };
	static uint32_t hist_buf_second[144][64] = { 0 };
	static uint32_t cdf_curve_last[144][65] = { 0 };
	struct drm_vs_ltm ltm;
	struct bo *ltm_hist_bo = NULL;
	uint32_t *hist_arr;
	cJSON *Grid_num_obj = NULL;
	cJSON *vid_setting_obj = NULL;
	uint16_t spatial_kernal[6] = { 164, 101, 60, 23, 14, 3 };
	double sensitivity = 0.4, iir_max_wgt = 1.0;

	BLOB_PROP_GLOBAL_DISABLE(obj);

	enable = (bool)dtest_get_obj_int(obj, "ltm_enable");
	if (enable) {
		Grid_num_obj = cJSON_GetObjectItem(obj, "Grid_num");
		get_uint32_list(Grid_num_obj, grid_num);

		mode = (uint32_t)dtest_get_obj_int(obj, "Mode");
		vid_mode = (uint32_t)dtest_get_obj_int(obj, "VidMode");
		Strength = (uint32_t)dtest_get_obj_int(obj, "Strength");
		layerWidth = (uint32_t)dtest_get_obj_int(obj, "width");
		layerHeight = (uint32_t)dtest_get_obj_int(obj, "height");

		if ((vid_setting_obj = cJSON_GetObjectItem(obj, "VidSetting"))) {
			cJSON *spatial_coef_obj =
				cJSON_GetObjectItem(vid_setting_obj, "Spatial_Coef");
			get_uint16_list(spatial_coef_obj, spatial_kernal);
			sensitivity = dtest_get_obj_double(vid_setting_obj, "Sensity");
			iir_max_wgt = dtest_get_obj_double(vid_setting_obj, "IIR_Wgt_max");
		}

		if (dev->ltm_hist_bo1 == NULL) {
			ltm_hist_bo = bo_create_dumb(dev->fd, 144, 32, 32);
			if (ltm_hist_bo == NULL)
				return 0;
			else
				dev->ltm_hist_bo1 = ltm_hist_bo;
		} else {
			if (bo_map(dev->ltm_hist_bo1, (void **)&hist_arr)) {
				fprintf(stderr, "failed to map buffer: %s\n", strerror(-errno));
				bo_destroy(dev->ltm_hist_bo1);
				dev->ltm_hist_bo1 = NULL;
				return 0;
			}

			/***update histogram***/
			/*use last frame's Hist*/
			if (vid_mode == 0) {
				for (i = 0; i < 144; i++) {
					for (j = 0; j < 32; j++) {
						temp1 = hist_arr[i * 32 + j] & 0xffff;
						temp2 = hist_arr[i * 32 + j] >> 16;
						hist_buf_first[i][2 * j] = temp1;
						hist_buf_first[i][2 * j + 1] = temp2;
					}
				}
			}
			/*lag 2 frames*/
			else {
				for (i = 0; i < 144; i++) {
					for (j = 0; j < 32; j++) {
						temp1 = hist_arr[i * 32 + j] & 0xffff;
						temp2 = hist_arr[i * 32 + j] >> 16;
						hist_buf_first[i][2 * j] =
							hist_buf_second[i][2 * j];
						hist_buf_first[i][2 * j + 1] =
							hist_buf_second[i][2 * j + 1];
						hist_buf_second[i][2 * j] = temp1;
						hist_buf_second[i][2 * j + 1] = temp2;
					}
				}
			}
			bo_unmap(dev->ltm_hist_bo1);
		}

		gridWidth = grid_num[0];
		gridHeight = grid_num[1];
		if (dev->dpu_algorithm->funcs->dpu_algo_ltm) {
			if (dev->dpu_algorithm->funcs->dpu_algo_ltm(
				    fr, mode, vid_mode, Strength, layerWidth, layerHeight,
				    gridWidth, gridHeight, sensitivity, iir_max_wgt, spatial_kernal,
				    hist_buf_first, cdf_curve_last, &ltm) != 0) {
				printf("LTM algo function calculate faield!\n");
				/*TBD*/
			}
		} else {
			printf("Not found the LTM algo function!\n");
			if (dev->ltm_hist_bo1) {
				bo_destroy(dev->ltm_hist_bo1);
				dev->ltm_hist_bo1 = NULL;
			}
			return 0;
		}

		/*set hist get handle*/
		ltm.ltm_hist_get.enable = true;
		ltm.ltm_hist_get.fd = dev->fd;
		ltm.ltm_hist_get.hist_bo_handle = dev->ltm_hist_bo1->handle;

		drmModeCreatePropertyBlob(dev->fd, &ltm, sizeof(struct drm_vs_ltm), &blob_id);
	}
	return blob_id;
}

uint64_t MAPAPI map_drm_ltm(cJSON *obj, struct device *dev)
{
	if (dev->use_vcmd && !dev->is_qemu)
		return _map_drm_ltm_vcmd(obj, dev);
	return _map_drm_ltm(obj, dev);
}

uint64_t update_ltm_slope_bias(cJSON *obj, struct device *dev, uint32_t frame_id)
{
	uint32_t i = 0, j = 0, temp1 = 0, temp2 = 0;
	bool enable;
	uint32_t fr = frame_id;
	uint32_t mode = 0, vid_mode = 0, Strength = 0, layerWidth = 0, layerHeight = 0;
	uint32_t gridWidth = 0, gridHeight = 0;
	uint32_t grid_num[2];
	static uint32_t hist_buf_first[144][64] = { 0 };
	//static uint32_t hist_buf_second[144][64] = { 0 };
	static uint32_t cdf_curve_last[144][65] = { 0 };
	struct drm_vs_ltm ltm;
	struct bo *ltm_hist_bo = NULL;
	uint32_t *hist_arr;
	cJSON *Grid_num_obj = NULL;
	cJSON *vid_setting_obj = NULL;
	uint16_t spatial_kernal[6] = { 164, 101, 60, 23, 14, 3 };
	double sensitivity = 0.4, iir_max_wgt = 1.0;
	printf("[%s] update_ltm_slope_bias...\n", __func__);

	enable = (bool)dtest_get_obj_int(obj, "ltm_enable");
	if (enable) {
		Grid_num_obj = cJSON_GetObjectItem(obj, "Grid_num");
		get_uint32_list(Grid_num_obj, grid_num);

		mode = (uint32_t)dtest_get_obj_int(obj, "Mode");
		vid_mode = (uint32_t)dtest_get_obj_int(obj, "VidMode");
		//vid_mode = 0;
		Strength = (uint32_t)dtest_get_obj_int(obj, "Strength");
		layerWidth = (uint32_t)dtest_get_obj_int(obj, "width");
		layerHeight = (uint32_t)dtest_get_obj_int(obj, "height");

		if ((vid_setting_obj = cJSON_GetObjectItem(obj, "VidSetting"))) {
			cJSON *spatial_coef_obj =
				cJSON_GetObjectItem(vid_setting_obj, "Spatial_Coef");
			get_uint16_list(spatial_coef_obj, spatial_kernal);
			sensitivity = dtest_get_obj_double(vid_setting_obj, "Sensity");
			iir_max_wgt = dtest_get_obj_double(vid_setting_obj, "IIR_Wgt_max");
		}

		gridWidth = grid_num[0];
		gridHeight = grid_num[1];
		if (dev->dpu_algorithm->funcs->dpu_algo_ltm) {
			/*query the exe frame count*/
			struct drm_vs_ctx arg = { .type = VS_CTX_QUERY };
			uint32_t frm_exe_count = 0, frm_rdy_count = 0;
			arg.handle = 1; /*the default ctx id, TODO: need refine*/
			drmIoctl(dev->fd, DRM_IOCTL_VS_SET_CTX, &arg);
			frm_rdy_count = arg.frm_rdy_count;
			frm_exe_count = arg.frm_exe_count;
			if (frm_exe_count > fr + 1) {
				printf("[%s] the frm_exe_count(%d) exceeds current query frm(%d)!\n",
				       __func__, frm_exe_count, fr);
				return 0;
			}
			while (frm_exe_count < fr + 1) {
				drmIoctl(dev->fd, DRM_IOCTL_VS_SET_CTX, &arg);
				frm_rdy_count = arg.frm_rdy_count;
				frm_exe_count = arg.frm_exe_count;
			}
			printf("[%s]cur frm:%d, query frm_rdy_count:%d, frm_exe_count:%d\n",
			       __func__, fr, frm_rdy_count, frm_exe_count);
			if (frm_exe_count == fr + 1) {
				if (frm_rdy_count < fr + 3) {
					printf("[%s] update slope/bias to frame %d, but the frame has not chain to vcmd!\n",
					       __func__, fr + 2);
					return 0;
				}
				if (fr % 2 == 0)
					ltm_hist_bo = dev->ltm_hist_bo1;
				else
					ltm_hist_bo = dev->ltm_hist_bo2;
				if (bo_map(ltm_hist_bo, (void **)&hist_arr)) {
					fprintf(stderr, "failed to map buffer: %s\n",
						strerror(-errno));
					bo_destroy(ltm_hist_bo);
					return 0;
				}

				/***update histogram***/
				for (i = 0; i < 144; i++) {
					for (j = 0; j < 32; j++) {
						temp1 = hist_arr[i * 32 + j] & 0xffff;
						temp2 = hist_arr[i * 32 + j] >> 16;
						hist_buf_first[i][2 * j] = temp1;
						hist_buf_first[i][2 * j + 1] = temp2;
					}
				}
				bo_unmap(ltm_hist_bo);
				if (dev->dpu_algorithm->funcs->dpu_algo_ltm(
					    fr + 2, mode, vid_mode, Strength, layerWidth,
					    layerHeight, gridWidth, gridHeight, sensitivity,
					    iir_max_wgt, spatial_kernal, hist_buf_first,
					    cdf_curve_last, &ltm) != 0) {
					printf("[%s] LTM algo function calculate faield!\n",
					       __func__);
					/*TBD*/
				} else {
					printf("[%s] update slope and bias...\n", __func__);
					memcpy(arg.slope, ltm.af_filter.slope,
					       VS_LTM_AFFINE_LUT_NUM * sizeof(__u32));
					memcpy(arg.bias, ltm.af_filter.bias,
					       VS_LTM_AFFINE_LUT_NUM * sizeof(__u32));
					arg.hist_frm_id = fr;
					arg.type = VS_CTX_UPDATE_LTM;
					drmIoctl(dev->fd, DRM_IOCTL_VS_SET_CTX, &arg);
					frm_rdy_count = arg.frm_rdy_count;
					frm_exe_count = arg.frm_exe_count;
					printf("[%s] frm_rdy_count:%d, frm_exe_count:%d\n",
					       __func__, frm_rdy_count, frm_exe_count);
					if (arg.is_update_ltm == false)
						printf("[%s] update ltm slope and bias failed! ctx_id: %d, hist_frm_id:%d\n",
						       __func__, arg.handle, fr);
					else
						printf("[%s] update ltm slope and bias success! ctx_id: %d, hist_frm_id:%d, update_frm_id:%d\n",
						       __func__, arg.handle, fr, arg.update_frm_id);
				}
			} else {
				printf("[%s] can not get ltm hist, ctx_id: %d, frame_id: %d\n",
				       __func__, arg.handle, fr);
			}
		} else {
			printf("[%s] Not found the LTM algo function!\n", __func__);
			return 0;
		}
	}
	return 1;
}

uint64_t MAPAPI map_drm_gtm(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	/*bool enable;*/
	uint32_t fr = dev->frame_id;
	uint32_t mode = 0, vid_mode = 0, Strength = 0, layerWidth = 0, layerHeight = 0;
	struct drm_vs_gtm gtm;

	BLOB_PROP_GLOBAL_DISABLE(obj);

	/*enable = (bool)dtest_get_obj_int(obj, "ltm_enable");*/
	mode = (uint32_t)dtest_get_obj_int(obj, "Mode");
	vid_mode = (uint32_t)dtest_get_obj_int(obj, "VidMode");
	Strength = (uint32_t)dtest_get_obj_int(obj, "Strength");
	layerWidth = (uint32_t)dtest_get_obj_int(obj, "width");
	layerHeight = (uint32_t)dtest_get_obj_int(obj, "height");
	if (dev->dpu_algorithm->funcs->dpu_algo_gtm) {
		dev->dpu_algorithm->funcs->dpu_algo_gtm(fr, mode, vid_mode, Strength, layerWidth,
							layerHeight, &gtm);
	} else {
		printf("Not found the GTM algo function!\n");
		return 0;
	}

	drmModeCreatePropertyBlob(dev->fd, &gtm, sizeof(struct drm_vs_gtm), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_sharpness(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_sharpness sharp;
	cJSON *sharpEnable_obj = cJSON_GetObjectItem(obj, "sharpEnable");
	cJSON *inkMode_obj = cJSON_GetObjectItem(obj, "inkMode");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (sharpEnable_obj)
		sharp.enable = (bool)sharpEnable_obj->valueint;
	if (!inkMode_obj) {
		printf("not found inkMode config.\n");
		return 0;
	}
	char *inkModeStr = inkMode_obj->valuestring;
	if (!strcmp(inkModeStr, "DEFAULT")) {
		sharp.ink_mode = VS_SHARPNESS_INK_DEFAULT;
	} else if (!strcmp(inkModeStr, "G0")) {
		sharp.ink_mode = VS_SHARPNESS_INK_G0;
	} else if (!strcmp(inkModeStr, "G1")) {
		sharp.ink_mode = VS_SHARPNESS_INK_G1;
	} else if (!strcmp(inkModeStr, "G2")) {
		sharp.ink_mode = VS_SHARPNESS_INK_G2;
	} else if (!strcmp(inkModeStr, "L0")) {
		sharp.ink_mode = VS_SHARPNESS_INK_L0;
	} else if (!strcmp(inkModeStr, "L1")) {
		sharp.ink_mode = VS_SHARPNESS_INK_L1;
	} else if (!strcmp(inkModeStr, "L2")) {
		sharp.ink_mode = VS_SHARPNESS_INK_L2;
	} else if (!strcmp(inkModeStr, "ADAPT")) {
		sharp.ink_mode = VS_SHARPNESS_INK_ADAPT;
	} else if (!strcmp(inkModeStr, "V0")) {
		sharp.ink_mode = VS_SHARPNESS_INK_V0;
	} else if (!strcmp(inkModeStr, "V1")) {
		sharp.ink_mode = VS_SHARPNESS_INK_V1;
	} else if (!strcmp(inkModeStr, "V2")) {
		sharp.ink_mode = VS_SHARPNESS_INK_V2;
	} else if (!strcmp(inkModeStr, "LUMA")) {
		sharp.ink_mode = VS_SHARPNESS_INK_LUMA;
	}
	drmModeCreatePropertyBlob(dev->fd, &sharp, sizeof(struct drm_vs_sharpness), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_sharpness_csc(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_sharpness_csc sharp_csc;
	cJSON *r2y_coef_obj = cJSON_GetObjectItem(obj, "r2y_coef");
	cJSON *y2r_coef_obj = cJSON_GetObjectItem(obj, "y2r_coef");
	cJSON *r2y_offset_obj = cJSON_GetObjectItem(obj, "r2y_offset");
	cJSON *y2r_offset_obj = cJSON_GetObjectItem(obj, "r2y_offset");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	get_int_list(r2y_coef_obj, sharp_csc.r2y_coef);
	get_int_list(y2r_coef_obj, sharp_csc.y2r_coef);
	get_int_list(r2y_offset_obj, sharp_csc.r2y_offset);
	get_int_list(y2r_offset_obj, sharp_csc.y2r_offset);

	drmModeCreatePropertyBlob(dev->fd, &sharp_csc, sizeof(struct drm_vs_sharpness_csc),
				  &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_sharpness_luma_gain(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_sharpness_luma_gain sharp_luma_gain;
	cJSON *lut_obj = cJSON_GetObjectItem(obj, "Lut");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	get_int_list(lut_obj, sharp_luma_gain.lut);

	drmModeCreatePropertyBlob(dev->fd, &sharp_luma_gain,
				  sizeof(struct drm_vs_sharpness_luma_gain), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_sharpness_lpf(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	int sum_lpf0_coef, sum_lpf1_coef, sum_lpf2_coef;
	struct drm_vs_sharpness_lpf sharp_lpf;
	cJSON *lpf0_coef_obj = cJSON_GetObjectItem(obj, "lpf0_coef");
	cJSON *lpf1_coef_obj = cJSON_GetObjectItem(obj, "lpf1_coef");
	cJSON *lpf2_coef_obj = cJSON_GetObjectItem(obj, "lpf2_coef");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	get_uint32_list(lpf0_coef_obj, sharp_lpf.lpf0_coef);
	get_uint32_list(lpf1_coef_obj, sharp_lpf.lpf1_coef);
	get_uint32_list(lpf2_coef_obj, sharp_lpf.lpf2_coef);

	sum_lpf0_coef = 2 * sharp_lpf.lpf0_coef[0] + 2 * sharp_lpf.lpf0_coef[1] +
			2 * sharp_lpf.lpf0_coef[2] + sharp_lpf.lpf0_coef[3];
	sum_lpf1_coef = 2 * sharp_lpf.lpf1_coef[0] + 2 * sharp_lpf.lpf1_coef[1] +
			2 * sharp_lpf.lpf1_coef[2] + sharp_lpf.lpf1_coef[3];
	sum_lpf2_coef = 2 * sharp_lpf.lpf2_coef[0] + 2 * sharp_lpf.lpf2_coef[1] +
			2 * sharp_lpf.lpf2_coef[2] + sharp_lpf.lpf2_coef[3];
	sharp_lpf.lpf0_norm = (int)((1 << VS_SHARPNESS_LPF_NORM_BPP) / (float)sum_lpf0_coef + 0.5);
	sharp_lpf.lpf1_norm = (int)((1 << VS_SHARPNESS_LPF_NORM_BPP) / (float)sum_lpf1_coef + 0.5);
	sharp_lpf.lpf2_norm = (int)((1 << VS_SHARPNESS_LPF_NORM_BPP) / (float)sum_lpf2_coef + 0.5);

	drmModeCreatePropertyBlob(dev->fd, &sharp_lpf, sizeof(struct drm_vs_sharpness_lpf),
				  &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_sharpness_lpf_noise(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_sharpness_lpf_noise sharp_lpf_noise;
	cJSON *lut0_obj = cJSON_GetObjectItem(obj, "lut0");
	cJSON *lut1_obj = cJSON_GetObjectItem(obj, "lut1");
	cJSON *lut2_obj = cJSON_GetObjectItem(obj, "lut2");
	cJSON *luma_strength0_obj = cJSON_GetObjectItem(obj, "luma_strength0");
	cJSON *luma_strength1_obj = cJSON_GetObjectItem(obj, "luma_strength1");
	cJSON *luma_strength2_obj = cJSON_GetObjectItem(obj, "luma_strength2");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	get_uint32_list(lut0_obj, sharp_lpf_noise.lut0);
	get_uint32_list(lut1_obj, sharp_lpf_noise.lut1);
	get_uint32_list(lut2_obj, sharp_lpf_noise.lut2);

	if (luma_strength0_obj)
		sharp_lpf_noise.luma_strength0 = (uint32_t)luma_strength0_obj->valueint;
	if (luma_strength1_obj)
		sharp_lpf_noise.luma_strength1 = (uint32_t)luma_strength1_obj->valueint;
	if (luma_strength2_obj)
		sharp_lpf_noise.luma_strength2 = (uint32_t)luma_strength2_obj->valueint;

	drmModeCreatePropertyBlob(dev->fd, &sharp_lpf_noise,
				  sizeof(struct drm_vs_sharpness_lpf_noise), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_sharpness_lpf_curve(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_sharpness_lpf_curve sharp_lpf_curve;
	cJSON *lut0_obj = cJSON_GetObjectItem(obj, "lut0");
	cJSON *lut1_obj = cJSON_GetObjectItem(obj, "lut1");
	cJSON *lut2_obj = cJSON_GetObjectItem(obj, "lut2");
	cJSON *master_gain_obj = cJSON_GetObjectItem(obj, "master_gain");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	get_uint32_list(lut0_obj, sharp_lpf_curve.lut0);
	get_uint32_list(lut1_obj, sharp_lpf_curve.lut1);
	get_uint32_list(lut2_obj, sharp_lpf_curve.lut2);

	if (master_gain_obj)
		sharp_lpf_curve.master_gain = (uint32_t)master_gain_obj->valueint;

	drmModeCreatePropertyBlob(dev->fd, &sharp_lpf_curve,
				  sizeof(struct drm_vs_sharpness_lpf_curve), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_sharpness_color_adaptive(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	uint32_t i;
	bool ca_enable[VS_SHARPNESS_CA_MODE_NUM] = { false, false, false };
	uint32_t ca_param[VS_SHARPNESS_CA_MODE_NUM][VS_SHARPNESS_CA_PARAM_NUM];
	struct drm_vs_sharpness_color_adaptive sharp_color_adaptive;

	cJSON *ca0_enable_obj = cJSON_GetObjectItem(obj, "ca0_enable");
	cJSON *ca1_enable_obj = cJSON_GetObjectItem(obj, "ca1_enable");
	cJSON *ca2_enable_obj = cJSON_GetObjectItem(obj, "ca2_enable");
	cJSON *ca0_param_obj = cJSON_GetObjectItem(obj, "ca0_param");
	cJSON *ca1_param_obj = cJSON_GetObjectItem(obj, "ca1_param");
	cJSON *ca2_param_obj = cJSON_GetObjectItem(obj, "ca2_param");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (ca0_enable_obj)
		ca_enable[0] = (bool)ca0_enable_obj->valueint;
	if (ca1_enable_obj)
		ca_enable[1] = (bool)ca1_enable_obj->valueint;
	if (ca2_enable_obj)
		ca_enable[2] = (bool)ca2_enable_obj->valueint;
	get_uint32_list(ca0_param_obj, ca_param[0]);
	get_uint32_list(ca1_param_obj, ca_param[1]);
	get_uint32_list(ca2_param_obj, ca_param[2]);

	for (i = 0; i < VS_SHARPNESS_CA_MODE_NUM; i++) {
		sharp_color_adaptive.mode[i].enable = ca_enable[i];
		sharp_color_adaptive.mode[i].gain = ca_param[i][0];
		sharp_color_adaptive.mode[i].theta_center = ca_param[i][1];
		sharp_color_adaptive.mode[i].theta_range = ca_param[i][2];
		sharp_color_adaptive.mode[i].theta_slope = ca_param[i][3];
		sharp_color_adaptive.mode[i].radius_center = ca_param[i][4];
		sharp_color_adaptive.mode[i].radius_range = ca_param[i][5];
		sharp_color_adaptive.mode[i].radius_slope = ca_param[i][6];
	}

	drmModeCreatePropertyBlob(dev->fd, &sharp_color_adaptive,
				  sizeof(struct drm_vs_sharpness_color_adaptive), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_sharpness_color_boost(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_sharpness_color_boost sharp_color_boost;

	cJSON *pos_gain_obj = cJSON_GetObjectItem(obj, "pos_gain");
	cJSON *neg_gain_obj = cJSON_GetObjectItem(obj, "neg_gain");
	cJSON *y_offset_obj = cJSON_GetObjectItem(obj, "y_offset");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (pos_gain_obj)
		sharp_color_boost.pos_gain = (uint32_t)pos_gain_obj->valueint;
	if (neg_gain_obj)
		sharp_color_boost.neg_gain = (uint32_t)neg_gain_obj->valueint;
	if (y_offset_obj)
		sharp_color_boost.y_offset = (uint32_t)y_offset_obj->valueint;

	drmModeCreatePropertyBlob(dev->fd, &sharp_color_boost,
				  sizeof(struct drm_vs_sharpness_color_boost), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_sharpness_soft_clip(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_sharpness_soft_clip sharp_soft_clip;

	cJSON *pos_offset_obj = cJSON_GetObjectItem(obj, "pos_offset");
	cJSON *neg_offset_obj = cJSON_GetObjectItem(obj, "neg_offset");
	cJSON *pos_wet_obj = cJSON_GetObjectItem(obj, "pos_wet");
	cJSON *neg_wet_obj = cJSON_GetObjectItem(obj, "neg_wet");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (pos_offset_obj)
		sharp_soft_clip.pos_offset = (uint32_t)pos_offset_obj->valueint;
	if (neg_offset_obj)
		sharp_soft_clip.neg_offset = (uint32_t)neg_offset_obj->valueint;
	if (pos_wet_obj)
		sharp_soft_clip.pos_wet = (uint32_t)pos_wet_obj->valueint;
	if (neg_wet_obj)
		sharp_soft_clip.neg_wet = (uint32_t)neg_wet_obj->valueint;

	drmModeCreatePropertyBlob(dev->fd, &sharp_soft_clip,
				  sizeof(struct drm_vs_sharpness_soft_clip), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_sharpness_dither(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_sharpness_dither sharp_dither;
	uint32_t dth_r[4], dth_g[4], dth_b[4];

	cJSON *dth_r_obj = cJSON_GetObjectItem(obj, "dth_r");
	cJSON *dth_g_obj = cJSON_GetObjectItem(obj, "dth_g");
	cJSON *dth_b_obj = cJSON_GetObjectItem(obj, "dth_b");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	get_uint32_list(dth_r_obj, dth_r);
	get_uint32_list(dth_g_obj, dth_g);
	get_uint32_list(dth_b_obj, dth_b);

	sharp_dither.table_low[0] = dth_r[1] << 16 | dth_r[0];
	sharp_dither.table_high[0] = dth_r[3] << 16 | dth_r[2];
	sharp_dither.table_low[1] = dth_g[1] << 16 | dth_g[0];
	sharp_dither.table_high[1] = dth_g[3] << 16 | dth_g[2];
	sharp_dither.table_low[2] = dth_b[1] << 16 | dth_b[0];
	sharp_dither.table_high[2] = dth_b[3] << 16 | dth_b[2];

	drmModeCreatePropertyBlob(dev->fd, &sharp_dither, sizeof(struct drm_vs_sharpness_dither),
				  &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_blur(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_blur blur = { 0 };
	uint32_t sum_coef_r = 0, sum_coef_g = 0, sum_coef_b = 0;
	static uint8_t blur_coef_r[VS_BLUR_COEF_NUM] = { 1, 2, 3, 4 };
	static uint8_t blur_coef_g[VS_BLUR_COEF_NUM] = { 1, 2, 3, 4 };
	static uint8_t blur_coef_b[VS_BLUR_COEF_NUM] = { 1, 2, 3, 4 };

	cJSON *roi_obj = cJSON_GetObjectItem(obj, "ROI");
	cJSON *mask_obj = cJSON_GetObjectItem(obj, "MASK");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (roi_obj)
		fill_rect(roi_obj, &blur.roi);
	if (blur.roi.w < VS_BLUR_ROI_MIN_WIDTH || blur.roi.w > VS_BLUR_ROI_MAX_WIDTH) {
		printf("The blur ROI width is out of range.\n");
		return 0;
	}

	blur.coef_num = VS_BLUR_COEF_NUM;

	memcpy(blur.coef[0], blur_coef_r, blur.coef_num * sizeof(uint8_t));
	memcpy(blur.coef[1], blur_coef_g, blur.coef_num * sizeof(uint8_t));
	memcpy(blur.coef[2], blur_coef_b, blur.coef_num * sizeof(uint8_t));

	sum_coef_r =
		2 * blur.coef[0][0] + 2 * blur.coef[0][1] + 2 * blur.coef[0][2] + blur.coef[0][3];
	sum_coef_g =
		2 * blur.coef[1][0] + 2 * blur.coef[1][1] + 2 * blur.coef[1][2] + blur.coef[1][3];
	sum_coef_b =
		2 * blur.coef[2][0] + 2 * blur.coef[2][1] + 2 * blur.coef[2][2] + blur.coef[2][3];

	if ((sum_coef_r < VS_BLUR_COEF_SUM_MIN || sum_coef_r > VS_BLUR_COEF_SUM_MAX) ||
	    (sum_coef_g < VS_BLUR_COEF_SUM_MIN || sum_coef_g > VS_BLUR_COEF_SUM_MAX) ||
	    (sum_coef_b < VS_BLUR_COEF_SUM_MIN || sum_coef_b > VS_BLUR_COEF_SUM_MAX)) {
		printf("The sum of blur coef should in range[1, 64].\n");
		return 0;
	}

	blur.norm[0] = (uint32_t)((1 << VS_BLUR_NORM_BPP) / (float)sum_coef_r + 0.5f);
	blur.norm[1] = (uint32_t)((1 << VS_BLUR_NORM_BPP) / (float)sum_coef_g + 0.5f);
	blur.norm[2] = (uint32_t)((1 << VS_BLUR_NORM_BPP) / (float)sum_coef_b + 0.5f);

	blur.mask_fb_id = fill_drm_mask(mask_obj, dev);

	drmModeCreatePropertyBlob(dev->fd, &blur, sizeof(struct drm_vs_blur), &blob_id);
	return blob_id;
}

static void map_drm_rcd_bg(cJSON *obj, struct drm_vs_rcd_bg *background)
{
	cJSON *rcdBgPnlColor_obj = cJSON_GetObjectItem(obj, "rcdBgPnlColor");
	cJSON *rcdBgRoiEnable_obj = cJSON_GetObjectItem(obj, "rcdBgRoiEnable");

	if (rcdBgPnlColor_obj)
		background->pnl_color = (uint32_t)rcdBgPnlColor_obj->valueint;

	if (rcdBgRoiEnable_obj) {
		background->roi_enable = !!(rcdBgRoiEnable_obj->valueint);

		cJSON *rcdBgRoiColor_obj = cJSON_GetObjectItem(obj, "rcdBgRoiColor");
		cJSON *rcdBgRoiX_obj = cJSON_GetObjectItem(obj, "rcdBgRoiX");
		cJSON *rcdBgRoiY_obj = cJSON_GetObjectItem(obj, "rcdBgRoiY");
		cJSON *rcdBgRoiWidth_obj = cJSON_GetObjectItem(obj, "rcdBgRoiWidth");
		cJSON *rcdBgRoiHeight_obj = cJSON_GetObjectItem(obj, "rcdBgRoiHeight");

		background->roi_color = (uint32_t)rcdBgRoiColor_obj->valueint;
		background->roi_rect.x = (uint16_t)rcdBgRoiX_obj->valueint;
		background->roi_rect.y = (uint16_t)rcdBgRoiY_obj->valueint;
		background->roi_rect.w = (uint16_t)rcdBgRoiWidth_obj->valueint;
		background->roi_rect.h = (uint16_t)rcdBgRoiHeight_obj->valueint;
	}
}

static void map_drm_rcd_roi(cJSON *obj, struct drm_vs_rcd_roi *rcd_roi)
{
	cJSON *rcdTopRoiEnable_obj = cJSON_GetObjectItem(obj, "rcdTopRoiEnable");
	cJSON *rcdBtmRoiEnable_obj = cJSON_GetObjectItem(obj, "rcdBtmRoiEnable");

	rcd_roi->top_enable = 0;
	rcd_roi->btm_enable = 0;

	if (rcdTopRoiEnable_obj) {
		rcd_roi->top_enable = !!(rcdTopRoiEnable_obj->valueint);

		cJSON *rcdTopRoiX_obj = cJSON_GetObjectItem(obj, "rcdTopRoiX");
		cJSON *rcdTopRoiY_obj = cJSON_GetObjectItem(obj, "rcdTopRoiY");
		cJSON *rcdTopRoiWidth_obj = cJSON_GetObjectItem(obj, "rcdTopRoiWidth");
		cJSON *rcdTopRoiHeight_obj = cJSON_GetObjectItem(obj, "rcdTopRoiHeight");

		rcd_roi->top_roi.x = (uint16_t)rcdTopRoiX_obj->valueint;
		rcd_roi->top_roi.y = (uint16_t)rcdTopRoiY_obj->valueint;
		rcd_roi->top_roi.w = (uint16_t)rcdTopRoiWidth_obj->valueint;
		rcd_roi->top_roi.h = (uint16_t)rcdTopRoiHeight_obj->valueint;
	} else
		rcd_roi->top_enable = false;

	if (rcdBtmRoiEnable_obj) {
		rcd_roi->btm_enable = !!(rcdBtmRoiEnable_obj->valueint);

		cJSON *rcdBtmRoiX_obj = cJSON_GetObjectItem(obj, "rcdBtmRoiX");
		cJSON *rcdBtmRoiY_obj = cJSON_GetObjectItem(obj, "rcdBtmRoiY");
		cJSON *rcdBtmRoiWidth_obj = cJSON_GetObjectItem(obj, "rcdBtmRoiWidth");
		cJSON *rcdBtmRoiHeight_obj = cJSON_GetObjectItem(obj, "rcdBtmRoiHeight");

		rcd_roi->btm_roi.x = (uint16_t)rcdBtmRoiX_obj->valueint;
		rcd_roi->btm_roi.y = (uint16_t)rcdBtmRoiY_obj->valueint;
		rcd_roi->btm_roi.w = (uint16_t)rcdBtmRoiWidth_obj->valueint;
		rcd_roi->btm_roi.h = (uint16_t)rcdBtmRoiHeight_obj->valueint;
	} else
		rcd_roi->btm_enable = false;
}

uint64_t map_drm_rcd(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_rcd rcd = { 0 };
	cJSON *background_obj = cJSON_GetObjectItem(obj, "BACKGROUND");
	cJSON *roi_obj = cJSON_GetObjectItem(obj, "ROI");
	cJSON *mask_obj = cJSON_GetObjectItem(obj, "MASK");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (background_obj)
		map_drm_rcd_bg(background_obj, &rcd.background);
	if (roi_obj)
		map_drm_rcd_roi(roi_obj, &rcd.roi);
	if (mask_obj) {
		rcd.mask.mask_fb_id = fill_drm_mask(mask_obj, dev);
		rcd.mask.type = VS_MASK_BLD_INV;
	}

	drmModeCreatePropertyBlob(dev->fd, &rcd, sizeof(struct drm_vs_rcd), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_brightness(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_brightness brightness = { 0 };
	uint16_t brt_luma_coef[3] = { 256, 256, 512 };
	cJSON *target_obj = cJSON_GetObjectItem(obj, "brtTargetGain");
	cJSON *threshold_obj = cJSON_GetObjectItem(obj, "protThreshold");
	cJSON *mode_obj = cJSON_GetObjectItem(obj, "lumaMode");
	cJSON *roi0_obj = cJSON_GetObjectItem(obj, "ROI0");
	cJSON *roi1_obj = cJSON_GetObjectItem(obj, "ROI1");
	cJSON *mask_obj = cJSON_GetObjectItem(obj, "MASK");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	/* initialized the brightness mode */
	brightness.mask.mode = VS_BRIGHTNESS_CAL_MODE_COUNT;

	if (target_obj)
		brightness.target = (uint16_t)target_obj->valueint;
	if (threshold_obj)
		brightness.threshold = (uint16_t)threshold_obj->valueint;

	if (mode_obj) {
		if (!strcmp(mode_obj->valuestring, "WEIGHT"))
			brightness.mask.mode = VS_BRIGHTNESS_CAL_MODE_WEIGHT;
		else if (!strcmp(mode_obj->valuestring, "MAX"))
			brightness.mask.mode = VS_BRIGHTNESS_CAL_MODE_MAX;
		else
			brightness.mask.mode = VS_BRIGHTNESS_CAL_MODE_COUNT;
	}

	if (brightness.mask.mode == VS_BRIGHTNESS_CAL_MODE_MAX)
		memset(brightness.luma_coef, 0, 3 * sizeof(uint16_t));
	else
		memcpy(brightness.luma_coef, brt_luma_coef, 3 * sizeof(uint16_t));

	if (roi0_obj) {
		cJSON *roi0_enable_obj = cJSON_GetObjectItem(roi0_obj, "enable");
		if (roi0_enable_obj)
			brightness.roi0.roi_enable = roi0_enable_obj->valueint;
		fill_rect(roi0_obj, &brightness.roi0.rect);
	}

	if (roi1_obj) {
		cJSON *roi1_enable_obj = cJSON_GetObjectItem(roi1_obj, "enable");
		if (roi1_enable_obj)
			brightness.roi1.roi_enable = roi1_enable_obj->valueint;
		fill_rect(roi1_obj, &brightness.roi1.rect);
	}

	if (mask_obj)
		brightness.mask.mask_fb_id = fill_drm_mask(mask_obj, dev);

	drmModeCreatePropertyBlob(dev->fd, &brightness, sizeof(struct drm_vs_brightness), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_pvric_clear(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_pvric_clear pvric_clear;
	uint32_t low0 = 0, low1 = 0, high0 = 0, high1 = 0;

	cJSON *decClearLow0_obj = cJSON_GetObjectItem(obj, "decClearLow0");
	cJSON *decClearHigh0_obj = cJSON_GetObjectItem(obj, "decClearHigh0");
	cJSON *decClearLow1_obj = cJSON_GetObjectItem(obj, "decClearLow1");
	cJSON *decClearHigh1_obj = cJSON_GetObjectItem(obj, "decClearHigh1");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (decClearLow0_obj)
		low0 = decClearLow0_obj->valueint;

	if (decClearHigh0_obj)
		high0 = decClearHigh0_obj->valueint;

	if (decClearLow1_obj)
		low1 = decClearLow1_obj->valueint;

	if (decClearHigh1_obj)
		high1 = decClearHigh1_obj->valueint;

	pvric_clear.color[0] = (((uint64_t)high0 << 32) | low0) & 0xFFFFFFFFFFFFFFFF;
	pvric_clear.color[1] = (((uint64_t)high1 << 32) | low1) & 0xFFFFFFFFFFFFFFFF;

	drmModeCreatePropertyBlob(dev->fd, &pvric_clear, sizeof(struct drm_vs_pvric_clear),
				  &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_pvric_const(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_pvric_const pvric_const;
	uint32_t dec_const_cfg[2] = { 0, 0 };
	bool is_yuv = false;

	cJSON *decConstColor0_obj = cJSON_GetObjectItem(obj, "decConstColor0");
	cJSON *decConstColor1_obj = cJSON_GetObjectItem(obj, "decConstColor1");
	cJSON *is_yuv_obj = cJSON_GetObjectItem(obj, "is_yuv");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (decConstColor0_obj)
		dec_const_cfg[0] = decConstColor0_obj->valueint;
	if (decConstColor1_obj)
		dec_const_cfg[1] = decConstColor1_obj->valueint;
	if (is_yuv_obj)
		is_yuv = !!(is_yuv_obj->valueint);

	pvric_const.color[0] = vs_dpu_color_to_struct(dec_const_cfg[0], is_yuv);
	pvric_const.color[1] = vs_dpu_color_to_struct(dec_const_cfg[1], is_yuv);

	drmModeCreatePropertyBlob(dev->fd, &pvric_const, sizeof(struct drm_vs_pvric_const),
				  &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_dither(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_dither dither;
	cJSON *index_type_obj = cJSON_GetObjectItem(obj, "index_type");
	cJSON *sw_index_obj = cJSON_GetObjectItem(obj, "sw_index");
	cJSON *table_low_obj = cJSON_GetObjectItem(obj, "table_low");
	cJSON *table_high_obj = cJSON_GetObjectItem(obj, "table_high");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (index_type_obj)
		dither.index_type = index_type_obj->valueint;

	if (sw_index_obj && dither.index_type == VS_DTH_FRM_IDX_SW)
		dither.sw_index = (uint8_t)sw_index_obj->valueint;

	get_uint32_list(table_low_obj, dither.table_low);
	get_uint32_list(table_high_obj, dither.table_high);

	drmModeCreatePropertyBlob(dev->fd, &dither, sizeof(struct drm_vs_dither), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_panel_dither(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_dither dither;
	dither.frm_mode = VS_DTH_FRM_4; /* the frame mode default set VS_DTH_FRM_4 */
	cJSON *index_type_obj = cJSON_GetObjectItem(obj, "index_type");
	cJSON *sw_index_obj = cJSON_GetObjectItem(obj, "sw_index");
	cJSON *table_low_obj = cJSON_GetObjectItem(obj, "table_low");
	cJSON *table_high_obj = cJSON_GetObjectItem(obj, "table_high");
	cJSON *frm_mode_obj = cJSON_GetObjectItem(obj, "frm_mode");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (index_type_obj)
		dither.index_type = index_type_obj->valueint;

	get_uint32_list(table_low_obj, dither.table_low);
	get_uint32_list(table_high_obj, dither.table_high);

	if (frm_mode_obj)
		dither.frm_mode = frm_mode_obj->valueint;

	if (sw_index_obj && dither.index_type == VS_DTH_FRM_IDX_SW) {
		dither.sw_index = (uint8_t)sw_index_obj->valueint;
	}

	drmModeCreatePropertyBlob(dev->fd, &dither, sizeof(struct drm_vs_dither), &blob_id);
	printf("Create a blob, id=%u\n", blob_id);
	return blob_id;
}

uint64_t MAPAPI map_llv_dither(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_llv_dither dither;
	cJSON *index_type_obj = cJSON_GetObjectItem(obj, "index_type");
	cJSON *sw_index_obj = cJSON_GetObjectItem(obj, "sw_index");
	cJSON *start_x_obj = cJSON_GetObjectItem(obj, "start_x");
	cJSON *start_y_obj = cJSON_GetObjectItem(obj, "start_y");
	cJSON *mask_obj = cJSON_GetObjectItem(obj, "mask");
	cJSON *threshold_obj = cJSON_GetObjectItem(obj, "threshold");
	cJSON *linear_threshold_obj = cJSON_GetObjectItem(obj, "linear_threshold");
	cJSON *seed_obj = cJSON_GetObjectItem(obj, "seed");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (index_type_obj)
		dither.index_type = index_type_obj->valueint;
	if (sw_index_obj)
		dither.sw_index = (uint8_t)sw_index_obj->valueint;
	if (start_x_obj)
		dither.start_x = (uint16_t)start_x_obj->valueint;
	if (start_y_obj)
		dither.start_y = (uint16_t)start_y_obj->valueint;
	if (mask_obj)
		dither.mask = (uint16_t)mask_obj->valueint;
	if (threshold_obj)
		dither.threshold = (uint16_t)threshold_obj->valueint;
	if (linear_threshold_obj)
		dither.linear_threshold = (uint16_t)linear_threshold_obj->valueint;

	fill_random_dither_seed(seed_obj, &dither.seed);

	drmModeCreatePropertyBlob(dev->fd, &dither, sizeof(struct drm_vs_llv_dither), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_blender_dither(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_blender_dither dither = { 0 };
	cJSON *index_type_obj = cJSON_GetObjectItem(obj, "index_type");
	cJSON *sw_index_obj = cJSON_GetObjectItem(obj, "sw_index");
	cJSON *noise_obj = cJSON_GetObjectItem(obj, "noise");
	cJSON *start_x_obj = cJSON_GetObjectItem(obj, "start_x");
	cJSON *start_y_obj = cJSON_GetObjectItem(obj, "start_y");
	cJSON *mask_obj = cJSON_GetObjectItem(obj, "mask");
	cJSON *seed_obj = cJSON_GetObjectItem(obj, "seed");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (index_type_obj)
		dither.index_type = index_type_obj->valueint;

	if (dither.index_type == VS_DTH_FRM_IDX_SW) {
		if (!sw_index_obj) {
			printf("not found sw_index config.\n");
			return 0;
		}
		dither.sw_index = (uint8_t)sw_index_obj->valueint;
	}

	if (noise_obj)
		dither.noise = (uint8_t)noise_obj->valueint;
	if (start_x_obj)
		dither.start_x = (uint16_t)start_x_obj->valueint;
	if (start_y_obj)
		dither.start_y = (uint16_t)start_y_obj->valueint;
	if (mask_obj)
		dither.mask = (uint16_t)mask_obj->valueint;

	fill_random_dither_seed(seed_obj, &dither.seed);
	drmModeCreatePropertyBlob(dev->fd, &dither, sizeof(struct drm_vs_blender_dither), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_wb_point(cJSON *obj, struct device *dev)
{
	enum drm_vs_wb_point wb_point = VS_WB_DISP_OUT;

	cJSON *wb_point_obj = NULL;

	wb_point_obj = cJSON_GetObjectItem(obj, "wbPoint");

	if (!wb_point_obj) {
		printf("not found wbPoint config.\n");
		return 0;
	}

	if (!strcmp(wb_point_obj->valuestring, "displayIn"))
		wb_point = VS_WB_DISP_IN;
	else if (!strcmp(wb_point_obj->valuestring, "displayOut"))
		wb_point = VS_WB_DISP_OUT;
	else if (!strcmp(wb_point_obj->valuestring, "colorCaliOut"))
		wb_point = VS_WB_DISP_CC;
	else if (!strcmp(wb_point_obj->valuestring, "DSCOut"))
		wb_point = VS_WB_DISP_DSC_OUT;
	else if (!strcmp(wb_point_obj->valuestring, "ofifoIn"))
		wb_point = VS_WB_OFIFO_IN;
	else if (!strcmp(wb_point_obj->valuestring, "ofifoOut"))
		wb_point = VS_WB_OFIFO_OUT;

	return (uint64_t)wb_point;
}

uint64_t MAPAPI map_wb_stripe_position(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_position pos = { 0 };

	cJSON *pos_x_obj = cJSON_GetObjectItem(obj, "X");
	cJSON *pos_y_obj = cJSON_GetObjectItem(obj, "Y");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (pos_x_obj)
		pos.x = pos_x_obj->valueint;
	if (pos_y_obj)
		pos.y = pos_y_obj->valueint;

	drmModeCreatePropertyBlob(dev->fd, &pos, sizeof(struct drm_vs_position), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_wb_dither(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_wb_dither dither = { 0 };

	cJSON *index_type_obj = cJSON_GetObjectItem(obj, "index_type");
	cJSON *frm_mode_obj = cJSON_GetObjectItem(obj, "frm_mode");
	cJSON *table_low_obj = cJSON_GetObjectItem(obj, "table_low");
	cJSON *table_high_obj = cJSON_GetObjectItem(obj, "table_high");
	cJSON *sw_index_obj = cJSON_GetObjectItem(obj, "sw_index");

	BLOB_PROP_GLOBAL_DISABLE(obj);
	if (index_type_obj)
		dither.index_type = index_type_obj->valueint;

	get_uint32_list(table_low_obj, dither.table_low);
	get_uint32_list(table_high_obj, dither.table_high);

	if (dither.index_type == VS_DTH_FRM_IDX_SW && sw_index_obj) {
		dither.sw_index = (uint8_t)sw_index_obj->valueint;
	}

	if (sw_index_obj == NULL)
		dither.frm_mode = 4;
	else {
		if (frm_mode_obj)
			dither.frm_mode = frm_mode_obj->valueint;
	}

	drmModeCreatePropertyBlob(dev->fd, &dither, sizeof(struct drm_vs_wb_dither), &blob_id);
	printf("Create a blob, id=%u\n", blob_id);
	return blob_id;
}

uint64_t MAPAPI map_wb_rotation(cJSON *obj, struct device *dev)
{
	uint32_t rotation_mode = 0;
	rotation_mode = dtest_get_obj_rotation(obj, "rotation");

	return rotation_mode;
}

uint64_t MAPAPI map_color_calibration_path_mode(cJSON *obj, struct device *dev)
{
	enum drm_vs_color_calibration_path_mode path_mode = obj->valueint;

	BLOB_PROP_GLOBAL_DISABLE(obj);

	return path_mode;
}

uint64_t MAPAPI map_ds_config(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_ds_config config;
	cJSON *h_mode_obj = cJSON_GetObjectItem(obj, "h_mode");
	cJSON *v_mode_obj = cJSON_GetObjectItem(obj, "v_mode");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (h_mode_obj) {
		if (!strcmp(h_mode_obj->valuestring, "DROP"))
			config.h_mode = VS_DS_DROP;
		else if (!strcmp(h_mode_obj->valuestring, "AVERAGE"))
			config.h_mode = VS_DS_AVERAGE;
		else if (!strcmp(h_mode_obj->valuestring, "FILTER"))
			config.h_mode = VS_DS_FILTER;
	}

	if (v_mode_obj) {
		if (!strcmp(v_mode_obj->valuestring, "DROP"))
			config.v_mode = VS_DS_DROP;
		else if (!strcmp(v_mode_obj->valuestring, "AVERAGE"))
			config.v_mode = VS_DS_AVERAGE;
		else if (!strcmp(v_mode_obj->valuestring, "FILTER"))
			config.v_mode = VS_DS_FILTER;
	}

	drmModeCreatePropertyBlob(dev->fd, &config, sizeof(struct drm_vs_ds_config), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_scale_config(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_scale_config scale;
	int16_t temp_coef[VS_SCALE_HORI_COEF_NUM * 2] = { 0 };
	/* when scale_factor_set = vsFALSE: the result is close to opencv's result, but not exactly match */
	bool scale_factor_set = (bool)1;
	uint8_t tap_h = 0;
	uint8_t tap_v = 0;
	enum drm_vs_scale_mode scaler_mode = VS_SCALER_MODE_COUNT;
	/* filter type from info */
	uint8_t filter_type_mask = 0;
	enum drm_vs_filter_type filter_type = VS_H9_V5;

	cJSON *src_w_obj = cJSON_GetObjectItem(obj, "src_w");
	cJSON *src_h_obj = cJSON_GetObjectItem(obj, "src_h");
	cJSON *dst_w_obj = cJSON_GetObjectItem(obj, "dst_w");
	cJSON *dst_h_obj = cJSON_GetObjectItem(obj, "dst_h");
	cJSON *filter_obj = cJSON_GetObjectItem(obj, "filter");
	cJSON *mode_obj = cJSON_GetObjectItem(obj, "mode");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (!src_w_obj || !src_h_obj || !dst_w_obj || !dst_h_obj || !filter_obj) {
		printf("config of SCALE is incomplete.\n");
		return 0;
	}
	scale.src_w = src_w_obj->valueint;
	scale.src_h = src_h_obj->valueint;
	scale.dst_w = dst_w_obj->valueint;
	scale.dst_h = dst_h_obj->valueint;

	/* get filter_type from case */
	if (!filter_obj) {
		printf("not found filter config.\n");
		return 0;
	} else {
		if (!strcmp(filter_obj->valuestring, "H9V5")) {
			scale.filter = VS_H9_V5;
		} else if (!strcmp(filter_obj->valuestring, "H5V3")) {
			scale.filter = VS_H5_V3;
		} else if (!strcmp(filter_obj->valuestring, "H3V3")) {
			scale.filter = VS_H3_V3;
		} else if (!strcmp(filter_obj->valuestring, "H8V4")) {
			scale.filter = VS_H8_V4;
		} else {
			scale.filter = VS_H9_V5;
		}
	}

	/* get filter_type from info */
	filter_type_mask = (enum drm_vs_filter_type)dev->resources->crtcs->info.filter_type_mask;

	if (!(filter_type_mask & BIT(scale.filter))) {
		printf("-------------------------------------------------------\n");
		filter_type = drm_vs_get_info_filter_type(filter_type_mask);
		printf("Change the filter type from %s to %s.\n",
		       drm_vs_get_info_filter_name(scale.filter),
		       drm_vs_get_info_filter_name(filter_type));

		printf("-------------------------------------------------------\n");
		scale.filter = filter_type;
	}

	if (scale.filter == VS_H8_V4) {
		if (!mode_obj)
			scaler_mode = VS_SCALER_BI_LINEAR;
		else {
			if (!strcmp(mode_obj->valuestring, "LINEAR"))
				scaler_mode = VS_SCALER_BI_LINEAR;
			else if (!strcmp(mode_obj->valuestring, "CUBIC"))
				scaler_mode = VS_SCALER_BI_CUBIC;
			else if (!strcmp(mode_obj->valuestring, "LANCZOS"))
				scaler_mode = VS_SCALER_LANCZOS;
		}
	}

	/* get filter tap */
	drm_vs_get_filter_tap(scale.filter, &tap_h, &tap_v);

	if (scale.filter != VS_H8_V4) {
		scale.factor_x =
			drm_vs_get_stretch_factor(scale.src_w, scale.dst_w, scale_factor_set);
		scale.factor_y =
			drm_vs_get_stretch_factor(scale.src_h, scale.dst_h, scale_factor_set);

		/*horizontal scale coefficient data for dc8200 and dc9x00*/
		drm_vs_calculate_sync_table(tap_h, scale.src_w, scale.dst_w, temp_coef,
					    scale.filter);

		memcpy(scale.coef_h, temp_coef, VS_SUBPIXELLOADCOUNT * 9 * sizeof(int16_t));

		if (scale.filter == VS_H9_V5) {
			/*vertical scale coefficient data for dc9x00*/
			drm_vs_calculate_sync_table(tap_v, scale.src_h, scale.dst_h, temp_coef,
						    scale.filter);
			memcpy(scale.coef_v, temp_coef, VS_SUBPIXELLOADCOUNT * 5 * sizeof(int16_t));
			scale.initial_offsetx =
				drm_vs_get_stretch_initOffset(scale.factor_x, scale_factor_set);
			scale.initial_offsety =
				drm_vs_get_stretch_initOffset(scale.factor_y, scale_factor_set);
		} else {
			/*vertical scale coefficient data for dc8200*/
			drm_vs_calculate_sync_table(tap_v, scale.src_h, scale.dst_h, temp_coef,
						    scale.filter);
			memcpy(scale.coef_v, temp_coef, VS_SUBPIXELLOADCOUNT * 9 * sizeof(int16_t));
		}
	} else {
		if (dev->dpu_algorithm->funcs->dpu_algo_scl) {
			dev->dpu_algorithm->funcs->dpu_algo_scl((uint32_t)scaler_mode, scale.src_w,
								scale.src_h, scale.dst_w,
								scale.dst_h, &scale);
		} else {
			printf("Not found the SCALER algo function!\n");
			return 0;
		}
	}

	drmModeCreatePropertyBlob(dev->fd, &scale, sizeof(struct drm_vs_scale_config), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_r2y_config(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_r2y_config r2y = { 0 };
	int temp = 0;
	cJSON *mode_obj = cJSON_GetObjectItem(obj, "mode");
	cJSON *gamut_obj = cJSON_GetObjectItem(obj, "gamut");
	cJSON *coef_obj = cJSON_GetObjectItem(obj, "coef");
	cJSON *bus_format_obj = cJSON_GetObjectItem(obj, "busFormat");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	const struct enum_str_match_list bus_format_list[] = {
		{ MEDIA_BUS_FMT_UYVY8_1X16, "YUV422_8BIT" },
		{ MEDIA_BUS_FMT_UYVY10_1X20, "YUV422_10BIT" },
		{ MEDIA_BUS_FMT_YUV8_1X24, "YUV444_8BIT" },
		{ MEDIA_BUS_FMT_YUV10_1X30, "YUV444_10BIT" },
	};

	r2y.output_bus_format =
		MEDIA_BUS_FMT_YUV10_1X30; /* the default output bus format is YUV444_10BIT, while r2y enabled */
	if (bus_format_obj) {
		temp = string_to_enum(bus_format_list, ARRAY_SIZE(bus_format_list),
				      bus_format_obj->valuestring);
		ARRAY_ENUM_MATCH_FAIL(temp);
		r2y.output_bus_format = temp;
	}

	if (!mode_obj) {
		printf("not found mode for R2Y config.\n");
		return 0;
	}

	if (!gamut_obj) {
		if (strcmp(mode_obj->valuestring, "USR")) {
			printf("not found gamut for R2Y config.\n");
			return 0;
		}
	} else {
		if (!strcmp(gamut_obj->valuestring, "BT601"))
			r2y.gamut = VS_CSC_CG_601;
		else if (!strcmp(gamut_obj->valuestring, "BT709"))
			r2y.gamut = VS_CSC_CG_709;
		else if (!strcmp(gamut_obj->valuestring, "BT2020"))
			r2y.gamut = VS_CSC_CG_2020;
		else if (!strcmp(gamut_obj->valuestring, "DCIP3"))
			r2y.gamut = VS_CSC_CG_P3;
		else if (!strcmp(gamut_obj->valuestring, "SRGB"))
			r2y.gamut = VS_CSC_CG_SRGB;
	}

	if (!strcmp(mode_obj->valuestring, "USR"))
		r2y.mode = VS_CSC_CM_USR;
	else if (!strcmp(mode_obj->valuestring, "L2L"))
		r2y.mode = VS_CSC_CM_L2L;
	else if (!strcmp(mode_obj->valuestring, "L2F"))
		r2y.mode = VS_CSC_CM_L2F;
	else if (!strcmp(mode_obj->valuestring, "F2L"))
		r2y.mode = VS_CSC_CM_F2L;
	else if (!strcmp(mode_obj->valuestring, "F2F"))
		r2y.mode = VS_CSC_CM_F2F;

	if (r2y.mode == VS_CSC_CM_USR)
		get_int_list(coef_obj, (int *)r2y.coef);

	drmModeCreatePropertyBlob(dev->fd, &r2y, sizeof(struct drm_vs_r2y_config), &blob_id);
	return blob_id;
}

static uint64_t _map_drm_ccm(cJSON *obj, struct device *dev, enum map_ccm_type type)
{
	uint32_t blob_id = 0;
	struct drm_vs_ccm ccm;
	enum drm_vs_ccm_mode mode = VS_CCM_USER_DEF;
	uint32_t ccm_bit;

	cJSON *mode_obj = cJSON_GetObjectItem(obj, "mode");
	cJSON *coef_obj = cJSON_GetObjectItem(obj, "coef");
	cJSON *offset_obj = cJSON_GetObjectItem(obj, "offset");
	BLOB_PROP_GLOBAL_DISABLE(obj);

	if ((mode_obj = cJSON_GetObjectItem(obj, "mode"))) {
		if (!strcmp(mode_obj->valuestring, "709TO2020"))
			mode = VS_CCM_709_TO_2020;
		else if (!strcmp(mode_obj->valuestring, "2020TO709"))
			mode = VS_CCM_2020_TO_709;
		else if (!strcmp(mode_obj->valuestring, "2020TOP3"))
			mode = VS_CCM_2020_TO_DCIP3;
		else if (!strcmp(mode_obj->valuestring, "P3TO2020"))
			mode = VS_CCM_DCIP3_TO_2020;
		else if (!strcmp(mode_obj->valuestring, "P3TOSRGB"))
			mode = VS_CCM_DCIP3_TO_SRGB;
		else if (!strcmp(mode_obj->valuestring, "SRGBTOP3"))
			mode = VS_CCM_SRGB_TO_DCIP3;
		else if (!strcmp(mode_obj->valuestring, "USR"))
			mode = VS_CCM_USER_DEF;
	}

	if (mode == VS_CCM_USER_DEF) {
		get_uint32_list(coef_obj, (uint32_t *)ccm.coef);
		get_uint32_list(offset_obj, (uint32_t *)ccm.offset);
	} else {
		/* get ccm_bit from info */
		switch (type) {
		case FE_LNR_CCM:
			ccm_bit = dev->resources->crtcs->info.fe_lnr_ccm_bits;
			break;
		case BLD_NONLNR_CCM:
			ccm_bit = dev->resources->crtcs->info.bld_nonlnr_ccm_bits;
			break;
		case BLD_LNR_CCM:
			ccm_bit = dev->resources->crtcs->info.bld_lnr_ccm_bits;
			break;
		case BE_NONLNR_CCM:
			ccm_bit = dev->resources->crtcs->info.be_nonlnr_ccm_bits;
			break;
		case BE_LNR_CCM:
			ccm_bit = dev->resources->crtcs->info.be_lnr_ccm_bits;
			break;
		case BE_LNR_CCM_EX:
			ccm_bit = dev->resources->crtcs->info.be_lnr_ccm_ex_bits;
			break;
		case LAYER_CCM:
			ccm_bit = 12;
			break;
		default:
			printf("CCM type error, can not found ccm bit from chip info!\n");
			break;
		}
		vs_dc_cal_ccm_coef(ccm.coef, ccm.offset, mode, ccm_bit);
	}

	drmModeCreatePropertyBlob(dev->fd, &ccm, sizeof(struct drm_vs_ccm), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_fe_ccm(cJSON *obj, struct device *dev)
{
	return _map_drm_ccm(obj, dev, FE_LNR_CCM);
}

uint64_t MAPAPI map_bld_nonlnr_ccm(cJSON *obj, struct device *dev)
{
	return _map_drm_ccm(obj, dev, BLD_NONLNR_CCM);
}

uint64_t MAPAPI map_bld_lnr_ccm(cJSON *obj, struct device *dev)
{
	return _map_drm_ccm(obj, dev, BLD_LNR_CCM);
}

uint64_t MAPAPI map_be_nonlnr_ccm(cJSON *obj, struct device *dev)
{
	return _map_drm_ccm(obj, dev, BE_NONLNR_CCM);
}

uint64_t MAPAPI map_be_lnr_ccm(cJSON *obj, struct device *dev)
{
	return _map_drm_ccm(obj, dev, BE_LNR_CCM);
}

uint64_t MAPAPI map_be_lnr_ccm_ex(cJSON *obj, struct device *dev)
{
	return _map_drm_ccm(obj, dev, BE_LNR_CCM_EX);
}

uint64_t MAPAPI map_layer_ccm(cJSON *obj, struct device *dev)
{
	return _map_drm_ccm(obj, dev, LAYER_CCM);
}

uint64_t MAPAPI map_prior_3dlut(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0, i;
	struct drm_vs_color lut[VS_MAX_PRIOR_3DLUT_SIZE] = { 0 };
	uint32_t r_data[VS_MAX_PRIOR_3DLUT_SIZE] = { 0 };
	uint32_t g_data[VS_MAX_PRIOR_3DLUT_SIZE] = { 0 };
	uint32_t b_data[VS_MAX_PRIOR_3DLUT_SIZE] = { 0 };

	cJSON *r_chan_obj = cJSON_GetObjectItem(obj, "red_channel");
	cJSON *g_chan_obj = cJSON_GetObjectItem(obj, "green_channel");
	cJSON *b_chan_obj = cJSON_GetObjectItem(obj, "blue_channel");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	get_uint32_list(r_chan_obj, r_data);
	get_uint32_list(g_chan_obj, g_data);
	get_uint32_list(b_chan_obj, b_data);

	for (i = 0; i < VS_MAX_PRIOR_3DLUT_SIZE; i++) {
		lut[i].r = r_data[i];
		lut[i].g = g_data[i];
		lut[i].b = b_data[i];
	}

	drmModeCreatePropertyBlob(dev->fd, &lut,
				  sizeof(struct drm_vs_color) * VS_MAX_PRIOR_3DLUT_SIZE, &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_roi_3dlut(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0, i;
	struct drm_vs_roi_lut_config lut;
	uint32_t r_data[VS_MAX_ROI_3DLUT_SIZE] = { 0 };
	uint32_t g_data[VS_MAX_ROI_3DLUT_SIZE] = { 0 };
	uint32_t b_data[VS_MAX_ROI_3DLUT_SIZE] = { 0 };

	cJSON *r_chan_obj = cJSON_GetObjectItem(obj, "red_channel");
	cJSON *g_chan_obj = cJSON_GetObjectItem(obj, "green_channel");
	cJSON *b_chan_obj = cJSON_GetObjectItem(obj, "blue_channel");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	cJSON *x_obj = cJSON_GetObjectItem(obj, "x");
	cJSON *y_obj = cJSON_GetObjectItem(obj, "y");
	cJSON *width_obj = cJSON_GetObjectItem(obj, "width");
	cJSON *height_obj = cJSON_GetObjectItem(obj, "height");

	if (x_obj && y_obj && width_obj && height_obj) {
		lut.rect.x = x_obj->valueint;
		lut.rect.y = y_obj->valueint;
		lut.rect.w = width_obj->valueint;
		lut.rect.h = height_obj->valueint;
	}

	get_uint32_list(r_chan_obj, r_data);
	get_uint32_list(g_chan_obj, g_data);
	get_uint32_list(b_chan_obj, b_data);

	for (i = 0; i < VS_MAX_ROI_3DLUT_SIZE; i++) {
		lut.data[i].r = r_data[i];
		lut.data[i].g = g_data[i];
		lut.data[i].b = b_data[i];
	};

	drmModeCreatePropertyBlob(dev->fd, &lut, sizeof(struct drm_vs_roi_lut_config), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_xstep_lut(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_xstep_lut lut;
	cJSON *seg_point_obj = cJSON_GetObjectItem(obj, "seg_point");
	cJSON *seg_step_obj = cJSON_GetObjectItem(obj, "seg_step");
	cJSON *entry_obj = cJSON_GetObjectItem(obj, "entry");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	get_uint32_list(seg_point_obj, lut.seg_point);
	lut.seg_cnt = get_uint32_list(seg_step_obj, lut.seg_step);
	lut.entry_cnt = get_uint32_list(entry_obj, lut.data);

	drmModeCreatePropertyBlob(dev->fd, &lut, sizeof(struct drm_vs_xstep_lut), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_tone_map(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_tone_map tm;
	cJSON *seg_point_obj = cJSON_GetObjectItem(obj, "seg_point");
	cJSON *entry_obj = cJSON_GetObjectItem(obj, "entry");
	cJSON *pseudoy_obj = cJSON_GetObjectItem(obj, "pseudo_y");
	cJSON *pseudoy_mode_obj = cJSON_GetObjectItem(pseudoy_obj, "mode");

	if (!pseudoy_mode_obj) {
		printf("not found pseudoy mode config.\n");
		return 0;
	}
	const char *ymode = pseudoy_mode_obj->valuestring;

	BLOB_PROP_GLOBAL_DISABLE(obj);

	get_uint32_list(seg_point_obj, tm.lut.seg_point);
	tm.lut.entry_cnt = get_uint32_list(entry_obj, tm.lut.data);
	if (strcmp("LINEAR", ymode) == 0) {
		tm.pseudo_y.y_mode = VS_CALC_LNR_COMBINE;
	} else if (strcmp("MAX", ymode) == 0) {
		tm.pseudo_y.y_mode = VS_CALC_MAX;
	} else if (strcmp("MIXED", ymode) == 0) {
		tm.pseudo_y.y_mode = VS_CALC_MIXED;
	} else {
		fprintf(stderr, "Unknown ymode %s\n", ymode);
		return 0;
	}
	tm.pseudo_y.coef0 = cJSON_GetObjectItem(pseudoy_obj, "coef0")->valueint;
	tm.pseudo_y.coef1 = cJSON_GetObjectItem(pseudoy_obj, "coef1")->valueint;
	tm.pseudo_y.coef2 = cJSON_GetObjectItem(pseudoy_obj, "coef2")->valueint;
	tm.pseudo_y.weight = cJSON_GetObjectItem(pseudoy_obj, "weight")->valueint;

	drmModeCreatePropertyBlob(dev->fd, &tm, sizeof(struct drm_vs_tone_map), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_drm_hdr(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_hdr_algo_config algo = { 0 };
	struct drm_vs_hdr hdr = { 0 };
	char file_src[256];
	size_t len = 0;
	cJSON *hdr_enable_obj;
	cJSON *eotf_obj;
	cJSON *gamut_obj;
	cJSON *tonemap_obj;
	cJSON *oetf_obj;
	cJSON *obj_eotf_programmable;
	cJSON *obj_gamut_programmable;
	cJSON *obj_tonemap_programmable;
	cJSON *obj_oetf_programmable;
	BLOB_PROP_GLOBAL_DISABLE(obj);

	hdr_enable_obj = cJSON_GetObjectItem(obj, "hdr_enable");
	if (hdr_enable_obj)
		algo.hdr_enable = hdr_enable_obj->valueint;

	if (algo.hdr_enable) {
		cJSON *de_multiply_obj = cJSON_GetObjectItem(obj, "de_multiply");
		if (de_multiply_obj)
			algo.de_multiply = de_multiply_obj->valueint;

		if ((eotf_obj = cJSON_GetObjectItem(obj, "eotf"))) {
			cJSON *eotf_enable_obj = cJSON_GetObjectItem(eotf_obj, "eotf_enable");
			if (eotf_enable_obj)
				algo.eotf_enable = eotf_enable_obj->valueint;
		}
		if ((gamut_obj = cJSON_GetObjectItem(obj, "gamut"))) {
			cJSON *gamut_enable_obj = cJSON_GetObjectItem(gamut_obj, "gamut_enable");
			if (gamut_enable_obj)
				algo.gamut_enable = gamut_enable_obj->valueint;
		}
		if ((tonemap_obj = cJSON_GetObjectItem(obj, "tonemapping"))) {
			cJSON *tm_enable_obj =
				cJSON_GetObjectItem(tonemap_obj, "tonemapping_enable");
			if (tm_enable_obj)
				algo.tonemapping_enable = tm_enable_obj->valueint;
		}
		if ((oetf_obj = cJSON_GetObjectItem(obj, "oetf"))) {
			cJSON *oetf_enable_obj = cJSON_GetObjectItem(oetf_obj, "oetf_enable");
			if (oetf_enable_obj)
				algo.oetf_enable = oetf_enable_obj->valueint;
		}

		hdr.demultiply = algo.de_multiply;
		hdr.eotf_enable = algo.eotf_enable;
		hdr.gamut_map_enable = algo.gamut_enable;
		hdr.oetf_enable = algo.oetf_enable;
		hdr.tone_map_enable = algo.tonemapping_enable;

		/* EOTF */
		if (hdr.eotf_enable) {
			cJSON *eotf_mode_obj = cJSON_GetObjectItem(eotf_obj, "eotf_mode");
			if (!eotf_mode_obj) {
				fprintf(stderr, "not found config for HDR eotf mode.\n");
				goto exit;
			}
			const char *eotf_mode = eotf_mode_obj->valuestring;
			if (strcmp("Programmable", eotf_mode) == 0)
				algo.eotf.mode = VS_EO_ProgConf;
			else if (strcmp("PQ", eotf_mode) == 0)
				algo.eotf.mode = VS_EO_PQ;
			else if (strcmp("sRGB", eotf_mode) == 0)
				algo.eotf.mode = VS_EO_SRGB;
			else if (strcmp("HLG", eotf_mode) == 0)
				algo.eotf.mode = VS_EO_HLG;
			else if (strcmp("170M", eotf_mode) == 0)
				algo.eotf.mode = VS_EO_170M;
			else if (strcmp("DeGamma2.2", eotf_mode) == 0)
				algo.eotf.mode = VS_EO_DeGamma2_2;
			else {
				fprintf(stderr, "Unknown eotf_mode %s\n", eotf_mode);
				goto exit;
			}

			if (algo.eotf.mode == VS_EO_ProgConf) {
				if ((obj_eotf_programmable =
					     cJSON_GetObjectItem(eotf_obj, "eotf_Programmable"))) {
					memset(file_src, '\0', sizeof(char) * 256);
					char *json_string = obj_eotf_programmable->valuestring;
					sprintf(file_src, "%s%s", LUT_PATH, json_string);
					len = strlen(file_src);
					algo.eotf.programmable_csv = calloc(len + 1, sizeof(char));
					if (!algo.eotf.programmable_csv) {
						printf("calloc for algo.eotf.programmable_csv failed.\n");
						goto exit;
					}
					memset(algo.eotf.programmable_csv, '\0',
					       (len + 1) * sizeof(char));
					strncpy(algo.eotf.programmable_csv, file_src, len);
				}
				/* if  "eotf_Programmable" is NULL, recceives the JSON obj from Tool */
				else {
					printf("get eotf coef from Tool json\n");
					algo.eotf_enable = false;
					cJSON *entry_cnt_obj =
						cJSON_GetObjectItem(eotf_obj, "entry");
					if (entry_cnt_obj)
						hdr.eotf_config.entry_cnt = entry_cnt_obj->valueint;
					cJSON *seg_cnt_obj = cJSON_GetObjectItem(eotf_obj, "count");
					if (seg_cnt_obj)
						hdr.eotf_config.seg_cnt = seg_cnt_obj->valueint + 1;
					get_uint32_list(cJSON_GetObjectItem(eotf_obj, "points"),
							(uint32_t *)hdr.eotf_config.seg_point);
					get_uint32_list(cJSON_GetObjectItem(eotf_obj, "step"),
							(uint32_t *)hdr.eotf_config.seg_step);
					get_uint32_list(cJSON_GetObjectItem(eotf_obj, "data"),
							(uint32_t *)hdr.eotf_config.data);
				}
			}
		}

		/* GAMUT */
		if (hdr.gamut_map_enable) {
			cJSON *gamut_mode_obj = cJSON_GetObjectItem(gamut_obj, "gamut_mode");
			if (!gamut_mode_obj) {
				fprintf(stderr, "not found config for HDR gamut mode.\n");
				goto exit;
			}
			const char *gamut_mode = gamut_mode_obj->valuestring;

			if (!strcmp(gamut_mode, "709TO2020"))
				algo.gamut.mode = 2;
			else if (!strcmp(gamut_mode, "2020TO709"))
				algo.gamut.mode = 3;
			else if (!strcmp(gamut_mode, "2020TOP3"))
				algo.gamut.mode = 4;
			else if (!strcmp(gamut_mode, "P3TO2020"))
				algo.gamut.mode = 6;
			else if (!strcmp(gamut_mode, "P3TOSRGB"))
				algo.gamut.mode = 5;
			else if (!strcmp(gamut_mode, "SRGBTOP3"))
				algo.gamut.mode = 7;
			else if (!strcmp(gamut_mode, "Programmable"))
				algo.gamut.mode = 1;
			else {
				fprintf(stderr, "Unknown gamut_mode %s\n", gamut_mode);
				goto exit;
			}

			if (algo.gamut.mode == 1) {
				if ((obj_gamut_programmable = cJSON_GetObjectItem(
					     gamut_obj, "gamut_Programmable"))) {
					memset(file_src, '\0', sizeof(char) * 256);
					char *json_string = obj_gamut_programmable->valuestring;
					sprintf(file_src, "%s%s", LUT_PATH, json_string);
					len = strlen(file_src);
					algo.gamut.programmable_csv = calloc(len + 1, sizeof(char));
					if (!algo.gamut.programmable_csv) {
						printf("calloc for algo.gamut.programmable_csv failed.\n");
						goto exit;
					}
					memset(algo.gamut.programmable_csv, '\0',
					       (len + 1) * sizeof(char));
					strncpy(algo.gamut.programmable_csv, file_src, len);
				}
				/* if  "gamut_Programmable" is NULL, recceives the JSON obj from Tool */
				else {
					printf("get gamut coef from Tool json\n");
					algo.gamut_enable = false;
					get_uint32_list(cJSON_GetObjectItem(gamut_obj, "matrix"),
							(uint32_t *)hdr.gamut_map.coef);
				}
			}
		}

		/*TONE_MAPPING*/
		if (hdr.tone_map_enable) {
			cJSON *tm_mode_obj = cJSON_GetObjectItem(tonemap_obj, "tonemapping_mode");
			if (!tm_mode_obj) {
				fprintf(stderr, "not found config for HDR tone map mode.\n");
				goto exit;
			}
			const char *tonemapping_mode = tm_mode_obj->valuestring;
			if (!strcmp(tonemapping_mode, "Programmable"))
				algo.tonemap.mode = VS_TM_ProgConf;
			else if (!strcmp(tonemapping_mode, "HDR10"))
				algo.tonemap.mode = VS_TM_HDR10;
			else if (!strcmp(tonemapping_mode, "HLG"))
				algo.tonemap.mode = VS_TM_HLG;
			else if (!strcmp(tonemapping_mode, "Reinhard"))
				algo.tonemap.mode = VS_TM_Reinhard;
			else if (!strcmp(tonemapping_mode, "InvTM_Reinhard"))
				algo.tonemap.mode = VS_InvTM_Reinhard;
			else if (!strcmp(tonemapping_mode, "ProgCurve"))
				algo.tonemap.mode = VS_TM_ProgCurve;
			else {
				fprintf(stderr, "Unknown tonemapping_mode %s\n", tonemapping_mode);
				goto exit;
			}

			if (algo.tonemap.mode == VS_TM_ProgConf) {
				if ((obj_tonemap_programmable = cJSON_GetObjectItem(
					     tonemap_obj, "tonemapping_Programmable"))) {
					memset(file_src, '\0', sizeof(char) * 256);
					char *json_string = obj_tonemap_programmable->valuestring;
					sprintf(file_src, "%s%s", LUT_PATH, json_string);
					len = strlen(file_src);
					algo.tonemap.programmable_csv =
						calloc(len + 1, sizeof(char));
					if (!algo.tonemap.programmable_csv) {
						printf("calloc for algo.tonemap.programmable_csv failed.\n");
						goto exit;
					}
					memset(algo.tonemap.programmable_csv, '\0',
					       (len + 1) * sizeof(char));
					strncpy(algo.tonemap.programmable_csv, file_src, len);
				}
				/* if  "tonemapping_Programmable" is NULL, recceives the JSON obj from Tool */
				else {
					printf("get tonemapping coef from Tool json\n");
					algo.tonemapping_enable = false;
					cJSON *y_obj = cJSON_GetObjectItem(tonemap_obj, "y_coef");
					if (!y_obj) {
						fprintf(stderr, "not found y_coef config.\n");
						goto exit;
					}
					cJSON *coef0_obj = cJSON_GetArrayItem(y_obj, 0);
					cJSON *coef1_obj = cJSON_GetArrayItem(y_obj, 1);
					cJSON *coef2_obj = cJSON_GetArrayItem(y_obj, 2);
					if (!coef0_obj || !coef1_obj || !coef2_obj) {
						fprintf(stderr, "not found coef0/1/2 config.\n");
						goto exit;
					}
					hdr.tone_map.pseudo_y.coef0 = coef0_obj->valueint;
					hdr.tone_map.pseudo_y.coef1 = coef1_obj->valueint;
					hdr.tone_map.pseudo_y.coef2 = coef2_obj->valueint;
					cJSON *weight_obj =
						cJSON_GetObjectItem(tonemap_obj, "y_weight");
					cJSON *y_mode_obj =
						cJSON_GetObjectItem(tonemap_obj, "y_mode");
					if (weight_obj)
						hdr.tone_map.pseudo_y.weight = weight_obj->valueint;
					if (y_mode_obj)
						hdr.tone_map.pseudo_y.y_mode = y_mode_obj->valueint;
					get_uint32_list(cJSON_GetObjectItem(tonemap_obj, "points"),
							(uint32_t *)hdr.tone_map.lut.seg_point);
					hdr.tone_map.lut.entry_cnt = get_uint32_list(
						cJSON_GetObjectItem(tonemap_obj, "data"),
						(uint32_t *)hdr.tone_map.lut.data);
				}
			}

			cJSON *max_cll_obj = cJSON_GetObjectItem(tonemap_obj, "tonemapping_maxCLL");
			cJSON *max_dll_obj = cJSON_GetObjectItem(tonemap_obj, "tonemapping_maxDLL");
			cJSON *ks_obj = cJSON_GetObjectItem(tonemap_obj, "tonemapping_Ks");
			cJSON *kf_obj = cJSON_GetObjectItem(tonemap_obj, "tonemapping_Kf");
			cJSON *len_p_obj = cJSON_GetObjectItem(tonemap_obj, "tonemapping_lenP");
			if (max_cll_obj)
				algo.tonemap.max_cll = max_cll_obj->valueint;
			if (max_dll_obj)
				algo.tonemap.max_dll = max_dll_obj->valueint;
			if (ks_obj)
				algo.tonemap.ks = ks_obj->valuedouble;
			if (kf_obj)
				algo.tonemap.kf = kf_obj->valuedouble;
			if (len_p_obj)
				algo.tonemap.len_p = len_p_obj->valueint;

			for (uint8_t j = 0; j < algo.tonemap.len_p; j++) {
				double temp = (cJSON_GetArrayItem(
						       cJSON_GetObjectItem(tonemap_obj,
									   "tonemapping_bezierP"),
						       j))
						      ->valuedouble;
				algo.tonemap.bezier_p[j] = temp;
			}
		}

		/* OETF */
		if (hdr.oetf_enable) {
			cJSON *oetf_mode_obj = cJSON_GetObjectItem(oetf_obj, "oetf_mode");
			if (!oetf_mode_obj) {
				fprintf(stderr, "not found config for HDR oetf mode.\n");
				goto exit;
			}
			const char *oetf_mode = oetf_mode_obj->valuestring;
			if (strcmp("Programmable", oetf_mode) == 0)
				algo.oetf.mode = VS_OE_ProgConf;
			else if (strcmp("PQ", oetf_mode) == 0)
				algo.oetf.mode = VS_OE_PQ;
			else if (strcmp("sRGB", oetf_mode) == 0)
				algo.oetf.mode = VS_OE_SRGB;
			else if (strcmp("HLG", oetf_mode) == 0)
				algo.oetf.mode = VS_OE_HLG;
			else if (strcmp("170M", oetf_mode) == 0)
				algo.oetf.mode = VS_OE_170M;
			else if (strcmp("ReGamma2.2", oetf_mode) == 0)
				algo.oetf.mode = VS_OE_ReGamma2_2;
			else {
				fprintf(stderr, "Unknown oetf_mode %s\n", oetf_mode);
				goto exit;
			}

			if (algo.oetf.mode == VS_OE_ProgConf) {
				if ((obj_oetf_programmable =
					     cJSON_GetObjectItem(oetf_obj, "oetf_Programmable"))) {
					memset(file_src, '\0', sizeof(char) * 256);
					char *json_string = obj_oetf_programmable->valuestring;
					sprintf(file_src, "%s%s", LUT_PATH, json_string);
					len = strlen(file_src);
					algo.oetf.programmable_csv = calloc(len + 1, sizeof(char));
					if (!algo.oetf.programmable_csv) {
						printf("calloc for algo.oetf.programmable_csv failed.\n");
						goto exit;
					}
					memset(algo.oetf.programmable_csv, '\0',
					       (len + 1) * sizeof(char));
					strncpy(algo.oetf.programmable_csv, file_src, len);
				}
				/* if  "oetf_Programmable" is NULL, recceives the JSON obj from Tool */
				else {
					printf("get oetf coef from Tool json\n");
					algo.oetf_enable = false;
					cJSON *entry_cnt_obj =
						cJSON_GetObjectItem(oetf_obj, "entry");
					cJSON *seg_cnt_obj = cJSON_GetObjectItem(oetf_obj, "count");
					if (entry_cnt_obj)
						hdr.oetf_config.entry_cnt = entry_cnt_obj->valueint;
					if (seg_cnt_obj)
						hdr.oetf_config.seg_cnt = seg_cnt_obj->valueint + 1;
					get_uint32_list(cJSON_GetObjectItem(oetf_obj, "points"),
							(uint32_t *)hdr.oetf_config.seg_point);
					get_uint32_list(cJSON_GetObjectItem(oetf_obj, "step"),
							(uint32_t *)hdr.oetf_config.seg_step);
					get_uint32_list(cJSON_GetObjectItem(oetf_obj, "data"),
							(uint32_t *)hdr.oetf_config.data);
				}
			}
		}

		if (dev->dpu_algorithm->funcs->dpu_algo_hdr) {
			dev->dpu_algorithm->funcs->dpu_algo_hdr(&algo, &hdr);
		} else {
			printf("Not found the HDR algo function!\n");
			goto exit;
		}

		dev->dpu_algorithm->funcs->dpu_algo_hdr(&algo, &hdr);

		drmModeCreatePropertyBlob(dev->fd, &hdr, sizeof(struct drm_vs_hdr), &blob_id);
	}

exit:
	if (algo.eotf.programmable_csv)
		free(algo.eotf.programmable_csv);
	if (algo.gamut.programmable_csv)
		free(algo.gamut.programmable_csv);
	if (algo.tonemap.programmable_csv)
		free(algo.tonemap.programmable_csv);
	if (algo.oetf.programmable_csv)
		free(algo.oetf.programmable_csv);

	return blob_id;
}

uint64_t MAPAPI map_regamma_lut(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_gamma_lut lut;
	cJSON *in_bit_obj = cJSON_GetObjectItem(obj, "in_bit");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	/*DC9x00: only 9x00 has in_bit*/
	if (in_bit_obj) {
		printf("regamma configurate DC9x00\n");
		int in_bit, out_bit, i;
		drm_vs_data_trans_mode mode = DRM_VS_OETF_SRGB;
		float exp = 2.2;
		uint32_t r_data[VS_MAX_GAMMA_ENTRY_CNT] = { 0 };
		uint32_t g_data[VS_MAX_GAMMA_ENTRY_CNT] = { 0 };
		uint32_t b_data[VS_MAX_GAMMA_ENTRY_CNT] = { 0 };
		cJSON *seg_point_obj = cJSON_GetObjectItem(obj, "seg_point");
		cJSON *seg_step_obj = cJSON_GetObjectItem(obj, "seg_step");
		cJSON *mode_obj = cJSON_GetObjectItem(obj, "mode");
		cJSON *gamma_obj = cJSON_GetObjectItem(obj, "gamma");
		in_bit = cJSON_GetObjectItem(obj, "in_bit")->valueint;
		out_bit = cJSON_GetObjectItem(obj, "out_bit")->valueint;
		lut.seg_cnt = get_uint32_list(seg_step_obj, lut.seg_step);
		get_uint32_list(seg_point_obj, lut.seg_point);

		if (mode_obj) {
			if (!strcmp(mode_obj->valuestring, "PQ"))
				mode = DRM_VS_OETF_PQ;
			else if (!strcmp(mode_obj->valuestring, "REGAMMA"))
				mode = DRM_VS_OETF_REGAMMA;
			else if (!strcmp(mode_obj->valuestring, "SRGB"))
				mode = DRM_VS_OETF_SRGB;
		}
		if (gamma_obj)
			exp = gamma_obj->valuedouble;

		lut.entry_cnt = drm_vs_init_data_trans_entry(mode, exp, in_bit, out_bit,
							     lut.seg_cnt, lut.seg_point,
							     lut.seg_step, r_data);

		drm_vs_init_data_trans_entry(mode, exp, in_bit, out_bit, lut.seg_cnt, lut.seg_point,
					     lut.seg_step, g_data);

		drm_vs_init_data_trans_entry(mode, exp, in_bit, out_bit, lut.seg_cnt, lut.seg_point,
					     lut.seg_step, b_data);

		for (i = 0; i < VS_MAX_GAMMA_ENTRY_CNT; i++) {
			lut.data[i].r = r_data[i];
			lut.data[i].g = g_data[i];
			lut.data[i].b = b_data[i];
		}

		drmModeCreatePropertyBlob(dev->fd, &lut, sizeof(struct drm_vs_gamma_lut), &blob_id);
	}
	/*DC9000SR prase*/
	else {
		printf("regamma configurate DC9000SR\n");
		char file_src[256];
		size_t len = 0;
		uint32_t regamma_mode = 0;
		char *lut_file_r = NULL;
		char *lut_file_g = NULL;
		char *lut_file_b = NULL;
		bool enable = false;
		bool algo_enable = false;
		const char *mode = NULL;
		cJSON *config_lut_r_obj;
		cJSON *config_lut_g_obj;
		cJSON *config_lut_b_obj;
		cJSON *enable_obj = cJSON_GetObjectItem(obj, "enable");
		cJSON *mode_obj = cJSON_GetObjectItem(obj, "mode");

		if (enable_obj) {
			enable = enable_obj->valueint;
			algo_enable = enable;
		}
		if (mode_obj)
			mode = mode_obj->valuestring;

		if (enable) {
			/* 0:Disable 1:ProgConf 2:Eotf_PQ 3:Eotf_HLG 4:Eotf_170M 5:Eotf_sRGB 6:Eotf_ReGamma2_2 */
			if (strcmp("Programmable", mode) == 0)
				regamma_mode = 1;
			else if (strcmp("PQ", mode) == 0)
				regamma_mode = 2;
			else if (strcmp("HLG", mode) == 0)
				regamma_mode = 3;
			else if (strcmp("170M", mode) == 0)
				regamma_mode = 4;
			else if (strcmp("SRGB", mode) == 0)
				regamma_mode = 5;
			else if (strcmp("ReGamma2.2", mode) == 0)
				regamma_mode = 6;
			else {
				fprintf(stderr, "Unknown regamma_mode %s\n", mode);
				return 0;
			}
			/*user define*/
			if (regamma_mode == 1) {
				if ((config_lut_r_obj = cJSON_GetObjectItem(obj, "config_lut_r"))) {
					/*lut r file*/
					memset(file_src, '\0', sizeof(char) * 256);
					char *lut_r_string = config_lut_r_obj->valuestring;
					sprintf(file_src, "%s%s", LUT_PATH, lut_r_string);
					len = strlen(file_src);
					lut_file_r = calloc(len, sizeof(char));
					strncpy(lut_file_r, file_src, len);
					/*lut g file*/
					config_lut_g_obj = cJSON_GetObjectItem(obj, "config_lut_g");
					memset(file_src, '\0', sizeof(char) * 256);
					char *lut_g_string = config_lut_g_obj->valuestring;
					sprintf(file_src, "%s%s", LUT_PATH, lut_g_string);
					len = strlen(file_src);
					lut_file_g = calloc(len, sizeof(char));
					strncpy(lut_file_g, file_src, len);
					/*lut b file*/
					config_lut_b_obj = cJSON_GetObjectItem(obj, "config_lut_b");
					memset(file_src, '\0', sizeof(char) * 256);
					char *lut_b_string = config_lut_b_obj->valuestring;
					sprintf(file_src, "%s%s", LUT_PATH, lut_b_string);
					len = strlen(file_src);
					lut_file_b = calloc(len, sizeof(char));
					strncpy(lut_file_b, file_src, len);
				}
				/* if  "config_lut_obj" is NULL, recceives the JSON obj from Tool */
				else {
					printf("get regamma coef from Tool json\n");
					algo_enable = 0;
					cJSON *seg_point_r_obj =
						cJSON_GetObjectItem(obj, "seg_point_r");
					cJSON *seg_step_r_obj =
						cJSON_GetObjectItem(obj, "seg_step_r");
					cJSON *entry_r_obj = cJSON_GetObjectItem(obj, "entry_r");
					cJSON *seg_point_g_obj =
						cJSON_GetObjectItem(obj, "seg_point_g");
					cJSON *seg_step_g_obj =
						cJSON_GetObjectItem(obj, "seg_step_g");
					cJSON *entry_g_obj = cJSON_GetObjectItem(obj, "entry_g");
					cJSON *seg_point_b_obj =
						cJSON_GetObjectItem(obj, "seg_point_b");
					cJSON *seg_step_b_obj =
						cJSON_GetObjectItem(obj, "seg_step_b");
					cJSON *entry_b_obj = cJSON_GetObjectItem(obj, "entry_b");

					uint32_t r_point[VS_MAX_LUT_SEG_CNT - 1] = { 0 };
					uint32_t g_point[VS_MAX_LUT_SEG_CNT - 1] = { 0 };
					uint32_t b_point[VS_MAX_LUT_SEG_CNT - 1] = { 0 };
					uint32_t r_step[VS_MAX_LUT_SEG_CNT] = { 0 };
					uint32_t g_step[VS_MAX_LUT_SEG_CNT] = { 0 };
					uint32_t b_step[VS_MAX_LUT_SEG_CNT] = { 0 };
					uint32_t r_data[VS_MAX_GAMMA_ENTRY_CNT] = { 0 };
					uint32_t g_data[VS_MAX_GAMMA_ENTRY_CNT] = { 0 };
					uint32_t b_data[VS_MAX_GAMMA_ENTRY_CNT] = { 0 };

					get_uint32_list(seg_point_r_obj, r_point);
					uint32_t seg_cnt_r =
						get_uint32_list(seg_step_r_obj, r_step);
					uint32_t entry_cnt_r = get_uint32_list(entry_r_obj, r_data);
					lut.seg_cnt_sr.r = seg_cnt_r;
					lut.entry_cnt_sr.r = entry_cnt_r;
					for (int i = 0; i < seg_cnt_r - 1; i++)
						lut.seg_point_sr[i].r = r_point[i];
					for (int i = 0; i < seg_cnt_r; i++)
						lut.seg_step_sr[i].r = r_step[i];
					for (int i = 0; i < entry_cnt_r; i++)
						lut.data[i].r = r_data[i];

					get_uint32_list(seg_point_g_obj, g_point);
					uint32_t seg_cnt_g =
						get_uint32_list(seg_step_g_obj, g_step);
					uint32_t entry_cnt_g = get_uint32_list(entry_g_obj, g_data);
					lut.seg_cnt_sr.g = seg_cnt_g;
					lut.entry_cnt_sr.g = entry_cnt_g;
					for (int i = 0; i < seg_cnt_g - 1; i++)
						lut.seg_point_sr[i].g = g_point[i];
					for (int i = 0; i < seg_cnt_g; i++)
						lut.seg_step_sr[i].g = g_step[i];
					for (int i = 0; i < entry_cnt_g; i++)
						lut.data[i].g = g_data[i];

					get_uint32_list(seg_point_b_obj, b_point);
					uint32_t seg_cnt_b =
						get_uint32_list(seg_step_b_obj, b_step);
					uint32_t entry_cnt_b = get_uint32_list(entry_b_obj, b_data);
					lut.seg_cnt_sr.b = seg_cnt_b;
					lut.entry_cnt_sr.b = entry_cnt_b;
					for (int i = 0; i < seg_cnt_b - 1; i++)
						lut.seg_point_sr[i].b = b_point[i];
					for (int i = 0; i < seg_cnt_b; i++)
						lut.seg_step_sr[i].b = b_step[i];
					for (int i = 0; i < entry_cnt_b; i++)
						lut.data[i].b = b_data[i];
				}
			}

			if (dev->dpu_algorithm->funcs->dpu_algo_regamma) {
				dev->dpu_algorithm->funcs->dpu_algo_regamma(regamma_mode,
									    lut_file_r, lut_file_g,
									    lut_file_b, &lut,
									    algo_enable);
			} else {
				printf("Not found the reGamma algo function!\n");
				return 0;
			}

			drmModeCreatePropertyBlob(dev->fd, &lut, sizeof(struct drm_vs_gamma_lut),
						  &blob_id);
		}
	}

	return blob_id;
}

uint64_t MAPAPI map_gamma_lut(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	double gamma_value;
	int new_gamma, gamma_entry_cnt;
	int gamma_bit_out = 12;
	cJSON *curve_type_obj = cJSON_GetObjectItem(obj, "curve_type");
	cJSON *gamma_value_obj = cJSON_GetObjectItem(obj, "gamma_value");
	cJSON *new_gamma_obj = cJSON_GetObjectItem(obj, "new_gamma");
	cJSON *gamma_bit_out_obj = cJSON_GetObjectItem(obj, "gamma_bit_out");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	gamma_entry_cnt = (int)get_crtc_property_init_value(dev, "GAMMA_LUT_SIZE");

	struct drm_color_lut *lut = calloc(gamma_entry_cnt, sizeof(struct drm_color_lut));

	if (!lut) {
		printf("calloc for gamma lut failed.\n");
		return 0;
	}

	if (!curve_type_obj || !gamma_value_obj || !new_gamma_obj || !gamma_bit_out_obj) {
		printf("config for GAMMA LUT is incomplete.\n");
		if (lut)
			free(lut);
		return 0;
	}

	gamma_value = gamma_value_obj->valueint;
	new_gamma = new_gamma_obj->valueint;
	gamma_bit_out = gamma_bit_out_obj->valueint;

	drm_vs_init_gamma_lut(new_gamma, curve_type_obj->valuestring, gamma_value, gamma_bit_out,
			      gamma_entry_cnt, lut);

	drmModeCreatePropertyBlob(dev->fd, lut, sizeof(struct drm_color_lut) * gamma_entry_cnt,
				  &blob_id);

	return blob_id;
}

/* for dc8x00 degamma feature */
uint64_t MAPAPI map_degamma_config(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_degamma_config degamma_config = { 0 };
	cJSON *degamma_mode_obj = cJSON_GetObjectItem(obj, "degammaMode");
	cJSON *r_chan_obj = cJSON_GetObjectItem(obj, "red_chanel");
	cJSON *g_chan_obj = cJSON_GetObjectItem(obj, "green_chanel");
	cJSON *b_chan_obj = cJSON_GetObjectItem(obj, "blue_chanel");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (!degamma_mode_obj) {
		printf("not found degammaMode config.\n");
		return 0;
	}
	if (!strcmp(degamma_mode_obj->valuestring, "BT709"))
		degamma_config.mode = VS_DEGAMMA_BT709;
	else if (!strcmp(degamma_mode_obj->valuestring, "BT2020"))
		degamma_config.mode = VS_DEGAMMA_BT2020;
	else if (!strcmp(degamma_mode_obj->valuestring, "USR"))
		degamma_config.mode = VS_DEGAMMA_USR;

	if (degamma_config.mode == VS_DEGAMMA_USR) {
		if (!r_chan_obj || !g_chan_obj || !b_chan_obj) {
			printf("config for degamma USR mode is incomplete.\n");
			return 0;
		}
		get_uint16_list(r_chan_obj, (uint16_t *)degamma_config.r);
		get_uint16_list(g_chan_obj, (uint16_t *)degamma_config.g);
		get_uint16_list(b_chan_obj, (uint16_t *)degamma_config.b);
	}

	drmModeCreatePropertyBlob(dev->fd, &degamma_config, sizeof(struct drm_vs_degamma_config),
				  &blob_id);
	return blob_id;
}

/* for dc9000sr degamma feature */
uint64_t MAPAPI map_drm_degamma(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	char lut_file[256] = { 0 };
	uint32_t degamma_mode = 0;
	struct drm_vs_xstep_lut lut;

	BLOB_PROP_GLOBAL_DISABLE(obj);

	cJSON *enable_obj = cJSON_GetObjectItem(obj, "enable");
	cJSON *mode_obj = cJSON_GetObjectItem(obj, "mode");
	bool enable = false;
	bool algo_enable = false;
	const char *mode = NULL;
	cJSON *config_lut_obj;

	if (enable_obj) {
		enable = enable_obj->valueint;
		algo_enable = enable;
	}
	if (mode_obj)
		mode = mode_obj->valuestring;

	if (enable) {
		/* 0:Disable 1:ProgConf 2:DeG_PQ 3:DeG_HLG 4:DeG_170M 5:DeG_sRGB 6:DeG_DeGamma2_2 */
		if (strcmp("Programmable", mode) == 0)
			degamma_mode = 1;
		else if (strcmp("PQ", mode) == 0)
			degamma_mode = 2;
		else if (strcmp("HLG", mode) == 0)
			degamma_mode = 3;
		else if (strcmp("170M", mode) == 0)
			degamma_mode = 4;
		else if (strcmp("SRGB", mode) == 0)
			degamma_mode = 5;
		else if (strcmp("DeGamma2.2", mode) == 0)
			degamma_mode = 6;
		else {
			fprintf(stderr, "Unknown degamma_mode %s\n", mode);
			return 0;
		}
		/*user define*/
		if (degamma_mode == 1) {
			if ((config_lut_obj = cJSON_GetObjectItem(obj, "config_lut"))) {
				memset(lut_file, '\0', sizeof(char) * 256);
				char *json_string = config_lut_obj->valuestring;
				printf("config_lut: %s\n", json_string);
				sprintf(lut_file, "%s%s", LUT_PATH, json_string);
			}
			/* if  "config_lut_obj" is NULL, recceives the JSON obj from Tool */
			else {
				printf("get degamma coef from Tool json\n");
				algo_enable = 0;

				cJSON *seg_point_obj = cJSON_GetObjectItem(obj, "seg_point");
				cJSON *seg_step_obj = cJSON_GetObjectItem(obj, "seg_step");
				cJSON *entry_obj = cJSON_GetObjectItem(obj, "entry");

				get_uint32_list(seg_point_obj, lut.seg_point);
				lut.seg_cnt = get_uint32_list(seg_step_obj, lut.seg_step);
				lut.entry_cnt = get_uint32_list(entry_obj, lut.data);
			}
		}

		if (dev->dpu_algorithm->funcs->dpu_algo_degamma) {
			dev->dpu_algorithm->funcs->dpu_algo_degamma(degamma_mode, lut_file, &lut,
								    algo_enable);
		} else {
			printf("Not found the deGamma algo function!\n");
			return 0;
		}

		drmModeCreatePropertyBlob(dev->fd, &lut, sizeof(struct drm_vs_xstep_lut), &blob_id);
	}

	return blob_id;
}

uint64_t MAPAPI map_data_extend(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_data_extend data_extend = { 0 };

	cJSON *data_extend_mode_obj = cJSON_GetObjectItem(obj, "dataExtendMode");
	cJSON *alpha_extend_value_obj = cJSON_GetObjectItem(obj, "alphaDataExtendValue");
	cJSON *alpha_extend_enable_obj = cJSON_GetObjectItem(obj, "alphaDataExtendEnable");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (!data_extend_mode_obj) {
		printf("not found dataExtendMode config.\n");
		return 0;
	}
	if (!strcmp(data_extend_mode_obj->valuestring, "STD"))
		data_extend.data_extend_mode = VS_DATA_EXT_STD;
	else if (!strcmp(data_extend_mode_obj->valuestring, "MSB"))
		data_extend.data_extend_mode = VS_DATA_EXT_MSB;
	else if (!strcmp(data_extend_mode_obj->valuestring, "RANDOM"))
		data_extend.data_extend_mode = VS_DATA_EXT_RANDOM;

	if (alpha_extend_value_obj)
		data_extend.alpha_data_extend.alpha_extend_value =
			alpha_extend_value_obj->valuedouble;

	if (alpha_extend_enable_obj)
		data_extend.alpha_data_extend.enable = alpha_extend_enable_obj->valueint;

	drmModeCreatePropertyBlob(dev->fd, &data_extend, sizeof(struct drm_vs_data_extend),
				  &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_data_trunc(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_data_trunc data_trunc = { 0 };
	int temp = 0;

	cJSON *gamma_data_trunc_mode_obj = cJSON_GetObjectItem(obj, "gammaDataTruncMode");
	cJSON *panel_data_trunc_mode_obj = cJSON_GetObjectItem(obj, "panelDataTruncMode");
	cJSON *blend_data_trunc_mode_obj = cJSON_GetObjectItem(obj, "blendDataTruncMode");
	cJSON *wb_data_trunc_mode_obj = cJSON_GetObjectItem(obj, "wbDataTruncMode");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	const struct enum_str_match_list data_trunc_mode_list[] = {
		{ VS_DATA_TRUNCATE, "TRUNCATE" },
		{ VS_DATA_ROUNDING, "ROUNDING" },
	};

	if (gamma_data_trunc_mode_obj) {
		temp = string_to_enum(data_trunc_mode_list, ARRAY_SIZE(data_trunc_mode_list),
				      gamma_data_trunc_mode_obj->valuestring);
		ARRAY_ENUM_MATCH_FAIL(temp);
		data_trunc.gamma_data_trunc = temp;
	}

	if (panel_data_trunc_mode_obj) {
		temp = string_to_enum(data_trunc_mode_list, ARRAY_SIZE(data_trunc_mode_list),
				      panel_data_trunc_mode_obj->valuestring);
		ARRAY_ENUM_MATCH_FAIL(temp);
		data_trunc.panel_data_trunc = temp;
	}

	if (blend_data_trunc_mode_obj) {
		temp = string_to_enum(data_trunc_mode_list, ARRAY_SIZE(data_trunc_mode_list),
				      blend_data_trunc_mode_obj->valuestring);
		ARRAY_ENUM_MATCH_FAIL(temp);
		data_trunc.blend_data_trunc = temp;
	}

	if (wb_data_trunc_mode_obj) {
		temp = string_to_enum(data_trunc_mode_list, ARRAY_SIZE(data_trunc_mode_list),
				      wb_data_trunc_mode_obj->valuestring);
		ARRAY_ENUM_MATCH_FAIL(temp);
		data_trunc.wb_data_trunc = temp;
	}

	drmModeCreatePropertyBlob(dev->fd, &data_trunc, sizeof(struct drm_vs_data_trunc), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_panel_crop(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_panel_crop panel_crop = { 0 };

	cJSON *x_obj = cJSON_GetObjectItem(obj, "cropX");
	cJSON *y_obj = cJSON_GetObjectItem(obj, "cropY");
	cJSON *w_obj = cJSON_GetObjectItem(obj, "cropWidth");
	cJSON *h_obj = cJSON_GetObjectItem(obj, "cropHeight");
	cJSON *src_w_obj = cJSON_GetObjectItem(obj, "srcPanelWidth");
	cJSON *src_h_obj = cJSON_GetObjectItem(obj, "srcPanelHeight");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (x_obj)
		panel_crop.crop_rect.x = x_obj->valueint;
	if (y_obj)
		panel_crop.crop_rect.y = y_obj->valueint;
	if (w_obj)
		panel_crop.crop_rect.w = w_obj->valueint;
	if (h_obj)
		panel_crop.crop_rect.h = h_obj->valueint;
	if (src_w_obj)
		panel_crop.panel_src_width = src_w_obj->valueint;
	if (src_h_obj)
		panel_crop.panel_src_height = src_h_obj->valueint;

	drmModeCreatePropertyBlob(dev->fd, &panel_crop, sizeof(struct drm_vs_panel_crop), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_splice_mode(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0, i = 0;
	struct drm_vs_splice_mode splice_mode = { 0 };
	char *str = NULL, *substr[4] = { "crtc0", "crtc1", "crtc2", "crtc3" };

	cJSON *splice0_enable_obj = cJSON_GetObjectItem(obj, "splice0Enable");
	cJSON *splice0_crtc_mask_obj = cJSON_GetObjectItem(obj, "splice0CrtcMask");
	cJSON *splice0_output_intf_obj = cJSON_GetObjectItem(obj, "splice0OutputIntf");

	cJSON *splice1_enable_obj = cJSON_GetObjectItem(obj, "splice1Enable");
	cJSON *splice1_crtc_mask_obj = cJSON_GetObjectItem(obj, "splice1CrtcMask");
	cJSON *splice1_output_intf_obj = cJSON_GetObjectItem(obj, "splice1OutputIntf");

	cJSON *src_panel_width0_obj = cJSON_GetObjectItem(obj, "srcPanelWidth0");
	cJSON *src_panel_width1_obj = cJSON_GetObjectItem(obj, "srcPanelWidth1");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (splice0_enable_obj) {
		splice_mode.splice0_enable = !!splice0_enable_obj->valueint;
		if (splice0_output_intf_obj)
			splice_mode.splice0_output_intf = splice0_output_intf_obj->valueint;
		if (splice0_crtc_mask_obj)
			str = splice0_crtc_mask_obj->valuestring;

		if (str != NULL) {
			for (i = 0; i < 4; i++) {
				if (strstr(str, substr[i]))
					splice_mode.splice0_crtc_mask |= BIT(i);
			}
		}
	}

	if (splice1_enable_obj) {
		splice_mode.splice1_enable = !!splice1_enable_obj->valueint;
		if (splice1_output_intf_obj)
			splice_mode.splice1_output_intf = splice1_output_intf_obj->valueint;
		if (splice1_crtc_mask_obj)
			str = splice1_crtc_mask_obj->valuestring;

		if (str != NULL) {
			for (i = 0; i < 4; i++) {
				if (strstr(str, substr[i]))
					splice_mode.splice1_crtc_mask |= BIT(i);
			}
		}
	}

	if (src_panel_width0_obj)
		splice_mode.src_panel_width0 = src_panel_width0_obj->valueint;
	if (src_panel_width1_obj)
		splice_mode.src_panel_width1 = src_panel_width1_obj->valueint;

	drmModeCreatePropertyBlob(dev->fd, &splice_mode, sizeof(struct drm_vs_splice_mode),
				  &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_lbox(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_lbox lbox = { 0 };
	cJSON *lbox_color_red_obj = cJSON_GetObjectItem(obj, "color_red");
	cJSON *lbox_color_green_obj = cJSON_GetObjectItem(obj, "color_green");
	cJSON *lbox_color_blue_obj = cJSON_GetObjectItem(obj, "color_blue");

	cJSON *lbox_roi_origin_x_obj = cJSON_GetObjectItem(obj, "roi_origin_x");
	cJSON *lbox_roi_origin_y_obj = cJSON_GetObjectItem(obj, "roi_origin_y");

	cJSON *lbox_roi_width_obj = cJSON_GetObjectItem(obj, "roi_size_width");
	cJSON *lbox_roi_height_obj = cJSON_GetObjectItem(obj, "roi_size_height");

	if (!lbox_roi_width_obj || !lbox_roi_height_obj) {
		printf("config of lbox is incomplete.\n");
		return 0;
	}

	BLOB_PROP_GLOBAL_DISABLE(obj);

	lbox.bg_color.r = lbox_color_red_obj ? lbox_color_red_obj->valueint : 0;
	lbox.bg_color.g = lbox_color_green_obj ? lbox_color_green_obj->valueint : 0;
	lbox.bg_color.b = lbox_color_blue_obj ? lbox_color_blue_obj->valueint : 0;

	lbox.box_rect.x = lbox_roi_origin_x_obj ? lbox_roi_origin_x_obj->valueint : 0;
	lbox.box_rect.y = lbox_roi_origin_y_obj ? lbox_roi_origin_y_obj->valueint : 0;

	lbox.box_rect.w = lbox_roi_width_obj ? lbox_roi_width_obj->valueint : 0;
	lbox.box_rect.h = lbox_roi_height_obj ? lbox_roi_height_obj->valueint : 0;

	drmModeCreatePropertyBlob(dev->fd, &lbox, sizeof(struct drm_vs_lbox), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_dp_sync(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0, i = 0;
	struct drm_vs_dp_sync dp_sync = { 0 };
	cJSON *dp_sync_mask_obj = cJSON_GetObjectItem(obj, "dpSyncCrtcMask");
	cJSON *duplicate_obj = cJSON_GetObjectItem(obj, "duplicate");
	char *str = NULL, *substr[4] = { "crtc0", "crtc1", "crtc2", "crtc3" };

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (!dp_sync_mask_obj) {
		printf("not found dpSyncCrtcMask config.\n");
		return 0;
	}
	str = dp_sync_mask_obj->valuestring;

	if (str != NULL) {
		for (i = 0; i < 4; i++) {
			if (strstr(str, substr[i]))
				dp_sync.dp_sync_crtc_mask |= BIT(i);
		}
	}

	if (duplicate_obj)
		dp_sync.duplicate = !!(duplicate_obj->valueint);
	else
		dp_sync.duplicate = (bool)0;

	drmModeCreatePropertyBlob(dev->fd, &dp_sync, sizeof(struct drm_vs_dp_sync), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_free_sync(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	cJSON *free_sync_type_obj = cJSON_GetObjectItem(obj, "freeSyncType");
	cJSON *max_delay_obj = NULL;
	cJSON *finish_obj = NULL;
	struct drm_vs_free_sync free_sync = { 0 };

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (!free_sync_type_obj) {
		printf("not found freeSyncType config.\n");
		return 0;
	}
	if (!strcmp(free_sync_type_obj->valuestring, "config"))
		free_sync.type = VS_FREE_SYNC_CONFIG;
	else if (!strcmp(free_sync_type_obj->valuestring, "finish"))
		free_sync.type = VS_FREE_SYNC_FINISH;
	else if (!strcmp(free_sync_type_obj->valuestring, "config_finish"))
		free_sync.type = VS_FREE_SYNC_CONFIG_FINISH;

	if (free_sync.type == VS_FREE_SYNC_CONFIG || free_sync.type == VS_FREE_SYNC_CONFIG_FINISH) {
		max_delay_obj = cJSON_GetObjectItem(obj, "freeSyncMaxDelay");
		if (max_delay_obj)
			free_sync.mode.free_sync_max_delay = max_delay_obj->valueint;
	}

	if (free_sync.type == VS_FREE_SYNC_FINISH || free_sync.type == VS_FREE_SYNC_CONFIG_FINISH) {
		finish_obj = cJSON_GetObjectItem(obj, "freeSyncFinish");
		if (finish_obj)
			free_sync.mode.free_sync_finish = !!(finish_obj->valueint);
	}

	drmModeCreatePropertyBlob(dev->fd, &free_sync, sizeof(struct drm_vs_free_sync), &blob_id);
	return blob_id;
}

/* for dc8x00 vrr feature */
uint64_t MAPAPI map_vrr_refresh(cJSON *obj, struct device *dev)
{
	uint32_t vrr_refresh = 60, blob_id = 0;
	cJSON *obj_vrr_refresh = cJSON_GetObjectItem(obj, "vrrRefresh");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (obj_vrr_refresh)
		vrr_refresh = obj_vrr_refresh->valueint;

	drmModeCreatePropertyBlob(dev->fd, &vrr_refresh, sizeof(uint32_t), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_color(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_color color = { 0 };

	BLOB_PROP_GLOBAL_DISABLE(obj);

	cJSON *a_obj = cJSON_GetObjectItem(obj, "a");
	cJSON *r_obj = cJSON_GetObjectItem(obj, "r");
	cJSON *g_obj = cJSON_GetObjectItem(obj, "g");
	cJSON *b_obj = cJSON_GetObjectItem(obj, "b");

	if (a_obj)
		color.a = a_obj->valueint;
	if (r_obj)
		color.r = r_obj->valueint;
	if (g_obj)
		color.g = g_obj->valueint;
	if (b_obj)
		color.b = b_obj->valueint;

	drmModeCreatePropertyBlob(dev->fd, &color, sizeof(struct drm_vs_color), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_colorkey(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_colorkey colorkey_data = { 0 };

	cJSON *colorkey_obj = cJSON_GetObjectItem(obj, "colorkey");
	cJSON *colorkey_high_obj = cJSON_GetObjectItem(obj, "colorkey_high");
	cJSON *transparency_obj = cJSON_GetObjectItem(obj, "transparency");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (colorkey_obj)
		colorkey_data.colorkey = colorkey_obj->valueint;
	if (colorkey_high_obj)
		colorkey_data.colorkey_high = colorkey_high_obj->valueint;
	if (transparency_obj)
		colorkey_data.transparency = (bool)transparency_obj->valueint;

	/* colorkey value needs to be no greater than colorkey_high value */
	if (colorkey_data.colorkey > colorkey_data.colorkey_high)
		colorkey_data.colorkey = colorkey_data.colorkey_high;

	drmModeCreatePropertyBlob(dev->fd, &colorkey_data, sizeof(struct drm_vs_colorkey),
				  &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_side_by_side(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	cJSON *left_w_obj = NULL, *right_x_obj = NULL, *right_w_obj = NULL;
	cJSON *sbs_mode_obj = NULL;
	struct drm_vs_sbs sbs = { 0 };

	BLOB_PROP_GLOBAL_DISABLE(obj);

	sbs_mode_obj = cJSON_GetObjectItem(obj, "sbsMode");

	if (!sbs_mode_obj) {
		printf("not found sbsMode config.\n");
		return 0;
	}

	if (!strcmp(sbs_mode_obj->valuestring, "split"))
		sbs.mode = VS_SBS_SPLIT;
	else if (!strcmp(sbs_mode_obj->valuestring, "left"))
		sbs.mode = VS_SBS_LEFT;
	else if (!strcmp(sbs_mode_obj->valuestring, "right"))
		sbs.mode = VS_SBS_RIGHT;

	if (sbs.mode == VS_SBS_SPLIT) {
		left_w_obj = cJSON_GetObjectItem(obj, "leftWidth");
		right_x_obj = cJSON_GetObjectItem(obj, "rightStartX");
		right_w_obj = cJSON_GetObjectItem(obj, "rightWidth");

		if (left_w_obj)
			sbs.left_w = left_w_obj->valueint;
		if (right_x_obj)
			sbs.right_x = right_x_obj->valueint;
		if (right_w_obj)
			sbs.right_w = right_w_obj->valueint;
	}

	drmModeCreatePropertyBlob(dev->fd, &sbs, sizeof(struct drm_vs_sbs), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_blend_mode(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	cJSON *obj_color_mode = NULL, *obj_alpha_mode = NULL;
	struct drm_vs_blend blend = { 0 };
	int temp = 0;

	const struct enum_str_match_list blend_mode_list[] = {
		{ VS_BLD_CLR, "CLEAR" },	 { VS_BLD_SRC, "SRC" },
		{ VS_BLD_DST, "DST" },		 { VS_BLD_SRC_OVR, "SRC_OVER" },
		{ VS_BLD_DST_OVR, "DST_OVER" },	 { VS_BLD_SRC_IN, "SRC_IN" },
		{ VS_BLD_DST_IN, "DST_IN" },	 { VS_BLD_SRC_OUT, "SRC_OUT" },
		{ VS_BLD_DST_OUT, "DST_OUT" },	 { VS_BLD_SRC_ATOP, "SRC_ATOP" },
		{ VS_BLD_DST_ATOP, "DST_ATOP" }, { VS_BLD_XOR, "XOR" },
		{ VS_BLD_PLUS, "PLUS" },	 { VS_BLD_BLD, "BLEND" },
		{ VS_BLD_UDEF, "USER_DEFINE" },
	};

	BLOB_PROP_GLOBAL_DISABLE(obj);

	obj_color_mode = cJSON_GetObjectItem(obj, "colorMode");
	obj_alpha_mode = cJSON_GetObjectItem(obj, "alphaMode");

	if (obj_color_mode) {
		temp = string_to_enum(blend_mode_list, ARRAY_SIZE(blend_mode_list),
				      obj_color_mode->valuestring);
		ARRAY_ENUM_MATCH_FAIL(temp);
		blend.color_mode = temp;
	}

	if (obj_alpha_mode) {
		temp = string_to_enum(blend_mode_list, ARRAY_SIZE(blend_mode_list),
				      obj_alpha_mode->valuestring);
		ARRAY_ENUM_MATCH_FAIL(temp);
		blend.alpha_mode = temp;
	}

	drmModeCreatePropertyBlob(dev->fd, &blend, sizeof(struct drm_vs_blend), &blob_id);
	return blob_id;
}

static uint64_t map_histogram(cJSON *obj, struct device *dev, enum drm_vs_hist_idx hist_idx)
{
	struct drm_vs_histogram histogram;
	uint32_t blob_id = 0;

	cJSON *pos_obj = cJSON_GetObjectItem(obj, "pos");
	cJSON *bin_mode_obj = cJSON_GetObjectItem(obj, "bin_mode");
	cJSON *coef_obj = cJSON_GetObjectItem(obj, "coef");
	cJSON *roi_x_obj = cJSON_GetObjectItem(obj, "roi_x");
	cJSON *roi_y_obj = cJSON_GetObjectItem(obj, "roi_y");
	cJSON *roi_w_obj = cJSON_GetObjectItem(obj, "roi_w");
	cJSON *roi_h_obj = cJSON_GetObjectItem(obj, "roi_h");
	cJSON *block_x_obj = cJSON_GetObjectItem(obj, "block_x");
	cJSON *block_y_obj = cJSON_GetObjectItem(obj, "block_y");
	cJSON *block_w_obj = cJSON_GetObjectItem(obj, "block_w");
	cJSON *block_h_obj = cJSON_GetObjectItem(obj, "block_h");
	cJSON *prot_enable_obj = cJSON_GetObjectItem(obj, "prot_enable");
	cJSON *out_mode_obj = cJSON_GetObjectItem(obj, "out_mode");
	cJSON *global_disable_obj = cJSON_GetObjectItem(obj, "globalDisable");

	if (global_disable_obj && global_disable_obj->valueint) {
		if (dev->histogram_bo[hist_idx])
			bo_destroy(dev->histogram_bo[hist_idx]);
		return 0;
	}

	if (pos_obj)
		histogram.pos = pos_obj->valueint;

	if (bin_mode_obj)
		histogram.bin.bin_mode = bin_mode_obj->valueint;
	if (coef_obj && histogram.bin.bin_mode == VS_HIST_BIN_WEIGHT)
		get_uint32_list(coef_obj, (uint32_t *)histogram.bin.coef);

	if (!roi_x_obj || !roi_y_obj || !roi_w_obj || !roi_h_obj) {
		DTESTLOGE("config of hist roi is incomplete.\n");
		return 0;
	}
	histogram.roi.x = roi_x_obj->valueint;
	histogram.roi.y = roi_y_obj->valueint;
	histogram.roi.w = roi_w_obj->valueint;
	histogram.roi.h = roi_h_obj->valueint;

	if (block_x_obj)
		histogram.block_roi.x = block_x_obj->valueint;
	if (block_y_obj)
		histogram.block_roi.y = block_y_obj->valueint;
	if (block_w_obj)
		histogram.block_roi.w = block_w_obj->valueint;
	if (block_h_obj)
		histogram.block_roi.h = block_h_obj->valueint;

	if (prot_enable_obj && prot_enable_obj->valueint == 1)
		histogram.read_confirm = true;

	if (out_mode_obj)
		histogram.out.out_mode = out_mode_obj->valueint;
	if (!dev->histogram_bo[hist_idx]) {
		dev->histogram_bo[hist_idx] = bo_create_dumb(dev->fd, 256, 3, 32);
		if (dev->histogram_bo[hist_idx] == NULL)
			return 0;
	}
	histogram.out.bo.handle = dev->histogram_bo[hist_idx]->handle;
	histogram.out.bo.fd = dev->fd;

	drmModeCreatePropertyBlob(dev->fd, &histogram, sizeof(struct drm_vs_histogram), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_histogram0(cJSON *obj, struct device *dev)
{
	return map_histogram(obj, dev, VS_HIST_IDX_0);
}

uint64_t MAPAPI map_histogram1(cJSON *obj, struct device *dev)
{
	return map_histogram(obj, dev, VS_HIST_IDX_1);
}

uint64_t MAPAPI map_histogram2(cJSON *obj, struct device *dev)
{
	return map_histogram(obj, dev, VS_HIST_IDX_2);
}

uint64_t MAPAPI map_histogram3(cJSON *obj, struct device *dev)
{
	return map_histogram(obj, dev, VS_HIST_IDX_3);
}

uint64_t MAPAPI map_histogram_rgb(cJSON *obj, struct device *dev)
{
	struct drm_vs_histogram_rgb histogram;
	uint32_t blob_id = 0;

	cJSON *prot_enable_obj = cJSON_GetObjectItem(obj, "prot_enable");
	cJSON *out_mode_obj = cJSON_GetObjectItem(obj, "out_mode");
	cJSON *global_disable_obj = cJSON_GetObjectItem(obj, "globalDisable");

	if (global_disable_obj && global_disable_obj->valueint) {
		if (dev->histogram_bo[VS_HIST_IDX_RGB])
			bo_destroy(dev->histogram_bo[VS_HIST_IDX_RGB]);
		return 0;
	}

	if (prot_enable_obj && prot_enable_obj->valueint == 1)
		histogram.read_confirm = true;

	if (out_mode_obj)
		histogram.out.out_mode = out_mode_obj->valueint;
	if (!dev->histogram_bo[VS_HIST_IDX_RGB]) {
		dev->histogram_bo[VS_HIST_IDX_RGB] = bo_create_dumb(dev->fd, 256, 3, 32);
		if (dev->histogram_bo[VS_HIST_IDX_RGB] == NULL)
			return 0;
	}
	histogram.out.bo.handle = dev->histogram_bo[VS_HIST_IDX_RGB]->handle;
	histogram.out.bo.fd = dev->fd;

	drmModeCreatePropertyBlob(dev->fd, &histogram, sizeof(struct drm_vs_histogram_rgb),
				  &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_hist(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_hist hist;

	/* initialized the histogram index */
	hist.idx = VS_HIST_IDX_COUNT;

	cJSON *idx_obj = cJSON_GetObjectItem(obj, "idx");
	cJSON *hist_enable_obj = cJSON_GetObjectItem(obj, "hist_enable");
	cJSON *pos_obj = cJSON_GetObjectItem(obj, "pos");
	cJSON *bin_mode_obj = cJSON_GetObjectItem(obj, "bin_mode");
	cJSON *coef_obj = cJSON_GetObjectItem(obj, "coef");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (idx_obj)
		hist.idx = idx_obj->valueint;
	if (hist_enable_obj)
		hist.hist_enable = hist_enable_obj->valueint;
	if (pos_obj)
		hist.pos = pos_obj->valueint;

	if (hist.idx != VS_HIST_IDX_RGB && bin_mode_obj && coef_obj) {
		hist.bin_mode = bin_mode_obj->valueint;

		if (hist.bin_mode == VS_HIST_BIN_WEIGHT)
			get_uint32_list(coef_obj, (uint32_t *)hist.coef);
	}

	drmModeCreatePropertyBlob(dev->fd, &hist, sizeof(struct drm_vs_hist), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_hist_roi(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_hist_roi hist_roi;

	cJSON *idx_obj = cJSON_GetObjectItem(obj, "idx");
	cJSON *x_obj = cJSON_GetObjectItem(obj, "x");
	cJSON *y_obj = cJSON_GetObjectItem(obj, "y");
	cJSON *w_obj = cJSON_GetObjectItem(obj, "w");
	cJSON *h_obj = cJSON_GetObjectItem(obj, "h");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (!idx_obj || !x_obj || !y_obj || !w_obj || !h_obj) {
		printf("config of hist roi is incomplete.\n");
		return 0;
	}

	hist_roi.idx = idx_obj->valueint;
	hist_roi.rect.x = x_obj->valueint;
	hist_roi.rect.y = y_obj->valueint;
	hist_roi.rect.w = w_obj->valueint;
	hist_roi.rect.h = h_obj->valueint;

	drmModeCreatePropertyBlob(dev->fd, &hist_roi, sizeof(struct drm_vs_hist_roi), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_hist_block(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_hist_block hist_block;

	cJSON *idx_obj = cJSON_GetObjectItem(obj, "idx");
	cJSON *x_obj = cJSON_GetObjectItem(obj, "x");
	cJSON *y_obj = cJSON_GetObjectItem(obj, "y");
	cJSON *w_obj = cJSON_GetObjectItem(obj, "w");
	cJSON *h_obj = cJSON_GetObjectItem(obj, "h");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (!idx_obj || !x_obj || !y_obj || !w_obj || !h_obj) {
		printf("config of hist block is incomplete.\n");
		return 0;
	}

	hist_block.idx = idx_obj->valueint;
	hist_block.rect.x = x_obj->valueint;
	hist_block.rect.y = y_obj->valueint;
	hist_block.rect.w = w_obj->valueint;
	hist_block.rect.h = h_obj->valueint;

	drmModeCreatePropertyBlob(dev->fd, &hist_block, sizeof(struct drm_vs_hist_block), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_hist_prot(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_hist_prot hist_prot;

	cJSON *idx_obj = cJSON_GetObjectItem(obj, "idx");
	cJSON *prot_enable_obj = cJSON_GetObjectItem(obj, "prot_enable");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (idx_obj)
		hist_prot.idx = idx_obj->valueint;
	if (prot_enable_obj)
		hist_prot.prot_enable = prot_enable_obj->valueint;

	drmModeCreatePropertyBlob(dev->fd, &hist_prot, sizeof(struct drm_vs_hist_prot), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_hist_info(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_hist_info hist_info;
	cJSON *idx_obj = cJSON_GetObjectItem(obj, "idx");
	cJSON *out_mode_obj = cJSON_GetObjectItem(obj, "out_mode");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (dev->frame_id == 0) {
		dev->hist_bo = bo_create_dumb(dev->fd, 256, 3, 32);
		if (dev->hist_bo == NULL)
			return 0;
	}

	if (idx_obj)
		hist_info.idx = idx_obj->valueint;
	if (out_mode_obj)
		hist_info.out_mode = out_mode_obj->valueint;
	hist_info.fd = dev->fd;
	hist_info.hist_bo_handle = dev->hist_bo->handle;

	drmModeCreatePropertyBlob(dev->fd, &hist_info, sizeof(struct drm_vs_hist_info), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_ops(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	int i = 0;
	struct drm_vs_ops ops = { 0 };
	struct drm_color_lut lut[256] = { 0 };
	cJSON *curve_type_obj = cJSON_GetObjectItem(obj, "curve_type");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	drm_vs_init_gamma_lut(1, curve_type_obj->valuestring, 2.2, 10, 256, lut);

	for (i = 0; i < 256; i++)
		ops.lut[i] = lut[i].red; /* each channel use the same coef */

	drmModeCreatePropertyBlob(dev->fd, &ops, sizeof(struct drm_vs_ops), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_wb_spliter(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_wb_spliter spliter;

	cJSON *x0_obj = cJSON_GetObjectItem(obj, "split0X");
	cJSON *w0_obj = cJSON_GetObjectItem(obj, "split0Width");
	cJSON *x1_obj = cJSON_GetObjectItem(obj, "split1X");
	cJSON *w1_obj = cJSON_GetObjectItem(obj, "split1Width");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (x0_obj)
		spliter.split_rect0.x = x0_obj->valueint;
	if (w0_obj)
		spliter.split_rect0.w = w0_obj->valueint;
	if (x1_obj)
		spliter.split_rect1.x = x1_obj->valueint;
	if (w1_obj)
		spliter.split_rect1.w = w1_obj->valueint;

	drmModeCreatePropertyBlob(dev->fd, &spliter, sizeof(struct drm_vs_wb_spliter), &blob_id);
	printf("Create a blob, id=%u\n", blob_id);
	return blob_id;
}

uint64_t MAPAPI map_dsc(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_dsc dsc = { 0 };

	cJSON *picture_width_obj = cJSON_GetObjectItem(obj, "picture_width");
	cJSON *picture_height_obj = cJSON_GetObjectItem(obj, "picture_height");
	cJSON *slices_per_line_obj = cJSON_GetObjectItem(obj, "slices_per_line");
	cJSON *slice_height_obj = cJSON_GetObjectItem(obj, "slice_height");
	cJSON *ss_num_obj = cJSON_GetObjectItem(obj, "ss_num");
	cJSON *split_panel_enable_obj = cJSON_GetObjectItem(obj, "split_panel_enable");
	cJSON *multiplex_mode_enable_obj = cJSON_GetObjectItem(obj, "multiplex_mode_enable");
	cJSON *multiplex_out_sel_obj = cJSON_GetObjectItem(obj, "multiplex_out_sel");
	cJSON *de_raster_enable_obj = cJSON_GetObjectItem(obj, "de_raster_enable");
	cJSON *multiplex_eoc_enable_obj = cJSON_GetObjectItem(obj, "multiplex_eoc_enable");
	cJSON *video_mode_obj = cJSON_GetObjectItem(obj, "video_mode");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (!picture_width_obj || !picture_height_obj || !slices_per_line_obj ||
	    !slice_height_obj || !ss_num_obj || !split_panel_enable_obj ||
	    !multiplex_mode_enable_obj || !multiplex_out_sel_obj || !de_raster_enable_obj ||
	    !multiplex_eoc_enable_obj || !video_mode_obj) {
		printf("config of DSC is incomplete.\n");
		return 0;
	}

	dsc.picture_width = (__u16)picture_width_obj->valueint;
	dsc.picture_height = (__u16)picture_height_obj->valueint;
	dsc.slices_per_line = (__u8)slices_per_line_obj->valueint;
	dsc.slice_height = (__u16)slice_height_obj->valueint;
	dsc.ss_num = (__u8)ss_num_obj->valueint;
	dsc.split_panel_enable = (bool)split_panel_enable_obj->valueint;
	dsc.multiplex_mode_enable = (bool)multiplex_mode_enable_obj->valueint;
	dsc.multiplex_out_sel = (int)multiplex_out_sel_obj->valueint;
	dsc.de_raster_enable = (bool)de_raster_enable_obj->valueint;
	dsc.multiplex_eoc_enable = (bool)multiplex_eoc_enable_obj->valueint;
	dsc.video_mode = (bool)video_mode_obj->valueint;

	drmModeCreatePropertyBlob(dev->fd, &dsc, sizeof(dsc), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_vdc(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_vdc vdc = { 0 };

	cJSON *slices_per_line_obj = cJSON_GetObjectItem(obj, "slices_per_line");
	cJSON *slice_height_obj = cJSON_GetObjectItem(obj, "slice_height");
	cJSON *ss_num_obj = cJSON_GetObjectItem(obj, "ss_num");
	cJSON *split_panel_enable_obj = cJSON_GetObjectItem(obj, "split_panel_enable");
	cJSON *multiplex_mode_enable_obj = cJSON_GetObjectItem(obj, "multiplex_mode_enable");
	cJSON *multiplex_out_sel_obj = cJSON_GetObjectItem(obj, "multiplex_out_sel");
	cJSON *multiplex_eoc_enable_obj = cJSON_GetObjectItem(obj, "multiplex_eoc_enable");
	cJSON *hs_split_input_enable_obj = cJSON_GetObjectItem(obj, "hs_split_input_enable");
	cJSON *video_mode_obj = cJSON_GetObjectItem(obj, "video_mode");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (!slices_per_line_obj || !slice_height_obj || !ss_num_obj || !split_panel_enable_obj ||
	    !multiplex_mode_enable_obj || !multiplex_out_sel_obj || !multiplex_eoc_enable_obj ||
	    !hs_split_input_enable_obj || !video_mode_obj) {
		printf("config of VDC is incomplete.\n");
		return 0;
	}

	vdc.slices_per_line = (__u8)slices_per_line_obj->valueint;
	vdc.slice_height = (__u16)slice_height_obj->valueint;
	vdc.ss_num = (__u8)ss_num_obj->valueint;
	vdc.split_panel_enable = (bool)split_panel_enable_obj->valueint;
	vdc.multiplex_mode_enable = (bool)multiplex_mode_enable_obj->valueint;
	vdc.multiplex_out_sel = (int)multiplex_out_sel_obj->valueint;
	vdc.multiplex_eoc_enable = (bool)multiplex_eoc_enable_obj->valueint;
	vdc.hs_split_input_enable = (bool)hs_split_input_enable_obj->valueint;
	vdc.video_mode = (bool)video_mode_obj->valueint;

	drmModeCreatePropertyBlob(dev->fd, &vdc, sizeof(vdc), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_spliter(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_spliter spliter = { 0 };

	cJSON *left_x_obj = cJSON_GetObjectItem(obj, "leftX");
	cJSON *left_w_obj = cJSON_GetObjectItem(obj, "leftWidth");
	cJSON *right_x_obj = cJSON_GetObjectItem(obj, "rightX");
	cJSON *right_w_obj = cJSON_GetObjectItem(obj, "rightWidth");
	cJSON *src_w_obj = cJSON_GetObjectItem(obj, "srcWidth");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (!left_x_obj || !left_w_obj || !right_x_obj || !right_w_obj || !src_w_obj) {
		printf("config for spliter is incomplete.\n");
		return 0;
	}
	spliter.left_x = left_x_obj->valueint;
	spliter.left_w = left_w_obj->valueint;
	spliter.right_x = right_x_obj->valueint;
	spliter.right_w = right_w_obj->valueint;
	spliter.src_w = src_w_obj->valueint;

	drmModeCreatePropertyBlob(dev->fd, &spliter, sizeof(struct drm_vs_spliter), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_ramless_fb(cJSON *obj, struct device *dev)
{
	cJSON *ramless_enable_obj = cJSON_GetObjectItem(obj, "enable");
	bool ramless_enable = false;

	cJSON *global_disable_obj = cJSON_GetObjectItem(obj, "globalDisable");

	if (global_disable_obj) {
		if (global_disable_obj->valueint)
			ramless_enable = false;
	}

	if (ramless_enable_obj)
		ramless_enable = !!ramless_enable_obj->valueint;

	if (!ramless_enable) {
		dev->ramless.fb_inuse = 0;
		return 0;
	} else {
		dev->ramless.fb_inuse = 1;
	}

	return dev->ramless.ramless_fb_id;
}

uint64_t MAPAPI map_bool(cJSON *obj, struct device *dev)
{
	return (uint64_t)obj->valueint;
}

uint64_t MAPAPI map_ctm(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0, arr_size = 0, i = 0, matrix_num = 0;
	struct drm_color_ctm ctm = { 0 };

	cJSON *matrix_obj = cJSON_GetObjectItem(obj, "matrix");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	arr_size = cJSON_GetArraySize(matrix_obj);
	matrix_num = sizeof(ctm.matrix) / sizeof(__u64);
	for (i = 0; i < arr_size && i < matrix_num; i++) {
		cJSON *item = cJSON_GetArrayItem(matrix_obj, i);
		ctm.matrix[i] = item->valueint;
	}

	drmModeCreatePropertyBlob(dev->fd, &ctm, sizeof(struct drm_color_ctm), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_sr2000(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	uint32_t dc9000sr_csc_bits = 10; /*if other series,should refine*/
	struct drm_vs_sr2000 sr = { 0 };
	int out_width = 0, out_height = 0;
	cJSON *obj_r2y_programmable = NULL;
	cJSON *obj_y2r_programmable = NULL;
	cJSON *sr_enable_obj = cJSON_GetObjectItem(obj, "sr_enable");
	bool sr_enable = false;

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (sr_enable_obj)
		sr_enable = sr_enable_obj->valueint;

	if (sr_enable) {
		cJSON *r2y = cJSON_GetObjectItem(obj, "r2y");
		cJSON *scaler = cJSON_GetObjectItem(obj, "scaler");
		cJSON *nr = cJSON_GetObjectItem(obj, "nr");
		cJSON *ssr3 = cJSON_GetObjectItem(obj, "ssr3");
		cJSON *y2r = cJSON_GetObjectItem(obj, "y2r");

		if (r2y) {
			cJSON *r2y_enable_obj = cJSON_GetObjectItem(r2y, "r2y_enable");
			if (r2y_enable_obj)
				sr.r2y_enable = r2y_enable_obj->valueint;
		}
		if (scaler) {
			cJSON *scale_enable_obj = cJSON_GetObjectItem(scaler, "scaler_enable");
			if (scale_enable_obj)
				sr.scale_enable = scale_enable_obj->valueint;
		}
		if (nr) {
			cJSON *nr_enable_obj = cJSON_GetObjectItem(nr, "nr_enable");
			if (nr_enable_obj)
				sr.nr_enable = nr_enable_obj->valueint;
		}
		if (ssr3) {
			cJSON *ssr3_enable_obj = cJSON_GetObjectItem(ssr3, "ssr3_enable");
			if (ssr3_enable_obj)
				sr.ssr3_enable = ssr3_enable_obj->valueint;
		}
		if (y2r) {
			cJSON *y2r_enable_obj = cJSON_GetObjectItem(y2r, "y2r_enable");
			if (y2r_enable_obj)
				sr.y2r_enable = y2r_enable_obj->valueint;
		}

		/* R2Y */
		if (sr.r2y_enable) {
			cJSON *r2y_mode_obj = cJSON_GetObjectItem(r2y, "r2y_mode");
			if (!r2y_mode_obj) {
				fprintf(stderr, "not found r2y_mode config for SR2000.\n");
				return 0;
			}
			const char *r2y_mode = r2y_mode_obj->valuestring;
			if (!strcmp(r2y_mode, "USR"))
				sr.r2y.mode = VS_CSC_CM_USR;
			else if (!strcmp(r2y_mode, "L2L"))
				sr.r2y.mode = VS_CSC_CM_L2L;
			else if (!strcmp(r2y_mode, "L2F"))
				sr.r2y.mode = VS_CSC_CM_L2F;
			else if (!strcmp(r2y_mode, "F2L"))
				sr.r2y.mode = VS_CSC_CM_F2L;
			else if (!strcmp(r2y_mode, "F2F"))
				sr.r2y.mode = VS_CSC_CM_F2F;
			else {
				fprintf(stderr, "Unknown r2y_mode %s\n", r2y_mode);
				return 0;
			}

			if (sr.r2y.mode == VS_CSC_CM_USR) {
				if ((obj_r2y_programmable =
					     cJSON_GetObjectItem(r2y, "r2y_Programmable"))) {
					const char *r2y_Programmable =
						obj_r2y_programmable->valuestring;
					if (dtest_get_csc_coef_from_csv(r2y_Programmable,
									dc9000sr_csc_bits,
									sr.r2y.coef) != 0) {
						fprintf(stderr,
							"Get SR2000 r2y coef from csv failed\n");
						return 0;
					}
				}
				/* if  "r2y_Programmable" is NULL, recceives the JSON obj from Tool */
				else {
					printf("get r2y eotf coef from Tool json\n");
					get_int_list(cJSON_GetObjectItem(r2y, "coef"),
						     (int *)sr.r2y.coef);
				}
			}

			cJSON *r2y_gamut_obj = cJSON_GetObjectItem(r2y, "r2y_gamut");
			if (!r2y_gamut_obj) {
				fprintf(stderr, "not found r2y_gamut_mode config for SR2000.\n");
				return 0;
			}
			const char *r2y_gamut_mode = r2y_gamut_obj->valuestring;
			if (!strcmp(r2y_gamut_mode, "BT601"))
				sr.r2y.gamut = VS_CSC_CG_601;
			else if (!strcmp(r2y_gamut_mode, "BT709"))
				sr.r2y.gamut = VS_CSC_CG_709;
			else if (!strcmp(r2y_gamut_mode, "BT2020"))
				sr.r2y.gamut = VS_CSC_CG_2020;
			else if (!strcmp(r2y_gamut_mode, "DCIP3"))
				sr.r2y.gamut = VS_CSC_CG_P3;
			else if (!strcmp(r2y_gamut_mode, "SRGB"))
				sr.r2y.gamut = VS_CSC_CG_SRGB;
			else {
				fprintf(stderr, "Unknown r2y_gamut_mode %s\n", r2y_gamut_mode);
				return 0;
			}
		}

		/* scale */
		if (sr.scale_enable) {
			int16_t temp_coef[VS_SCALE_HORI_COEF_NUM * 2] = { 0 };
			/* when scale_factor_set = vsFALSE: the result is close to opencv's result, but not exactly match */
			bool scale_factor_set = (bool)1;

			/*different with comdel driver*/
			cJSON *src_w_obj = cJSON_GetObjectItem(scaler, "src_w");
			cJSON *src_h_obj = cJSON_GetObjectItem(scaler, "src_h");
			cJSON *dst_w_obj = cJSON_GetObjectItem(scaler, "dst_w");
			cJSON *dst_h_obj = cJSON_GetObjectItem(scaler, "dst_h");
			if (!src_w_obj || !src_h_obj || !dst_w_obj || !dst_h_obj) {
				fprintf(stderr, "config for SR2000 scale is incomplete.\n");
				return 0;
			}
			sr.scale.src_w = src_w_obj->valueint;
			sr.scale.src_h = src_h_obj->valueint;
			sr.scale.dst_w = dst_w_obj->valueint;
			sr.scale.dst_h = dst_h_obj->valueint;

			sr.scale.factor_x = drm_vs_get_stretch_factor(
				sr.scale.src_w, sr.scale.dst_w, scale_factor_set);
			sr.scale.factor_y = drm_vs_get_stretch_factor(
				sr.scale.src_h, sr.scale.dst_h, scale_factor_set);

			drm_vs_calculate_sync_table(9, sr.scale.src_w, sr.scale.dst_w, temp_coef,
						    VS_H9_V5);
			memcpy(sr.scale.coef_h, temp_coef,
			       VS_SUBPIXELLOADCOUNT * 9 * sizeof(int16_t));
			drm_vs_calculate_sync_table(5, sr.scale.src_h, sr.scale.dst_h, temp_coef,
						    VS_H9_V5);
			memcpy(sr.scale.coef_v, temp_coef,
			       VS_SUBPIXELLOADCOUNT * 5 * sizeof(int16_t));

			sr.scale.initial_offsetx =
				drm_vs_get_stretch_initOffset(sr.scale.factor_x, scale_factor_set);
			sr.scale.initial_offsety =
				drm_vs_get_stretch_initOffset(sr.scale.factor_y, scale_factor_set);
		}

		/* nr */
		if (sr.nr_enable) {
			cJSON *nr_mode_obj = cJSON_GetObjectItem(nr, "nr_mode");
			if (!nr_mode_obj) {
				fprintf(stderr, "not found config for SR2000 nr mode.\n");
				return 0;
			}
			const char *nr_mode = nr_mode_obj->valuestring;
			if (!strcmp(nr_mode, "weak"))
				memcpy(sr.nr.coef, NR_weak, VS_NR_COEF_NUM * sizeof(uint32_t));
			else if (!strcmp(nr_mode, "strong"))
				memcpy(sr.nr.coef, NR_strong, VS_NR_COEF_NUM * sizeof(uint32_t));
			else {
				fprintf(stderr, "Unknown nr_mode %s\n", nr_mode);
				return 0;
			}
		}

		/* ssr3 */
		if (sr.ssr3_enable) {
			cJSON *ssr3_mode_obj = cJSON_GetObjectItem(ssr3, "ssr3_mode");
			if (!ssr3_mode_obj) {
				fprintf(stderr, "not found config for SR2000 ssr3 mode.\n");
				return 0;
			}
			const char *ssr3_mode = ssr3_mode_obj->valuestring;
			if (!strcmp(ssr3_mode, "strongUps"))
				memcpy(sr.ssr3.coef, SSR3_strongUps,
				       VS_SSR3_COEF_NUM * sizeof(uint32_t));
			else if (!strcmp(ssr3_mode, "TV1"))
				memcpy(sr.ssr3.coef, SSR3_TV1, VS_SSR3_COEF_NUM * sizeof(uint32_t));
			else if (!strcmp(ssr3_mode, "TV2"))
				memcpy(sr.ssr3.coef, SSR3_TV2, VS_SSR3_COEF_NUM * sizeof(uint32_t));
			else if (!strcmp(ssr3_mode, "TV3"))
				memcpy(sr.ssr3.coef, SSR3_TV3, VS_SSR3_COEF_NUM * sizeof(uint32_t));
			else if (!strcmp(ssr3_mode, "DESK"))
				memcpy(sr.ssr3.coef, SSR3_DESK,
				       VS_SSR3_COEF_NUM * sizeof(uint32_t));
			else if (!strcmp(ssr3_mode, "GAME"))
				memcpy(sr.ssr3.coef, SSR3_GAME,
				       VS_SSR3_COEF_NUM * sizeof(uint32_t));
			else if (!strcmp(ssr3_mode, "NATURE"))
				memcpy(sr.ssr3.coef, SSR3_NATURE,
				       VS_SSR3_COEF_NUM * sizeof(uint32_t));
			else if (!strcmp(ssr3_mode, "DEFAULT"))
				memcpy(sr.ssr3.coef, SSR3_DEFAULT,
				       VS_SSR3_COEF_NUM * sizeof(uint32_t));
			else {
				fprintf(stderr, "Unknown ssr3_mode %s\n", ssr3_mode);
				return 0;
			}

			/* from RTL*/
			if (sr.scale_enable) {
				out_width = sr.scale.dst_w;
				out_height = sr.scale.dst_h;
			} else {
				out_width = dev->mode.width;
				out_height = dev->mode.height;
			}

			sr.ssr3.DepHWid = 0x30; // followup dv hard code.
			sr.ssr3.DepVWid = 0x22; // HW Fix config.
			ssr3_get_ratio_and_shift(sr.ssr3.DepHWid, out_width, &sr.ssr3.HWidRatio,
						 &sr.ssr3.HRatioSft);
			ssr3_get_ratio_and_shift(sr.ssr3.DepVWid, out_height, &sr.ssr3.VWidRatio,
						 &sr.ssr3.VRatioSft);

			if (out_width < 3025) //DepthHDownFactor
				sr.ssr3.coef[114] = (out_width + 47) / 48;
			else if (out_width < 4561)
				sr.ssr3.coef[114] = ((out_width + 47) / 48 / 2) * 2;
			else if (out_width < 6097)
				sr.ssr3.coef[114] = ((out_width + 47) / 48 / 3) * 3;
			else
				sr.ssr3.coef[114] = ((out_width + 47) / 48 / 4) * 4;

			if (out_height < 2143) //DepthVDownFactor
				sr.ssr3.coef[115] = (out_height + 33) / 34;
			else if (out_height < 3231)
				sr.ssr3.coef[115] = ((out_height + 33) / 34 / 2) * 2;
			else if (out_height < 4319)
				sr.ssr3.coef[115] = ((out_height + 33) / 34 / 3) * 3;
			else
				sr.ssr3.coef[115] = ((out_height + 33) / 34 / 4) * 4;
		}

		/* Y2R */
		if (sr.y2r_enable) {
			cJSON *y2r_mode_obj = cJSON_GetObjectItem(y2r, "y2r_mode");
			if (!y2r_mode_obj) {
				fprintf(stderr, "not found config for SR2000 y2r mode.\n");
				return 0;
			}
			const char *y2r_mode = y2r_mode_obj->valuestring;
			if (!strcmp(y2r_mode, "USR"))
				sr.y2r.mode = VS_CSC_CM_USR;
			else if (!strcmp(y2r_mode, "L2L"))
				sr.y2r.mode = VS_CSC_CM_L2L;
			else if (!strcmp(y2r_mode, "L2F"))
				sr.y2r.mode = VS_CSC_CM_L2F;
			else if (!strcmp(y2r_mode, "F2L"))
				sr.y2r.mode = VS_CSC_CM_F2L;
			else if (!strcmp(y2r_mode, "F2F"))
				sr.y2r.mode = VS_CSC_CM_F2F;
			else {
				fprintf(stderr, "Unknown y2r_mode %s\n", y2r_mode);
				return 0;
			}

			if (sr.y2r.mode == VS_CSC_CM_USR) {
				if ((obj_y2r_programmable =
					     cJSON_GetObjectItem(y2r, "y2r_Programmable"))) {
					const char *y2r_Programmable =
						obj_y2r_programmable->valuestring;
					if (dtest_get_csc_coef_from_csv(y2r_Programmable,
									dc9000sr_csc_bits,
									sr.y2r.coef) != 0) {
						fprintf(stderr,
							"Get SR2000 y2r coef from csv failed\n");
						return 0;
					}
				}
				/* if  "y2r_Programmable" is NULL, recceives the JSON obj from Tool */
				else {
					printf("get y2r eotf coef from Tool json\n");
					get_int_list(cJSON_GetObjectItem(y2r, "coef"),
						     (int *)sr.y2r.coef);
				}
			}

			cJSON *y2r_gamut_obj = cJSON_GetObjectItem(y2r, "y2r_gamut");
			if (!y2r_gamut_obj) {
				fprintf(stderr, "not found y2r_gamut_mode config for SR2000.\n");
				return 0;
			}
			const char *y2r_gamut_mode = y2r_gamut_obj->valuestring;
			if (!strcmp(y2r_gamut_mode, "BT601"))
				sr.y2r.gamut = VS_CSC_CG_601;
			else if (!strcmp(y2r_gamut_mode, "BT709"))
				sr.y2r.gamut = VS_CSC_CG_709;
			else if (!strcmp(y2r_gamut_mode, "BT2020"))
				sr.y2r.gamut = VS_CSC_CG_2020;
			else if (!strcmp(y2r_gamut_mode, "DCIP3"))
				sr.y2r.gamut = VS_CSC_CG_P3;
			else if (!strcmp(y2r_gamut_mode, "SRGB"))
				sr.y2r.gamut = VS_CSC_CG_SRGB;
			else {
				fprintf(stderr, "Unknown y2r_gamut_mode %s\n", y2r_gamut_mode);
				return 0;
			}
		}
	} else {
		sr.r2y_enable = 0;
		sr.scale_enable = 0;
		sr.nr_enable = 0;
		sr.ssr3_enable = 0;
		sr.y2r_enable = 0;
	}

	drmModeCreatePropertyBlob(dev->fd, &sr, sizeof(struct drm_vs_sr2000), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_dec_fc(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct dc_dec400_fc dec400_fc = { 0 };
	cJSON *fc_enable = cJSON_GetObjectItem(obj, "fcEnable");
	cJSON *fc_size = cJSON_GetObjectItem(obj, "fcSize");
	cJSON *fc_rgby = cJSON_GetObjectItem(obj, "fcRGBY");
	cJSON *fc_uv = cJSON_GetObjectItem(obj, "fcUV");

	BLOB_PROP_GLOBAL_DISABLE(obj);

	if (fc_enable)
		dec400_fc.fc_enable = fc_enable->valueint;
	if (fc_size)
		dec400_fc.fc_size = fc_size->valueint;
	else
		dec400_fc.fc_size = 128;
	if (fc_rgby)
		dec400_fc.fc_rgby_value = (__u32)strtoul(fc_rgby->valuestring, NULL, 16);
	if (fc_uv)
		dec400_fc.fc_uv_value = (__u32)strtoul(fc_uv->valuestring, NULL, 16);

	drmModeCreatePropertyBlob(dev->fd, &dec400_fc, sizeof(struct dc_dec400_fc), &blob_id);
	return blob_id;
}

uint64_t MAPAPI map_ctx_id(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_ctx_id ctx_id;
	int stream_id = 0;
	cJSON *stream_id_obj = cJSON_GetObjectItem(obj, "stream_id");

	if (stream_id_obj)
		stream_id = stream_id_obj->valueint;

	ctx_id.id = dev->ctx_id_list[stream_id];

	drmModeCreatePropertyBlob(dev->fd, &ctx_id, sizeof(struct drm_vs_ctx_id), &blob_id);

	return blob_id;
}

static void fill_dbi_correction_coef(struct drm_vs_dbi_correction_coef *coef, cJSON *obj)
{
	/* red chanel */
	cJSON *slope_m_red_obj = cJSON_GetObjectItem(obj, "slope_m_red");
	cJSON *slope_a_red_obj = cJSON_GetObjectItem(obj, "slope_a_red");
	cJSON *intercept_m_red_obj = cJSON_GetObjectItem(obj, "intercept_m_red");
	cJSON *intercept_a_red_obj = cJSON_GetObjectItem(obj, "intercept_a_red");
	cJSON *threshold_red_obj = cJSON_GetObjectItem(obj, "threshold_red");
	/* green chanel */
	cJSON *slope_m_green_obj = cJSON_GetObjectItem(obj, "slope_m_green");
	cJSON *slope_a_green_obj = cJSON_GetObjectItem(obj, "slope_a_green");
	cJSON *intercept_m_green_obj = cJSON_GetObjectItem(obj, "intercept_m_green");
	cJSON *intercept_a_green_obj = cJSON_GetObjectItem(obj, "intercept_a_green");
	cJSON *threshold_green_obj = cJSON_GetObjectItem(obj, "threshold_green");
	/* blue chanel */
	cJSON *slope_m_blue_obj = cJSON_GetObjectItem(obj, "slope_m_blue");
	cJSON *slope_a_blue_obj = cJSON_GetObjectItem(obj, "slope_a_blue");
	cJSON *intercept_m_blue_obj = cJSON_GetObjectItem(obj, "intercept_m_blue");
	cJSON *intercept_a_blue_obj = cJSON_GetObjectItem(obj, "intercept_a_blue");
	cJSON *threshold_blue_obj = cJSON_GetObjectItem(obj, "threshold_blue");

	if (slope_m_red_obj)
		get_int_list(slope_m_red_obj, coef->slope_m_r);
	if (slope_a_red_obj)
		get_int_list(slope_a_red_obj, coef->slope_a_r);
	if (intercept_m_red_obj)
		get_int_list(intercept_m_red_obj, coef->intercept_m_r);
	if (intercept_a_red_obj)
		get_int_list(intercept_a_red_obj, coef->intercept_a_r);
	if (threshold_red_obj)
		get_uint32_list(threshold_red_obj, coef->area_threshold_r);

	if (slope_m_green_obj)
		get_int_list(slope_m_green_obj, coef->slope_m_g);
	if (slope_a_green_obj)
		get_int_list(slope_a_green_obj, coef->slope_a_g);
	if (intercept_m_green_obj)
		get_int_list(intercept_m_green_obj, coef->intercept_m_g);
	if (intercept_a_green_obj)
		get_int_list(intercept_a_green_obj, coef->intercept_a_g);
	if (threshold_green_obj)
		get_uint32_list(threshold_green_obj, coef->area_threshold_g);

	if (slope_m_blue_obj)
		get_int_list(slope_m_blue_obj, coef->slope_m_b);
	if (slope_a_blue_obj)
		get_int_list(slope_a_blue_obj, coef->slope_a_b);
	if (intercept_m_blue_obj)
		get_int_list(intercept_m_blue_obj, coef->intercept_m_b);
	if (intercept_a_blue_obj)
		get_int_list(intercept_a_blue_obj, coef->intercept_a_b);
	if (threshold_blue_obj)
		get_uint32_list(threshold_blue_obj, coef->area_threshold_b);
}

static void fill_dbi_lut_bo(struct bo *bo, cJSON *obj, uint32_t aligned_width,
			    uint32_t aligned_height, uint32_t stride)
{
	char file_name[RESOURCE_LEN] = { 0 }, file[RESOURCE_LEN + 9] = { 0 };
	FILE *fp = NULL;
	uint32_t ret = 0, x = 0, y = 0, group = 0, index = 0;
	uint32_t lut_width = 0, lut_height = 0, lut_max = 0, bit_length = 0, byte_length = 0;
	void *va_addr = { 0 };
	uint8_t *r = NULL, *g = NULL, *b = NULL;
	char header[18];
	char *dot = NULL;

	if (strlen(obj->valuestring) > 0 && strlen(obj->valuestring) < (RESOURCE_LEN + 1)) {
		strncpy(file_name, obj->valuestring, strlen(obj->valuestring));
	} else {
		DTESTLOGE("invalid lut name\n");
		return;
	}

	dot = strrchr(file_name, '.');
	if (!dot || strlen(dot) < 4 || strcmp(dot + 1, "ppm") != 0) {
		DTESTLOGE("invalid file:%s", file);
		goto exit;
	}

	sprintf(file, "resource/%s", file_name);
	fp = fopen(file, "rb");
	if (NULL == fp) {
		DTESTLOGE("Open %s fail", file);
		goto exit;
	}

	fread(header, sizeof(char), 18, fp);
	lut_width = value_in_range(header[3] - '0', 0, 9) * 1000 +
		    value_in_range(header[4] - '0', 0, 9) * 100 +
		    value_in_range(header[5] - '0', 0, 9) * 10 +
		    value_in_range(header[6] - '0', 0, 9);
	lut_height = value_in_range(header[8] - '0', 0, 9) * 1000 +
		     value_in_range(header[9] - '0', 0, 9) * 100 +
		     value_in_range(header[10] - '0', 0, 9) * 10 +
		     value_in_range(header[11] - '0', 0, 9);
	if (lut_width != aligned_width || lut_height != aligned_height) {
		DTESTLOGE("invalid lut\n");
		goto exit;
	}

	lut_max = value_in_range(header[13] - '0', 0, 9) * 1000 +
		  value_in_range(header[14] - '0', 0, 9) * 100 +
		  value_in_range(header[15] - '0', 0, 9) * 10 +
		  value_in_range(header[16] - '0', 0, 9);
	do {
		bit_length++;
		lut_max >>= 1;
	} while (lut_max != 0);
	byte_length = (bit_length + 7) / 8;
	r = (uint8_t *)calloc(byte_length, sizeof(uint8_t));
	g = (uint8_t *)calloc(byte_length, sizeof(uint8_t));
	b = (uint8_t *)calloc(byte_length, sizeof(uint8_t));

	ret = bo_map(bo, &va_addr);
	if (ret) {
		DTESTLOGE("failed to map buffer: %s", strerror(-errno));
		bo_destroy(bo);
		goto exit;
	}

	for (y = 0; y < aligned_height; ++y) {
		for (x = 0; x < aligned_width; ++x) {
			//need to be aligned to 32Byte, one tile's lut occupies 3 Byte
			group = x / 10;
			index = x % 10;
			fread(r, sizeof(uint8_t), byte_length, fp);
			fread(g, sizeof(uint8_t), byte_length, fp);
			fread(b, sizeof(uint8_t), byte_length, fp);
			memcpy((uint8_t *)va_addr + y * stride + group * 32 + index * 3,
			       &b[byte_length - 1], 1);
			memcpy((uint8_t *)va_addr + y * stride + group * 32 + index * 3 + 1,
			       &g[byte_length - 1], 1);
			memcpy((uint8_t *)va_addr + y * stride + group * 32 + index * 3 + 2,
			       &r[byte_length - 1], 1);
		}
	}

exit:
	bo_unmap(bo);

	if (r)
		free(r);
	if (g)
		free(g);
	if (b)
		free(b);
	if (fp)
		fclose(fp);
}

uint64_t MAPAPI map_dbi(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0, aligned_width = 0, aligned_stride = 0, aligned_height = 0;
	uint32_t panel_width = 0, panel_height = 0;
	struct drm_vs_dbi dbi = { 0 };
	cJSON *panel_width_obj = cJSON_GetObjectItem(obj, "panel_width");
	cJSON *panel_height_obj = cJSON_GetObjectItem(obj, "panel_height");
	cJSON *roi0_ctl_obj = cJSON_GetObjectItem(obj, "ROI0_DBI_CONTROL");
	cJSON *roi1_ctl_obj = cJSON_GetObjectItem(obj, "ROI1_DBI_CONTROL");
	cJSON *roi2_ctl_obj = cJSON_GetObjectItem(obj, "ROI2_DBI_CONTROL");
	cJSON *udc0_ctl_obj = cJSON_GetObjectItem(obj, "UDC0_DBI_CONTROL");
	cJSON *udc1_ctl_obj = cJSON_GetObjectItem(obj, "UDC1_DBI_CONTROL");
	cJSON *dither_obj = cJSON_GetObjectItem(obj, "DITHER");
	cJSON *main_lut_obj = cJSON_GetObjectItem(obj, "MAIN_LUT");
	cJSON *udc0_lut_obj = cJSON_GetObjectItem(obj, "UDC0_LUT");
	cJSON *udc1_lut_obj = cJSON_GetObjectItem(obj, "UDC1_LUT");
	cJSON *global_disable_obj = cJSON_GetObjectItem(obj, "globalDisable");

	if (global_disable_obj && global_disable_obj->valueint) {
		if (dev->dbi_bo.dbi_lut_main_bo)
			bo_destroy(dev->dbi_bo.dbi_lut_main_bo);
		if (dev->dbi_bo.dbi_lut_udc0_bo)
			bo_destroy(dev->dbi_bo.dbi_lut_udc0_bo);
		if (dev->dbi_bo.dbi_lut_udc1_bo)
			bo_destroy(dev->dbi_bo.dbi_lut_udc1_bo);
		if (dev->dbi_bo.dbi_collect_bo)
			bo_destroy(dev->dbi_bo.dbi_collect_bo);
		return 0;
	}

	if (panel_width_obj)
		panel_width = panel_width_obj->valueint;
	if (panel_height_obj)
		panel_height = panel_height_obj->valueint;

	/* dbi control */
	if (roi0_ctl_obj) {
		dbi.main_roi0.enable = true;
		fill_rect(roi0_ctl_obj, &dbi.main_roi0.roi_rect);
	}

	if (roi1_ctl_obj) {
		dbi.main_roi1.enable = true;
		fill_rect(roi1_ctl_obj, &dbi.main_roi1.roi_rect);
	}

	if (roi2_ctl_obj) {
		dbi.main_roi2.enable = true;
		fill_rect(roi2_ctl_obj, &dbi.main_roi2.roi_rect);
	}

	if (udc0_ctl_obj) {
		dbi.udc0.enable = true;
		fill_position(udc0_ctl_obj, &dbi.udc0.pos);
	}

	if (udc1_ctl_obj) {
		dbi.udc1.enable = true;
		fill_position(udc1_ctl_obj, &dbi.udc1.pos);
	}

	/* dbi dither */
	if (dither_obj) {
		dbi.dither.enable = true;
		cJSON *index_type_obj = cJSON_GetObjectItem(dither_obj, "index_type");
		cJSON *sw_index_obj = cJSON_GetObjectItem(dither_obj, "sw_index");
		cJSON *frm_mode_obj = cJSON_GetObjectItem(dither_obj, "frm_mode");
		cJSON *table_low_obj = cJSON_GetObjectItem(dither_obj, "table_low");
		cJSON *table_high_obj = cJSON_GetObjectItem(dither_obj, "table_high");

		if (index_type_obj)
			dbi.dither.dither.index_type = index_type_obj->valueint;
		if (sw_index_obj)
			dbi.dither.dither.sw_index = sw_index_obj->valueint;
		/* the frame mode default set VS_DTH_FRM_10 */
		dbi.dither.dither.frm_mode = VS_DTH_FRM_10;
		if (frm_mode_obj)
			dbi.dither.dither.frm_mode = frm_mode_obj->valueint;
		if (table_low_obj)
			get_uint32_list(table_low_obj, dbi.dither.dither.table_low);
		if (table_high_obj)
			get_uint32_list(table_high_obj, dbi.dither.dither.table_high);
	}

	//main roi lut need (panel_width+39)/40*32*(panel_height+3)/4 Byte,udc lut need (200+19)/20*32*(200+19)/20 Byte
	if (main_lut_obj) {
		aligned_width = (panel_width + 3) / 4;
		aligned_stride = (panel_width + 39) / 40 * 32;
		aligned_height = (panel_height + 3) / 4;

		if (dev->dbi_bo.dbi_lut_main_bo &&
		    ((aligned_stride != dev->dbi_bo.dbi_lut_main_bo->width) ||
		     (aligned_height != dev->dbi_bo.dbi_lut_main_bo->height)))
			bo_destroy(dev->dbi_bo.dbi_lut_main_bo);
		if (!dev->dbi_bo.dbi_lut_main_bo)
			dev->dbi_bo.dbi_lut_main_bo =
				bo_create_dumb(dev->fd, aligned_stride, aligned_height, 8);
		dbi.main_lut_bo.fd = dev->fd;
		dbi.main_lut_bo.handle = dev->dbi_bo.dbi_lut_main_bo->handle;

		fill_dbi_lut_bo(dev->dbi_bo.dbi_lut_main_bo, main_lut_obj, aligned_width,
				aligned_height, aligned_stride);
	}
	if (udc0_lut_obj) {
		aligned_width = (VS_DBI_UDC_WIDHT + 1) / 2;
		aligned_stride = (VS_DBI_UDC_WIDHT + 19) / 20 * 32;
		aligned_height = (VS_DBI_UDC_HEIGHT + 1) / 2;
		if (!dev->dbi_bo.dbi_lut_udc0_bo)
			dev->dbi_bo.dbi_lut_udc0_bo =
				bo_create_dumb(dev->fd, aligned_stride, aligned_height, 8);
		dbi.udc0_lut_bo.fd = dev->fd;
		dbi.udc0_lut_bo.handle = dev->dbi_bo.dbi_lut_udc0_bo->handle;

		fill_dbi_lut_bo(dev->dbi_bo.dbi_lut_udc0_bo, udc0_lut_obj, aligned_width,
				aligned_height, aligned_stride);
	}
	if (udc1_lut_obj) {
		aligned_width = (VS_DBI_UDC_WIDHT + 1) / 2;
		aligned_stride = (VS_DBI_UDC_WIDHT + 19) / 20 * 32;
		aligned_height = (VS_DBI_UDC_HEIGHT + 1) / 2;
		if (!dev->dbi_bo.dbi_lut_udc1_bo)
			dev->dbi_bo.dbi_lut_udc1_bo =
				bo_create_dumb(dev->fd, aligned_stride, aligned_height, 8);
		dbi.udc1_lut_bo.fd = dev->fd;
		dbi.udc1_lut_bo.handle = dev->dbi_bo.dbi_lut_udc1_bo->handle;

		fill_dbi_lut_bo(dev->dbi_bo.dbi_lut_udc1_bo, udc1_lut_obj, aligned_width,
				aligned_height, aligned_stride);
	}

	//collect mem need (panel_width+39)/40*32*(panel_height+3)/4 Byte
	aligned_stride = (panel_width + 39) / 40 * 32;
	aligned_height = (panel_height + 3) / 4;
	if (dev->dbi_bo.dbi_collect_bo && ((aligned_stride != dev->dbi_bo.dbi_collect_bo->width) ||
					   (aligned_height != dev->dbi_bo.dbi_collect_bo->height)))

		bo_destroy(dev->dbi_bo.dbi_collect_bo);
	if (!dev->dbi_bo.dbi_collect_bo)
		dev->dbi_bo.dbi_collect_bo =
			bo_create_dumb(dev->fd, aligned_stride, aligned_height, 8);
	dbi.collect_bo.fd = dev->fd;
	dbi.collect_bo.handle = dev->dbi_bo.dbi_collect_bo->handle;

	drmModeCreatePropertyBlob(dev->fd, &dbi, sizeof(struct drm_vs_dbi), &blob_id);
	return blob_id;
}

uint64_t map_dbi_correction_coef(cJSON *obj, struct device *dev)
{
	uint32_t blob_id = 0;
	struct drm_vs_dbi_correction_coefs coefs = { 0 };
	cJSON *main_coef_obj = cJSON_GetObjectItem(obj, "MAIN");
	cJSON *udc0_coef_obj = cJSON_GetObjectItem(obj, "UDC0");
	cJSON *udc1_coef_obj = cJSON_GetObjectItem(obj, "UDC1");

	if (main_coef_obj)
		fill_dbi_correction_coef(&coefs.main_roi_coef, main_coef_obj);

	if (udc0_coef_obj)
		fill_dbi_correction_coef(&coefs.udc0_coef, udc0_coef_obj);

	if (udc1_coef_obj)
		fill_dbi_correction_coef(&coefs.udc1_coef, udc1_coef_obj);

	drmModeCreatePropertyBlob(dev->fd, &coefs, sizeof(struct drm_vs_dbi_correction_coefs),
				  &blob_id);
	return blob_id;
}
