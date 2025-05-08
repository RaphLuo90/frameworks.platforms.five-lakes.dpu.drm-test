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

#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xf86drm.h"
#include "xf86drmMode.h"
#include "xf86drm.h"
#include "drm/drm_mode.h"

#include "type.h"
#include "util/format.h"
#include "util/log.h"
#include "drmtest_helper.h"
#include "util/error.h"
#include "util/common.h"
#include "buffers.h"
#include "viv_dc_functions.h"

static int device_fd;

static inline cJSON *__get_obj(cJSON *obj, const char *name)
{
	cJSON *item = cJSON_GetObjectItem(obj, name);
	if (NULL == item) {
		printf("Get json obj %s fail\n", name);
		return NULL;
	}
	return item;
}

dtest_status set_property(struct device *dev, struct property_arg *p)
{
	drmModeObjectProperties *props = NULL;
	drmModePropertyRes **props_info = NULL;
	const char *obj_type;
	int ret;
	int i;

	p->obj_type = 0;
	p->prop_id = 0;

#define find_object(_res, __res, type, Type)                                \
	do {                                                                \
		for (i = 0; i < (int)(_res)->__res->count_##type##s; ++i) { \
			struct type *obj = &(_res)->type##s[i];             \
			if (obj->type->type##_id != p->obj_id)              \
				continue;                                   \
			p->obj_type = DRM_MODE_OBJECT_##Type;               \
			obj_type = #Type;                                   \
			props = obj->props;                                 \
			props_info = obj->props_info;                       \
		}                                                           \
	} while (0)

	find_object(dev->resources, res, crtc, CRTC);
	if (p->obj_type == 0)
		find_object(dev->resources, res, connector, CONNECTOR);
	if (p->obj_type == 0)
		find_object(dev->resources, plane_res, plane, PLANE);
	if (p->obj_type == 0) {
		fprintf(stderr, "Object %i not found, can't set property\n", p->obj_id);
		return DTEST_FAILURE;
	}

	if (!props) {
		fprintf(stderr, "%s %i has no properties\n", obj_type, p->obj_id);
		return false;
	}

	for (i = 0; i < (int)props->count_props; ++i) {
		if (!props_info[i])
			continue;
		if (strcmp(props_info[i]->name, p->name) == 0)
			break;
	}

	if (i == (int)props->count_props) {
		if (!p->optional)
			fprintf(stderr, "%s %i has no %s property\n", obj_type, p->obj_id, p->name);
		return DTEST_NOT_SUPPORT;
	}

	p->prop_id = props->props[i];

	ret = drmModeAtomicAddProperty(dev->req, p->obj_id, p->prop_id, p->value);

	if (ret < 0)
		fprintf(stderr, "failed to set %s %i property %s to %" PRIu64 ": %s\n", obj_type,
			p->obj_id, p->name, p->value, strerror(errno));

	return DTEST_SUCCESS;
}

static dtest_status __remap_plane(const struct device *dev, uint32_t *pid)
{
	int i;
	int id = (int)*pid;
	int org_id = id;
	int count = dev->resources->plane_res->count_planes;
	const struct plane *_obj = NULL;

	if (id >= count) {
		DTESTLOGE("Unsupport plane id %d, max is %d", id, count);
		return DTEST_NOT_SUPPORT;
	}

	for (i = 0; i < count; i++) {
		_obj = &dev->resources->planes[i];
		if (!_obj) {
			continue;
		}
		id--;
		if (id < 0) {
			*pid = _obj->plane->plane_id;
			DTESTLOGI("Remap plane id %u to %u", org_id, *pid);
			break;
		}
	}

	if (id >= 0) {
		DTESTLOGE("Unsupport plane id %d, max is %d", id, org_id - id);
		return DTEST_NOT_SUPPORT;
	}
	return DTEST_SUCCESS;
}

dtest_status __remap_connector(const struct device *dev, uint32_t *pid)
{
	int i;
	int id = (int)*pid;
	int org_id = id;
	int count = dev->resources->res->count_connectors;
	const struct connector *_obj = NULL;

	if (id >= count) {
		DTESTLOGE("Invalid connector id %d, max is %d", id, count);
		return DTEST_FAILURE;
	}

	for (i = 0; i < count; i++) {
		_obj = &dev->resources->connectors[i];
		if (!_obj) {
			continue;
		}
		id--;
		if (id < 0) {
			*pid = _obj->connector->connector_id;
			DTESTLOGI("Remap connetor id %u to %u", org_id, *pid);
			break;
		}
	}

	if (id >= 0) {
		DTESTLOGE("Invalid connector id %d, max is %d", id, org_id - id);
		return DTEST_FAILURE;
	}
	return DTEST_SUCCESS;
}

static dtest_status __remap_crtc(const struct device *dev, uint32_t *pid)
{
	int i;
	int id = (int)*pid;
	int org_id = id;
	int count = dev->resources->res->count_crtcs;
	const struct crtc *_obj = NULL;

	if (id >= count) {
		DTESTLOGE("Invalid crtc id %d, max is %d", id, count);
		return DTEST_FAILURE;
	}

	for (i = 0; i < count; i++) {
		_obj = &dev->resources->crtcs[i];
		if (!_obj) {
			continue;
		}
		id--;
		if (id < 0) {
			*pid = _obj->crtc->crtc_id;
			DTESTLOGI("Remap crtc id %u to %u", org_id, *pid);
			break;
		}
	}

	if (id >= 0) {
		DTESTLOGE("Invalid crtc id %d, max is %d", id, org_id - id);
		return DTEST_FAILURE;
	}
	return DTEST_SUCCESS;
}

dtest_status remap_plane_obj_id(const struct device *dev, struct plane_arg *args, uint32_t count)
{
	uint32_t i;
	dtest_status status = DTEST_SUCCESS;
	for (i = 0; i < count; i++) {
		status = __remap_plane(dev, &args[i].plane_id);
		DTEST_CHECK_STATUS(status, final);
		status = __remap_crtc(dev, &args[i].crtc_id);
		DTEST_CHECK_STATUS(status, final);
	}

final:
	return status;
}

dtest_status remap_cursor_obj_id(const struct device *dev, struct cursor_arg *args, uint32_t count)
{
	uint32_t i;
	dtest_status status = DTEST_SUCCESS;

	for (i = 0; i < count; i++) {
		/* TODO: remap cursor_id if needed in the future */
		/*  status = __remap_cursor(dev, &args[i].cursor_id);
		DTEST_CHECK_STATUS(status, final);
	*/
		status = __remap_crtc(dev, &args[i].crtc_id);
		DTEST_CHECK_STATUS(status, final);
	}

final:
	return status;
}

dtest_status remap_pipe_obj_id(const struct device *dev, struct pipe_arg *args, uint32_t count)
{
	uint32_t i, j;
	dtest_status status = DTEST_SUCCESS;
	for (i = 0; i < count; i++) {
		for (j = 0; j < args->num_cons; j++) {
			status = __remap_connector(dev, &args[i].con_ids[j]);
			DTEST_CHECK_STATUS(status, final);
		}
		status = __remap_crtc(dev, &args[i].crtc_id);
		DTEST_CHECK_STATUS(status, final);
	}

final:
	return status;
}

dtest_status remap_resource_obj_id(const struct device *dev, struct resource_arg *args,
				   uint32_t count)
{
	uint32_t i;
	dtest_status status = DTEST_SUCCESS;
	for (i = 0; i < count; i++) {
		status = __remap_plane(dev, &args[i].plane_id);
		if (DTEST_SUCCESS != status) {
			break;
		}
	}
	return status;
}

dtest_status remap_property_obj_id(const struct device *dev, struct property_arg *prop_args,
				   uint32_t prop_count)
{
	uint32_t i = 0;
	dtest_status status = DTEST_SUCCESS;
	for (i = 0; i < prop_count; i++) {
		prop_args[i].obj_id = prop_args[i].obj_id_ori;
		switch (prop_args[i].obj_type) {
		case DRM_MODE_OBJECT_CONNECTOR:
			status = __remap_connector(dev, &prop_args[i].obj_id);
			break;
		case DRM_MODE_OBJECT_PLANE:
			status = __remap_plane(dev, &prop_args[i].obj_id);
			break;
		case DRM_MODE_OBJECT_CRTC:
			status = __remap_crtc(dev, &prop_args[i].obj_id);
			break;
		}
	}
	return status;
}

dtest_status dtest_remap_obj_id(device_t *dev, frame_t *frame)
{
	dtest_status status = DTEST_SUCCESS;
	dtest_pipe_t *dtest_pipes = &frame->pipe;
	dtest_plane_t *dtest_planes = &frame->plane;
	dtest_cursor_t *dtest_cursors = &frame->cursor;
	dtest_property_t *dtest_properties = &frame->property;

	status = remap_plane_obj_id(dev, dtest_planes->planes, dtest_planes->num);
	DTEST_CHECK_STATUS(status, final);

	status = remap_cursor_obj_id(dev, dtest_cursors->cursors, dtest_cursors->num);
	DTEST_CHECK_STATUS(status, final);

	status = remap_pipe_obj_id(dev, dtest_pipes->pipes, dtest_pipes->num);
	DTEST_CHECK_STATUS(status, final);

	status = remap_property_obj_id(dev, dtest_properties->properties, dtest_properties->num);
	DTEST_CHECK_STATUS(status, final);

final:
	return status;
}

cJSON *dtest_load_json(const char *path)
{
	FILE *fp = fopen(path, "r");
	char *buf = NULL;
	size_t file_size = 0, size = 0;
	cJSON *root = NULL;
	if (NULL == fp) {
		DTESTLOGE("open %s fail\n", path);
		return NULL;
	}
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	buf = (char *)malloc(file_size + 1);
	if (!buf) {
		DTESTLOGE("Memory allocation failed.\n");
		fclose(fp);
		return NULL;
	}

	fseek(fp, 0, SEEK_SET);
	size = fread(buf, 1, file_size, fp);
	if (size != file_size) {
		DTESTLOGE("Read %ld size from %s, but file size is %ld\n", size, path, file_size);
		if (buf) {
			free(buf);
			buf = NULL;
		}
		fclose(fp);
		return NULL;
	}
	buf[file_size] = '\0';

	root = cJSON_Parse(buf);
	if (NULL == root) {
		DTESTLOGE("cJSON_Parse fail\n");
	}

	if (buf) {
		free(buf);
		buf = NULL;
	}
	fclose(fp);
	return root;
}

int dtest_get_obj_int(cJSON *obj, const char *name)
{
	cJSON *o = __get_obj(obj, name);
	if (!o)
		return 0;
	return o->valueint;
}

double dtest_get_obj_double(cJSON *obj, const char *name)
{
	cJSON *o = __get_obj(obj, name);
	if (!o)
		return 0.0;
	return o->valuedouble;
}

char *dtest_get_obj_string(cJSON *obj, const char *name)
{
	cJSON *o = __get_obj(obj, name);
	if (!o)
		return "";
	return o->valuestring;
}

bool dtest_get_obj_vrr_enable(cJSON *obj, const char *name)
{
	bool vrr_enable = false;
	cJSON *obj_vrr_enable = cJSON_GetObjectItem(obj, "vrrEnable");

	if (obj_vrr_enable)
		vrr_enable = obj_vrr_enable->valueint;

	return vrr_enable;
}

int dtest_get_obj_vrr_refresh(cJSON *obj, const char *name)
{
	int vrr_refresh = 60;
	cJSON *obj_vrr_refresh = cJSON_GetObjectItem(obj, "vrrRefresh");

	if (obj_vrr_refresh)
		vrr_refresh = obj_vrr_refresh->valueint;

	return vrr_refresh;
}

uint8_t dtest_get_obj_dec_type(cJSON *obj, const char *name)
{
	uint8_t dec_type = DRM_FORMAT_MOD_VS_TYPE_NORMAL;
	cJSON *obj_dec_type = cJSON_GetObjectItem(obj, "decType");
	if (obj_dec_type) {
		if (!strcmp(obj_dec_type->valuestring, "NONE"))
			dec_type = DRM_FORMAT_MOD_VS_TYPE_NORMAL;
		else if (!strcmp(obj_dec_type->valuestring, "DEC"))
			dec_type = DRM_FORMAT_MOD_VS_TYPE_COMPRESSED;
		else if (!strcmp(obj_dec_type->valuestring, "PVRIC"))
			dec_type = DRM_FORMAT_MOD_VS_TYPE_PVRIC;
		else if (!strcmp(obj_dec_type->valuestring, "DECNano"))
			dec_type = DRM_FORMAT_MOD_VS_TYPE_DECNANO;
		else if (!strcmp(obj_dec_type->valuestring, "ETC2"))
			dec_type = DRM_FORMAT_MOD_VS_TYPE_ETC2;
		else if (!strcmp(obj_dec_type->valuestring, "DEC400A"))
			dec_type = DRM_FORMAT_MOD_VS_TYPE_DEC400A;
	}

	return dec_type;
}

uint8_t dtest_get_obj_normal_tilemode(cJSON *obj, const char *name)
{
	uint8_t tile_mode = DRM_FORMAT_MOD_VS_LINEAR;
	cJSON *obj_tile_mode = cJSON_GetObjectItem(obj, "tileMode");

	if (obj_tile_mode) {
		if (!strcmp(obj_tile_mode->valuestring, "LINEAR"))
			tile_mode = DRM_FORMAT_MOD_VS_LINEAR;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE8X8"))
			tile_mode = DRM_FORMAT_MOD_VS_TILE_8X8;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE16X4"))
			tile_mode = DRM_FORMAT_MOD_VS_TILE_16X4;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE32X2"))
			tile_mode = DRM_FORMAT_MOD_VS_TILE_32X2;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE32X4"))
			tile_mode = DRM_FORMAT_MOD_VS_TILE_32X4;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE32X8"))
			tile_mode = DRM_FORMAT_MOD_VS_TILE_32X8;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE32X8_A"))
			tile_mode = DRM_FORMAT_MOD_VS_TILE_32X8_A;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE16X8"))
			tile_mode = DRM_FORMAT_MOD_VS_TILE_16X8;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE16X16"))
			tile_mode = DRM_FORMAT_MOD_VS_TILE_16X16;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE8X4"))
			tile_mode = DRM_FORMAT_MOD_VS_TILE_8X4;
		else if (!strcmp(obj_tile_mode->valuestring, "SUPER_TILED_XMAJOR"))
			tile_mode = DRM_FORMAT_MOD_VS_SUPER_TILED_XMAJOR;
		else if (!strcmp(obj_tile_mode->valuestring, "SUPER_TILED_YMAJOR"))
			tile_mode = DRM_FORMAT_MOD_VS_SUPER_TILED_YMAJOR;
		else if (!strcmp(obj_tile_mode->valuestring, "SUPER_TILED_XMAJOR_8X4"))
			tile_mode = DRM_FORMAT_MOD_VS_SUPER_TILED_XMAJOR_8X4;
		else if (!strcmp(obj_tile_mode->valuestring, "SUPER_TILED_YMAJOR_4X8"))
			tile_mode = DRM_FORMAT_MOD_VS_SUPER_TILED_YMAJOR_4X8;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE4X4"))
			tile_mode = DRM_FORMAT_MOD_VS_TILE_MODE4X4;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE_8X8_UNIT2X2"))
			tile_mode = DRM_FORMAT_MOD_VS_TILE_8X8_UNIT2X2;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE_8X4_UNIT2X2"))
			tile_mode = DRM_FORMAT_MOD_VS_TILE_8X4_UNIT2X2;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE_8X8_SUPERTILE_X"))
			tile_mode = DRM_FORMAT_MOD_VS_TILE_8X8_SUPERTILE_X;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE_32X8_YUVSP8X8"))
			tile_mode = DRM_FORMAT_MOD_VS_TILE_32X8_YUVSP8X8;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE_16X8_YUVSP8X8"))
			tile_mode = DRM_FORMAT_MOD_VS_TILE_16X8_YUVSP8X8;
	}

	return tile_mode;
}

uint8_t dtest_get_obj_pvric_tilemode(cJSON *obj, const char *name)
{
	uint8_t tile_mode = DRM_FORMAT_MOD_VS_DEC_TILE_16X4;
	cJSON *obj_tile_mode = cJSON_GetObjectItem(obj, "tileMode");

	if (obj_tile_mode) {
		if (!strcmp(obj_tile_mode->valuestring, "TILE8X8"))
			tile_mode = DRM_FORMAT_MOD_VS_DEC_TILE_8X8;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE16X4"))
			tile_mode = DRM_FORMAT_MOD_VS_DEC_TILE_16X4;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE32X2"))
			tile_mode = DRM_FORMAT_MOD_VS_DEC_TILE_32X2;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE32X4"))
			tile_mode = DRM_FORMAT_MOD_VS_DEC_TILE_32X4;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE32X8"))
			tile_mode = DRM_FORMAT_MOD_VS_DEC_TILE_32X8;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE16X8"))
			tile_mode = DRM_FORMAT_MOD_VS_DEC_TILE_16X8;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE16X16"))
			tile_mode = DRM_FORMAT_MOD_VS_DEC_TILE_16X16;
	}

	return tile_mode;
}

uint8_t dtest_get_obj_dec_tilemode(cJSON *obj, const char *name)
{
	cJSON *obj_tile_mode = cJSON_GetObjectItem(obj, "tileMode");
	const char *tile_mode = NULL;
	if (obj_tile_mode)
		tile_mode = obj_tile_mode->valuestring;
	else {
		DTESTLOGE("not found tileMode config.\n");
		return -1;
	}

#define TILECMP(tilename)                                        \
	do {                                                     \
		if (!strcmp(#tilename, tile_mode))               \
			return DRM_FORMAT_MOD_VS_DEC_##tilename; \
	} while (0)

	TILECMP(TILE_8X8_XMAJOR);
	TILECMP(TILE_8X8_YMAJOR);
	TILECMP(TILE_16X4);
	TILECMP(TILE_8X4);
	TILECMP(TILE_4X8);
	TILECMP(RASTER_16X4);
	TILECMP(TILE_64X4);
	TILECMP(TILE_32X4);
	TILECMP(RASTER_256X1);
	TILECMP(RASTER_128X1);
	TILECMP(RASTER_64X4);
	TILECMP(RASTER_256X2);
	TILECMP(RASTER_128X2);
	TILECMP(RASTER_128X4);
	TILECMP(RASTER_64X1);
	TILECMP(TILE_16X8);
	TILECMP(TILE_8X16);
	TILECMP(RASTER_512X1);
	TILECMP(RASTER_32X4);
	TILECMP(RASTER_64X2);
	TILECMP(RASTER_32X2);
	TILECMP(RASTER_32X1);
	TILECMP(RASTER_16X1);
	TILECMP(TILE_128X4);
	TILECMP(TILE_256X4);
	TILECMP(TILE_512X4);
	TILECMP(TILE_16X16);
	TILECMP(TILE_32X16);
	TILECMP(TILE_64X16);
	TILECMP(TILE_128X8);
	TILECMP(TILE_8X4_S);
	TILECMP(TILE_16X4_S);
	TILECMP(TILE_32X4_S);
	TILECMP(TILE_16X4_LSB);
	TILECMP(TILE_32X4_LSB);
	TILECMP(TILE_32X8);
	TILECMP(TILE_8X8);
	TILECMP(TILE_32X2);
	TILECMP(TILE_8X8_UNIT2X2);
	TILECMP(TILE_8X4_UNIT2X2);
	TILECMP(TILE_8X8_SUPERTILE_X);
	TILECMP(TILE_32X8_YUVSP8X8);
	TILECMP(TILE_16X8_YUVSP8X8);
	TILECMP(TILE_32X8_A);

	DTESTLOGE("Can't find tile mode %s\n", tile_mode);
	return -1;
}

bool dtest_get_obj_stream(cJSON *obj, const char *name)
{
	bool stream = false;
	cJSON *obj_stream = cJSON_GetObjectItem(obj, "stream");

	if (obj_stream)
		stream = !!obj_stream->valueint;

	return stream;
}

bool dtest_get_obj_global_disable(cJSON *obj, const char *name)
{
	bool global_disable = false;
	cJSON *obj_global_disable = cJSON_GetObjectItem(obj, "globalDisable");

	if (obj_global_disable)
		global_disable = !!obj_global_disable->valueint;

	return global_disable;
}

bool dtest_get_bool_obj_by_name(cJSON *obj, const char *name)
{
	bool ret_bool = false;
	cJSON *obj_bool = cJSON_GetObjectItem(obj, name);

	if (obj_bool)
		ret_bool = !!obj_bool->valueint;

	return ret_bool;
}

uint8_t dtest_get_obj_decnano_tilemode(cJSON *obj, const char *name)
{
	uint8_t tile_mode = DRM_FORMAT_MOD_VS_DEC_LINEAR;
	cJSON *obj_tile_mode = cJSON_GetObjectItem(obj, "tileMode");

	if (obj_tile_mode) {
		if (!strcmp(obj_tile_mode->valuestring, "LINEAR"))
			tile_mode = DRM_FORMAT_MOD_VS_DEC_LINEAR;
		else if (!strcmp(obj_tile_mode->valuestring, "TILE4X4"))
			tile_mode = DRM_FORMAT_MOD_VS_DEC_TILE_4X4;
	}

	return tile_mode;
}

uint8_t dtest_get_obj_etc2_tilemode(cJSON *obj, const char *name)
{
	uint8_t tile_mode = DRM_FORMAT_MOD_VS_DEC_TILE_4X4;
	cJSON *obj_tile_mode = cJSON_GetObjectItem(obj, "tileMode");

	if (obj_tile_mode) {
		if (!strcmp(obj_tile_mode->valuestring, "TILE4X4"))
			tile_mode = DRM_FORMAT_MOD_VS_DEC_TILE_4X4;
	}

	return tile_mode;
}

void dtest_get_obj_normal_modifier(cJSON *obj, uint64_t *mod)
{
	uint8_t tile_mode = dtest_get_obj_normal_tilemode(obj, "tileMode");
	*mod = fourcc_mod_vs_norm_code(tile_mode);
}

void dtest_get_obj_pvric_modifier(cJSON *obj, uint64_t *mod)
{
	bool lossy = false;
	uint8_t tile_mode = dtest_get_obj_pvric_tilemode(obj, "tileMode");

	cJSON *lossy_obj = cJSON_GetObjectItem(obj, "lossy");
	if (lossy_obj) {
		lossy = !!(lossy_obj->valueint);
	}

	*mod = lossy ? fourcc_mod_vs_pvric_code(tile_mode, DRM_FORMAT_MOD_VS_DEC_LOSSY) :
			     fourcc_mod_vs_pvric_code(tile_mode, DRM_FORMAT_MOD_VS_DEC_LOSSLESS);
}

void dtest_get_obj_dec_modifier(cJSON *obj, uint64_t *mod)
{
	bool align64 = false;
	uint8_t tile_mode = dtest_get_obj_dec_tilemode(obj, "tileMode");

	cJSON *align64_obj = cJSON_GetObjectItem(obj, "align64");
	if (align64_obj) {
		align64 = !!(align64_obj->valueint);
	}

	*mod = align64 ? fourcc_mod_vs_dec_code(tile_mode, DRM_FORMAT_MOD_VS_DEC_ALIGN_64) :
			       fourcc_mod_vs_dec_code(tile_mode, DRM_FORMAT_MOD_VS_DEC_ALIGN_32);
}

void dtest_get_obj_decnano_modifier(cJSON *obj, uint64_t *mod)
{
	uint32_t decnano_mode = DRM_FORMAT_MOD_VS_DECNANO_NON_SAMPLE;
	uint8_t tile_mode = dtest_get_obj_decnano_tilemode(obj, "tileMode");

	cJSON *decnano_mode_obj = cJSON_GetObjectItem(obj, "decNanoMode");
	if (decnano_mode_obj) {
		if (!strcmp(decnano_mode_obj->valuestring, "NON_SAMPLE"))
			decnano_mode = DRM_FORMAT_MOD_VS_DECNANO_NON_SAMPLE;
		else if (!strcmp(decnano_mode_obj->valuestring, "H_SAMPLE"))
			decnano_mode = DRM_FORMAT_MOD_VS_DECNANO_H_SAMPLE;
		else if (!strcmp(decnano_mode_obj->valuestring, "HV_SAMPLE"))
			decnano_mode = DRM_FORMAT_MOD_VS_DECNANO_HV_SAMPLE;
	}

	*mod = fourcc_mod_vs_decnano_code(tile_mode, decnano_mode);
}

void dtest_get_obj_etc2_modifier(cJSON *obj, uint64_t *mod)
{
	uint8_t tile_mode = dtest_get_obj_etc2_tilemode(obj, "tileMode");
	*mod = fourcc_mod_vs_etc2_code(tile_mode);
}

void dtest_get_obj_dec400a_modifier(cJSON *obj, uint64_t *mod)
{
	uint32_t yuv_transform = DRM_FORMAT_MOD_VS_DEC_YUV_TRANSFORM_DISABLE;
	uint8_t tile_mode = dtest_get_obj_dec_tilemode(obj, "tileMode");

	cJSON *yuv_transform_obj = cJSON_GetObjectItem(obj, "yuv_transform");

	if (yuv_transform_obj) {
		if (yuv_transform_obj->valueint == 1) {
			yuv_transform = DRM_FORMAT_MOD_VS_DEC_YUV_TRANSFORM_ENABLE;
		}
	}

	*mod = fourcc_mod_vs_dec400a_code(tile_mode, yuv_transform);
}

uint64_t dtest_get_obj_dec_fb_modifier(cJSON *obj, const char *name)
{
	uint8_t dec_type = DRM_FORMAT_MOD_VS_TYPE_NORMAL;
	uint8_t tile_mode = DRM_FORMAT_MOD_VS_LINEAR;
	uint64_t mod = fourcc_mod_vs_code(dec_type, tile_mode); /* DEC/PVRIC NONE, tile LINEAR */
	cJSON *custom_format_obj = cJSON_GetObjectItem(obj, "customFormat");

	dec_type = dtest_get_obj_dec_type(obj, "decType");

	switch (dec_type) {
	case DRM_FORMAT_MOD_VS_TYPE_NORMAL:
		dtest_get_obj_normal_modifier(obj, &mod);
		break;
	case DRM_FORMAT_MOD_VS_TYPE_COMPRESSED:
		dtest_get_obj_dec_modifier(obj, &mod);
		break;
	case DRM_FORMAT_MOD_VS_TYPE_PVRIC:
		dtest_get_obj_pvric_modifier(obj, &mod);
		break;
	case DRM_FORMAT_MOD_VS_TYPE_DECNANO:
		dtest_get_obj_decnano_modifier(obj, &mod);
		break;
	case DRM_FORMAT_MOD_VS_TYPE_ETC2:
		dtest_get_obj_etc2_modifier(obj, &mod);
		break;
	case DRM_FORMAT_MOD_VS_TYPE_DEC400A:
		dtest_get_obj_dec400a_modifier(obj, &mod);
		break;
	default:
		break;
	}

	if (custom_format_obj) {
		if (custom_format_obj->valueint == 1) {
			mod |= (uint64_t)DRM_FORMAT_MOD_VS_CUSTOM_FORMAT_ENABLE;
		}
	}

	return mod;
}

uint16_t dtest_get_obj_std_blend_mode(cJSON *obj, const char *name)
{
	int std_blend_mode = BLEND_PIXEL_NONE;
	cJSON *obj_blend_mode = cJSON_GetObjectItem(obj, "stdBlendMode");

	if (obj_blend_mode) {
		if (!strcmp(obj_blend_mode->valuestring, "PIXEL_NONE"))
			std_blend_mode = BLEND_PIXEL_NONE;
		else if (!strcmp(obj_blend_mode->valuestring, "PREMULTI"))
			std_blend_mode = BLEND_PREMULTI;
		else if (!strcmp(obj_blend_mode->valuestring, "COVERAGE"))
			std_blend_mode = BLEND_COVERAGE;
	}

	return std_blend_mode;
}

uint32_t dtest_get_obj_std_blend_alpha(cJSON *obj, const char *name)
{
	uint32_t alpha_value = BLEND_ALPHA_OPAQUE;
	cJSON *obj_alpha_value = cJSON_GetObjectItem(obj, "stdBlendAlpha");

	if (obj_alpha_value)
		alpha_value = obj_alpha_value->valueint;

	return alpha_value;
}

uint32_t dtest_get_obj_zpos(cJSON *obj, const char *name)
{
	uint32_t zpos = atoi(obj->string) % 8; /* the zpos default equa the plane ID % 8 */
	cJSON *obj_zpos = cJSON_GetObjectItem(obj, "zpos");

	if (obj_zpos)
		zpos = obj_zpos->valueint;

	return zpos;
}

uint8_t dtest_get_obj_color_encoding(cJSON *obj, const char *name)
{
	uint8_t color_encoding = COLOR_YCBCR_BT2020;
	cJSON *obj_color_encoding = cJSON_GetObjectItem(obj, "yuvColorEncoding");

	if (obj_color_encoding) {
		if (!strcmp(obj_color_encoding->valuestring, "BT601"))
			color_encoding = COLOR_YCBCR_BT601;
		else if (!strcmp(obj_color_encoding->valuestring, "BT709"))
			color_encoding = COLOR_YCBCR_BT709;
		else if (!strcmp(obj_color_encoding->valuestring, "BT2020"))
			color_encoding = COLOR_YCBCR_BT2020;
	}

	return color_encoding;
}

uint8_t dtest_get_obj_color_range(cJSON *obj, const char *name)
{
	uint8_t color_range = COLOR_YCBCR_LIMITED_RANGE;
	cJSON *obj_color_range = cJSON_GetObjectItem(obj, "yuvColorRange");

	if (obj_color_range) {
		if (!strcmp(obj_color_range->valuestring, "LIMITED"))
			color_range = COLOR_YCBCR_LIMITED_RANGE;
		else if (!strcmp(obj_color_range->valuestring, "FULL"))
			color_range = COLOR_YCBCR_FULL_RANGE;
	}

	return color_range;
}

size_t dtest_get_obj_str(cJSON *obj, const char *name, char *buf)
{
	cJSON *obj_str = __get_obj(obj, name);
	if (!obj_str)
		return 0;
	size_t len = strlen(obj_str->valuestring);
	if (len > 0 && buf && len < (RESOURCE_LEN + 1)) {
		strncpy(buf, obj_str->valuestring, len);
	}
	return len;
}

uint32_t dtest_get_obj_format(cJSON *obj, char *format_str)
{
	uint32_t fourcc;
	size_t format_len = dtest_get_obj_str(obj, "format", format_str);
	if (format_len <= 0) {
		// Don't find the format in case, auto fill 'XR24' to format
		DTESTLOGW("Don't find the format in case, auto fill 'XR24' to format");
		strcpy(format_str, "XR24");
	}
	if (format_len > 15) {
		printf("Invalid format, format_len=%ld\n", format_len);
		return -1;
	}
	fourcc = util_format_fourcc(format_str);
	if (fourcc == 0) {
		fprintf(stderr, "unknown format %s\n", format_str);
		return -1;
	}
	return fourcc;
}

uint32_t dtest_get_obj_rotation(cJSON *obj, char *name)
{
	uint32_t rotation_mode = DRM_MODE_ROTATE_0;
	cJSON *obj_rotation_mode = cJSON_GetObjectItem(obj, "rotation");

	if (obj_rotation_mode) {
		if (!strcmp(obj_rotation_mode->valuestring, "0"))
			rotation_mode = DRM_MODE_ROTATE_0;
		else if (!strcmp(obj_rotation_mode->valuestring, "90"))
			rotation_mode = DRM_MODE_ROTATE_90;
		else if (!strcmp(obj_rotation_mode->valuestring, "180"))
			rotation_mode = DRM_MODE_ROTATE_180;
		else if (!strcmp(obj_rotation_mode->valuestring, "270"))
			rotation_mode = DRM_MODE_ROTATE_270;
		else if (!strcmp(obj_rotation_mode->valuestring, "FLIPX"))
			rotation_mode = DRM_MODE_REFLECT_X | DRM_MODE_ROTATE_0;
		else if (!strcmp(obj_rotation_mode->valuestring, "FLIPY"))
			rotation_mode = DRM_MODE_REFLECT_Y | DRM_MODE_ROTATE_0;
		else if (!strcmp(obj_rotation_mode->valuestring, "FLIPXY"))
			rotation_mode = DRM_MODE_REFLECT_X | DRM_MODE_REFLECT_Y | DRM_MODE_ROTATE_0;
		else if (!strcmp(obj_rotation_mode->valuestring, "FLIPX_90"))
			rotation_mode = DRM_MODE_REFLECT_X | DRM_MODE_ROTATE_90;
		else if (!strcmp(obj_rotation_mode->valuestring, "FLIPY_90"))
			rotation_mode = DRM_MODE_REFLECT_Y | DRM_MODE_ROTATE_90;
	}

	return rotation_mode;
}

int dtest_get_feature_cap(int fd, enum drm_vs_feature_cap_type cap_type, int32_t *cap)
{
	struct drm_vs_query_feature_cap feature_cap = { 0 };
	int ret;
	feature_cap.type = cap_type;
	ret = drmIoctl(fd, DRM_IOCTL_VS_GET_FEATURE_CAP, &feature_cap);
	*cap = feature_cap.cap;
	return ret;
}

int dtest_get_plane_valid_rot(device_t *dev, uint32_t id, uint32_t *cap)
{
	struct plane *plane = NULL;
	drmModeObjectProperties *props = NULL;
	drmModePropertyRes **props_info = NULL;
	uint32_t i, j, k;

	if (!dev->resources->plane_res)
		return -1;

	for (i = 0; i < dev->resources->plane_res->count_planes; i++) {
		plane = &dev->resources->planes[i];
		props = plane->props;
		props_info = plane->props_info;

		if (plane->plane->plane_id != id)
			continue;

		if (!props) {
			DTESTLOGE("plane %d: no properties found", id);
			return -1;
		}

		for (j = 0; j < props->count_props; ++j) {
			if (!props_info[j])
				continue;
			if (strcmp(props_info[j]->name, "rotation") == 0) {
				if (!drm_property_type_is(props_info[j], DRM_MODE_PROP_BITMASK)) {
					DTESTLOGE("rotation in plane is not bitmask, need check");
					return -1;
				}
				*cap = 0;
				for (k = 0; k < props_info[j]->count_enums; k++) {
					*cap |= (1 << props_info[j]->enums[k].value);
				}
				return 0;
			}
		}
	}

	DTESTLOGE("no found plane %d", id);
	return -1;
}

int dtest_get_connector_possible_crtcs(device_t *dev, uint32_t id, uint32_t *cap)
{
	struct connector *connector = NULL;
	drmModeEncoder *encoder;
	uint32_t encoder_id = 0;
	uint32_t i, j;

	if (!dev->resources->res)
		return -1;

	for (i = 0; i < dev->resources->res->count_connectors; ++i) {
		connector = &dev->resources->connectors[i];

		if (connector->connector->connector_id != id)
			continue;

		encoder_id = connector->connector->encoder_id;
		if (encoder_id == 0)
			encoder_id = dev->resources->encoders[i].encoder->encoder_id;

		for (j = 0; j < dev->resources->res->count_encoders; j++) {
			encoder = dev->resources->encoders[j].encoder;

			if (encoder->encoder_id != encoder_id)
				continue;

			*cap = encoder->possible_crtcs;
			return 0;
		}
	}

	DTESTLOGE("no found connector %d or it's encoder", id);
	return -1;
}

void **dtest_get_crtc_info(device_t *dev, uint32_t id)
{
	uint32_t blob_id = 0;
	drmModePropertyBlobPtr blob = NULL;

	blob_id = get_crtc_property_default_value(dev, id, "DC_INFO");

	if (!blob_id) {
		DTESTLOGE("failed to get blob id of 'INFO' in crtc %d", id);
		return NULL;
	}

	blob = drmModeGetPropertyBlob(dev->fd, blob_id);

	if (!blob || !blob->data) {
		DTESTLOGE("failed to get blob %d", blob_id);
		drmModeFreePropertyBlob(blob);
		return NULL;
	}

	return &blob->data;
}

void **dtest_get_plane_info(device_t *dev, uint32_t id)
{
	uint32_t blob_id = 0;
	drmModePropertyBlobPtr blob = NULL;

	blob_id = get_plane_property_default_value(dev, id, "INFO");

	if (!blob_id) {
		DTESTLOGE("failed to get blob id of 'INFO' in plane %d", id);
		return NULL;
	}

	blob = drmModeGetPropertyBlob(dev->fd, blob_id);

	if (!blob || !blob->data) {
		DTESTLOGE("failed to get blob %d", blob_id);
		drmModeFreePropertyBlob(blob);
		return NULL;
	}

	return &blob->data;
}

void dtest_free_info_blob(void **info)
{
	drmModePropertyBlobPtr blob = NULL;

	if (!info || !*info)
		return;

	blob = container_of(info, drmModePropertyBlobRes, data);

	if (blob)
		drmModeFreePropertyBlob(blob);
	else
		DTESTLOGE("failed to get blob ptr via data ptr");
}

int dtest_get_hist_info(device_t *dev, frame_t *frame)
{
	int ret = 0;
	int i, j, k;
	bool enable[2][VS_HIST_IDX_COUNT] = { false };
	enum drm_vs_hist_out_mode hist_out_mode[2][VS_HIST_IDX_COUNT] = { VS_HIST_BY_APB };
	struct drm_vs_get_hist_info get_hist_info = { 0 };
	property_t *prop = frame->crtc.__property.properties;
	cJSON *obj = NULL;
	cJSON *hist_enable_obj = NULL, *hist_out_mode_obj = NULL;
	cJSON *idx_obj = NULL;
	uint32_t *hist_arr;

	for (i = 0; i < frame->crtc.__property.num; i++) {
		if (strncmp(prop->name, "HIST", DRM_PROP_NAME_LEN + 1) == 0) {
			obj = prop->json_obj;
			hist_enable_obj = cJSON_GetObjectItem(obj, "hist_enable");
			idx_obj = cJSON_GetObjectItem(obj, "idx");
			if (hist_enable_obj) {
				if (hist_enable_obj->valueint && idx_obj)
					enable[(int)prop->obj_id][idx_obj->valueint] = true;
			}
		}
		if (strncmp(prop->name, "HIST_INFO", DRM_PROP_NAME_LEN + 1) == 0) {
			obj = prop->json_obj;
			hist_out_mode_obj = cJSON_GetObjectItem(obj, "out_mode");
			idx_obj = cJSON_GetObjectItem(obj, "idx");
			if (hist_out_mode_obj) {
				if (hist_out_mode_obj->valueint && idx_obj)
					hist_out_mode[(int)prop->obj_id][idx_obj->valueint] =
						VS_HIST_BY_WDMA;
			}
		}
		prop++;
	}

	enable[1][VS_HIST_IDX_RGB] = false;

	for (i = 0; i < 2; i++) {
		for (j = 0; j < VS_HIST_IDX_COUNT; j++) {
			if (enable[i][j]) {
				if (hist_out_mode[i][j] == VS_HIST_BY_APB) {
					get_hist_info.crtc_id = i;
					get_hist_info.idx = j;
					ret = drmIoctl(dev->fd, DRM_IOCTL_VS_GET_HIST_INFO,
						       &get_hist_info);
					if (ret) {
						DTESTLOGE("get hist info failed");
						bo_destroy(dev->hist_bo);
						dev->hist_bo = NULL;
						return ret;
					}
					fprintf(stdout, "crtc[%d] hist[%d] result:", i, j);
					fprintf(stdout, "VS_HIST_BY_APB\n");
					for (k = 0; k < (j == VS_HIST_IDX_RGB ?
								       (VS_HIST_RESULT_BIN_CNT * 3) :
								       VS_HIST_RESULT_BIN_CNT);
					     k++) {
						if ((k % 16) == 0)
							fprintf(stdout, "\n");
						fprintf(stdout, "%d ",
							j == VS_HIST_IDX_RGB ?
								      get_hist_info.u.rgb_hist.result[k] :
								      get_hist_info.u.hist.result[k]);
					}
					fprintf(stdout, "\n");
				} else if (hist_out_mode[i][j] == VS_HIST_BY_WDMA) {
					if (bo_map(dev->hist_bo, (void **)&hist_arr)) {
						fprintf(stderr, "failed to map buffer: %s\n",
							strerror(-errno));
						bo_destroy(dev->hist_bo);
						dev->hist_bo = NULL;
						return 0;
					}
					fprintf(stdout, "crtc[%d] hist[%d] result: ", i, j);
					fprintf(stdout, "VS_HIST_BY_WDMA\n");
					for (k = 0; k < (j == VS_HIST_IDX_RGB ?
								       (VS_HIST_RESULT_BIN_CNT * 3) :
								       VS_HIST_RESULT_BIN_CNT);
					     k++) {
						if ((k % 16) == 0)
							fprintf(stdout, "\n");

						fprintf(stdout, "%x ", hist_arr[k]);
					}
					fprintf(stdout, "\n");

					bo_unmap(dev->hist_bo);
				}
			}
		}
	}

	return ret;
}

int dtest_get_ops_result(device_t *dev, frame_t *frame)
{
	int i = 0;
	uint32_t mapped_crtc_id = 0;
	property_t *prop = frame->crtc.__property.properties;
	cJSON *cjson_obj = NULL;
	cJSON *cjson_disable_obj = NULL;
	uint64_t ops_blob_id = 0;
	drmModePropertyBlobPtr ops_result_blob = NULL;
	struct drm_vs_ops_result *ops_result = NULL;

	for (i = 0; i < frame->crtc.__property.num; i++) {
		if (strncmp(prop->name, "OPS", DRM_PROP_NAME_LEN + 1) == 0) {
			cjson_obj = prop->json_obj;
			cjson_disable_obj = cJSON_GetObjectItem(cjson_obj, "globalDisable");
			if (cjson_disable_obj) {
				if (cjson_disable_obj->valueint)
					return 0;
			}
			/* get the CRTC id */
			mapped_crtc_id = prop->obj_id;
			if (__remap_crtc(dev, &mapped_crtc_id))
				return 0;
			/* get the OPS result property BLOB ID */
			ops_blob_id =
				get_crtc_property_default_value(dev, mapped_crtc_id, "OPS_RESULT");

			if (!ops_blob_id) {
				printf("dtest_get_opst_result: cannot get he 'OPS_RESULT' property for CRTC%d.\n",
				       mapped_crtc_id);
				return 0;
			}

			/* Get the BLOB data */
			ops_result_blob = drmModeGetPropertyBlob(dev->fd, ops_blob_id);
			if (!ops_result_blob) {
				printf("dtest_get_opst_result: get the 'OPS_RESULT' blob failed.\n");
				return 0;
			}
			if (!ops_result_blob->data) {
				drmModeFreePropertyBlob(ops_result_blob);
				printf("dtest_get_opst_result: get the 'OPS_RESULT' blob data failed.\n");
				return 0;
			}
			ops_result = (struct drm_vs_ops_result *)ops_result_blob->data;
			printf("The CRTC %d, On Pixel Sum Result: red:%llx, green:%llx, blue:%llx.\n",
			       mapped_crtc_id, ops_result->ops_red_sum, ops_result->ops_green_sum,
			       ops_result->ops_blue_sum);
		}
		prop++;
	}

	return 0;
}

int dtest_get_histogram_result(device_t *dev, frame_t *frame)
{
	int i = 0, m = 0;
	uint32_t mapped_crtc_id = 0;
	property_t *prop = frame->crtc.__property.properties;
	cJSON *cjson_obj = NULL;
	cJSON *cjson_disable_obj = NULL;
	cJSON *cjson_out_mode_obj = NULL;
	uint64_t histogram_blob_id = 0;
	drmModePropertyBlobPtr histogram_result_blob = NULL;
	struct drm_vs_histogram_result *histogram_result = NULL;
	uint32_t *hist_arr;

	for (i = 0; i < frame->crtc.__property.num; i++) {
		if (strncmp(prop->name, "HISTOGRAM_", strlen("HISTOGRAM_")) == 0) {
			cjson_obj = prop->json_obj;
			cjson_disable_obj = cJSON_GetObjectItem(cjson_obj, "globalDisable");
			cjson_out_mode_obj = cJSON_GetObjectItem(cjson_obj, "out_mode");
			if (cjson_disable_obj) {
				if (cjson_disable_obj->valueint)
					return 0;
			}

			/* get histogram result by wdma */
			if (cjson_out_mode_obj->valueint == VS_HIST_BY_WDMA) {
				if (strncmp(prop->name, "HISTOGRAM_0", DRM_PROP_NAME_LEN + 1) ==
				    0) {
					if (bo_map(dev->histogram_bo[VS_HIST_IDX_0],
						   (void **)&hist_arr)) {
						fprintf(stderr, "failed to map buffer: %s\n",
							strerror(-errno));
						bo_destroy(dev->histogram_bo[VS_HIST_IDX_0]);
						dev->histogram_bo[VS_HIST_IDX_0] = NULL;
						return 0;
					}
					fprintf(stdout, "crtc[%d] histogram[0] result by wdma:",
						prop->obj_id);
					for (m = 0; m < VS_HIST_RESULT_BIN_CNT; m++) {
						if ((m % 16) == 0)
							fprintf(stdout, "\n");
						fprintf(stdout, "%u ", hist_arr[m]);
					}
					fprintf(stdout, "\n");

					bo_unmap(dev->histogram_bo[VS_HIST_IDX_0]);
				}
				if (strncmp(prop->name, "HISTOGRAM_1", DRM_PROP_NAME_LEN + 1) ==
				    0) {
					if (bo_map(dev->histogram_bo[VS_HIST_IDX_1],
						   (void **)&hist_arr)) {
						fprintf(stderr, "failed to map buffer: %s\n",
							strerror(-errno));
						bo_destroy(dev->histogram_bo[VS_HIST_IDX_1]);
						dev->histogram_bo[VS_HIST_IDX_1] = NULL;
						return 0;
					}
					fprintf(stdout, "crtc[%d] histogram[1] result by wdma:",
						prop->obj_id);
					for (m = 0; m < VS_HIST_RESULT_BIN_CNT; m++) {
						if ((m % 16) == 0)
							fprintf(stdout, "\n");
						fprintf(stdout, "%u ", hist_arr[m]);
					}
					fprintf(stdout, "\n");

					bo_unmap(dev->histogram_bo[VS_HIST_IDX_1]);
				}
				if (strncmp(prop->name, "HISTOGRAM_2", DRM_PROP_NAME_LEN + 1) ==
				    0) {
					if (bo_map(dev->histogram_bo[VS_HIST_IDX_2],
						   (void **)&hist_arr)) {
						fprintf(stderr, "failed to map buffer: %s\n",
							strerror(-errno));
						bo_destroy(dev->histogram_bo[VS_HIST_IDX_2]);
						dev->histogram_bo[VS_HIST_IDX_2] = NULL;
						return 0;
					}
					fprintf(stdout, "crtc[%d] histogram[2] result by wdma:",
						prop->obj_id);
					for (m = 0; m < VS_HIST_RESULT_BIN_CNT; m++) {
						if ((m % 16) == 0)
							fprintf(stdout, "\n");
						fprintf(stdout, "%u ", hist_arr[m]);
					}
					fprintf(stdout, "\n");

					bo_unmap(dev->histogram_bo[VS_HIST_IDX_2]);
				}
				if (strncmp(prop->name, "HISTOGRAM_3", DRM_PROP_NAME_LEN + 1) ==
				    0) {
					if (bo_map(dev->histogram_bo[VS_HIST_IDX_3],
						   (void **)&hist_arr)) {
						fprintf(stderr, "failed to map buffer: %s\n",
							strerror(-errno));
						bo_destroy(dev->histogram_bo[VS_HIST_IDX_3]);
						dev->histogram_bo[VS_HIST_IDX_3] = NULL;
						return 0;
					}
					fprintf(stdout, "crtc[%d] histogram[3] result by wdma:",
						prop->obj_id);
					for (m = 0; m < VS_HIST_RESULT_BIN_CNT; m++) {
						if ((m % 16) == 0)
							fprintf(stdout, "\n");
						fprintf(stdout, "%u ", hist_arr[m]);
					}
					fprintf(stdout, "\n");

					bo_unmap(dev->histogram_bo[VS_HIST_IDX_3]);
				}
				if (strncmp(prop->name, "HISTOGRAM_RGB", DRM_PROP_NAME_LEN + 1) ==
				    0) {
					if (bo_map(dev->histogram_bo[VS_HIST_IDX_RGB],
						   (void **)&hist_arr)) {
						fprintf(stderr, "failed to map buffer: %s\n",
							strerror(-errno));
						bo_destroy(dev->histogram_bo[VS_HIST_IDX_RGB]);
						dev->histogram_bo[VS_HIST_IDX_RGB] = NULL;
						return 0;
					}
					fprintf(stdout, "crtc[%d] histogram[rgb] result by wdma:",
						prop->obj_id);
					for (m = 0; m < VS_HIST_RESULT_BIN_CNT * 3; m++) {
						if ((m % 16) == 0)
							fprintf(stdout, "\n");
						fprintf(stdout, "%u ", hist_arr[m]);
					}
					fprintf(stdout, "\n");

					bo_unmap(dev->histogram_bo[VS_HIST_IDX_RGB]);
				}
				prop++;
				continue;
			}

			/* get histogram result by apb */
			/* get the CRTC id */
			mapped_crtc_id = prop->obj_id;
			if (__remap_crtc(dev, &mapped_crtc_id))
				return 0;
			/* get the histogram result property BLOB ID */
			histogram_blob_id = get_crtc_property_default_value(dev, mapped_crtc_id,
									    "HISTOGRAM_RESULT");
			if (!histogram_blob_id) {
				printf("dtest_get_histogram_result: cannot get he 'HISTOGRAM_RESULT' property for CRTC%d.\n",
				       mapped_crtc_id);
				return 0;
			}

			/* Get the BLOB data */
			histogram_result_blob = drmModeGetPropertyBlob(dev->fd, histogram_blob_id);
			if (!histogram_result_blob) {
				printf("dtest_get_histogram_result: get the 'HISTOGRAM_RESULT' blob failed.\n");
				return 0;
			}
			if (!histogram_result_blob->data) {
				drmModeFreePropertyBlob(histogram_result_blob);
				printf("dtest_get_histogram_result: get the 'HISTOGRAM_RESULT' blob data failed.\n");
				return 0;
			}
			histogram_result =
				(struct drm_vs_histogram_result *)histogram_result_blob->data;

			if (strncmp(prop->name, "HISTOGRAM_0", DRM_PROP_NAME_LEN + 1) == 0) {
				fprintf(stdout,
					"crtc[%d] histogram[0] result by apb:", prop->obj_id);
				for (m = 0; m < VS_HIST_RESULT_BIN_CNT; m++) {
					if ((m % 16) == 0)
						fprintf(stdout, "\n");
					fprintf(stdout, "%u ",
						histogram_result->histogram_0_result[m]);
				}
			}
			if (strncmp(prop->name, "HISTOGRAM_1", DRM_PROP_NAME_LEN + 1) == 0) {
				fprintf(stdout,
					"crtc[%d] histogram[1] result by apb:", prop->obj_id);
				for (m = 0; m < VS_HIST_RESULT_BIN_CNT; m++) {
					if ((m % 16) == 0)
						fprintf(stdout, "\n");
					fprintf(stdout, "%u ",
						histogram_result->histogram_1_result[m]);
				}
			}
			if (strncmp(prop->name, "HISTOGRAM_2", DRM_PROP_NAME_LEN + 1) == 0) {
				fprintf(stdout,
					"crtc[%d] histogram[2] result by apb:", prop->obj_id);
				for (m = 0; m < VS_HIST_RESULT_BIN_CNT; m++) {
					if ((m % 16) == 0)
						fprintf(stdout, "\n");
					fprintf(stdout, "%u ",
						histogram_result->histogram_2_result[m]);
				}
			}
			if (strncmp(prop->name, "HISTOGRAM_3", DRM_PROP_NAME_LEN + 1) == 0) {
				fprintf(stdout,
					"crtc[%d] histogram[3] result by apb:", prop->obj_id);
				for (m = 0; m < VS_HIST_RESULT_BIN_CNT; m++) {
					if ((m % 16) == 0)
						fprintf(stdout, "\n");
					fprintf(stdout, "%u ",
						histogram_result->histogram_3_result[m]);
				}
			}
			if (strncmp(prop->name, "HISTOGRAM_RGB", DRM_PROP_NAME_LEN + 1) == 0) {
				fprintf(stdout,
					"crtc[%d] histogram[rgb] result by apb:", prop->obj_id);
				for (m = 0; m < VS_HIST_RESULT_BIN_CNT * 3; m++) {
					if ((m % 16) == 0)
						fprintf(stdout, "\n");
					fprintf(stdout, "%u ",
						histogram_result->histogram_rgb_result[m]);
				}
			}
		}
		prop++;
	}

	return 0;
}

int dtest_get_csc_coef_from_csv(const char *fileName, uint32_t csc_bit, __s32 *coef)
{
	int ret = 0;
	FILE *fp = NULL;
	char line[256];
	float coef_float[9];
	uint32_t line_count = 0;
	const uint32_t matrix_num = 9;
	const uint32_t pre_offset_num = 3;
	const uint32_t post_offset_num = 3;
	/* open the .csv */
	fp = fopen(fileName, "r");
	if (!fp) {
		printf("Can not find %s\n", fileName);
		ret = -1;
		goto exit;
	}

	/* read data */
	while (fgets(line, 256, fp)) {
		line_count++;
		/* use the strtok function to separate the fields of each row by commas */
		char *token = strtok(line, ",");

		/* read  csc matrix, line1*/
		if (line_count == 1) {
			char *fields[256] = { NULL };
			uint32_t field_count = 0;
			while (token != NULL) {
				fields[field_count] = token;
				field_count++;
				token = strtok(NULL, ",");
			}
			for (uint32_t i = 0; i < matrix_num; i++)
				coef_float[i] = atof(fields[i + 1]);

			/* fix point process */
			for (uint32_t i = 0; i < matrix_num; i++) {
				if (coef_float[i] >= 0)
					coef[i] = (__s32)(coef_float[i] * (1 << csc_bit) + 0.5);
				else
					coef[i] = (__s32)(coef_float[i] * (1 << csc_bit) - 0.5);
			}
		}
		/* read  pre offset, line2*/
		if (line_count == 2) {
			char *fields[256] = { NULL };
			uint32_t field_count = 0;
			while (token != NULL) {
				fields[field_count] = token;
				field_count++;
				token = strtok(NULL, ",");
			}

			for (uint32_t i = 0; i < pre_offset_num; i++)
				coef[matrix_num + i] = atoi(fields[i + 1]);
		}
		/* read  post offset, line3*/
		if (line_count == 3) {
			char *fields[256] = { NULL };
			uint32_t field_count = 0;
			while (token != NULL) {
				fields[field_count] = token;
				field_count++;
				token = strtok(NULL, ",");
			}
			for (uint32_t i = 0; i < post_offset_num; i++)
				coef[matrix_num + pre_offset_num + i] = atoi(fields[i + 1]);
		}
	}

exit:
	if (fp) {
		fclose(fp);
	}

	return ret;
}

uint64_t ext_fb_create(struct device *dev, char file_src[4][RESOURCE_LEN + 1], unsigned int fourcc,
		       uint64_t modifier, uint32_t num_planes, uint32_t width, uint32_t height)
{
	unsigned int fb_id = 0;
	uint32_t i = 0;
	uint32_t handles[4] = { 0 }, pitches[4] = { 0 }, offsets[4] = { 0 };
	uint64_t modifiers[4] = { 0 };
	struct bo *plane_bo[4];

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

int dtest_soft_reset(int fd, enum drm_vs_reset_mode mode)
{
	struct drm_vs_reset arg = { .mode = mode };
	return drmIoctl(fd, DRM_IOCTL_VS_SW_RESET, &arg);
}

void dtest_get_dec_decoder_fc(plane_t *plane, uint32_t id, dtest_property_t properties)
{
	uint32_t i;
	property_t property;
	cJSON *obj;

	for (i = 0; i < properties.num; i++) {
		property = properties.properties[i];
		if (!strcmp(property.name, "DEC_DECODER_FC")) {
			obj = property.json_obj;

			cJSON *fc_enable = cJSON_GetObjectItem(obj, "fcEnable");
			cJSON *fc_size = cJSON_GetObjectItem(obj, "fcSize");
			cJSON *fc_rgby = cJSON_GetObjectItem(obj, "fcRGBY");
			cJSON *fc_uv = cJSON_GetObjectItem(obj, "fcUV");

			if (fc_enable) {
				plane->dec400_fc.fc_enable = fc_enable->valueint;
				if (plane->dec400_fc.fc_enable) {
					plane->modifier |=
						(uint64_t)DRM_FORMAT_MOD_VS_DEC_FC_ENABLE;
				}
			}
			if (fc_size) {
				plane->modifier |= (uint64_t)DRM_FORMAT_MOD_VS_DEC_FC_SIZE_128;
				plane->dec400_fc.fc_size = fc_size->valueint;
				if (plane->dec400_fc.fc_size == 256)
					plane->modifier |=
						(uint64_t)DRM_FORMAT_MOD_VS_DEC_FC_SIZE_256;
			} else {
				plane->dec400_fc.fc_size = 128;
			}
			if (fc_rgby)
				plane->dec400_fc.fc_rgby_value =
					(__u32)strtoul(fc_rgby->valuestring, NULL, 16);
			if (fc_uv)
				plane->dec400_fc.fc_uv_value =
					(__u32)strtoul(fc_uv->valuestring, NULL, 16);

			break;
		}
	}
}

static void __vcmd_exception_signal_handler(int sig, siginfo_t *info, void *context)
{
	struct drm_vs_vcmd_exception excep;
	memset(&excep, 0, sizeof(struct drm_vs_vcmd_exception));

#define PRINT_EXCP(value, bit)                           \
	do {                                             \
		if (value & (1 << (bit)))                \
			DTESTLOGE("received: %s", #bit); \
	} while (0)

	if (sig == VS_VCMD_EXCEPTION_SIGNAL) {
		if (drmIoctl(device_fd, DRM_IOCTL_VS_GET_VCMD_EXCEPTION, &excep) == 0) {
			DTESTLOGE("exception:");
			DTESTLOGE("error_code: 0x%08x", excep.error_code);
			PRINT_EXCP(excep.error_code, AXI_BUS_CLEAN_DONE);
			PRINT_EXCP(excep.error_code, AXI_BUS_CLEAN_TIMEOUT);
			PRINT_EXCP(excep.error_code, AXI_RD_BUS_HANG);
			PRINT_EXCP(excep.error_code, AXI_RD_BUS_ERROR);
			PRINT_EXCP(excep.error_code, AXI_WR_BUS_HANG);
			PRINT_EXCP(excep.error_code, AXI_WR_BUS_ERROR);
			PRINT_EXCP(excep.error_code, APB_BUS_HANG);
			PRINT_EXCP(excep.error_code, VCMD_BUS_ERROR);
			PRINT_EXCP(excep.error_code, VCMD_TIMEOUT);
			PRINT_EXCP(excep.error_code, VCMD_CMD_ERR);
			DTESTLOGE("err_ctx: %u", excep.err_ctx);
			DTESTLOGE("err_frm: %u", excep.err_frm);
			DTESTLOGE("rerun_ctx: %u", excep.rerun_ctx);
			DTESTLOGE("rerun_frm: %u", excep.rerun_frm);
			if (excep.error_code & (1 << VCMD_CMD_ERR)) {
				DTESTLOGE("skipped_ctx0: %u", excep.skipped_ctx0);
				DTESTLOGE("skipped_frm0: %u", excep.skipped_frm0);
				if (excep.skipped_two_frms) {
					DTESTLOGE("skipped_ctx1: %u", excep.skipped_ctx1);
					DTESTLOGE("skipped_frm1: %u", excep.skipped_frm1);
				}
			}
		} else {
			DTESTLOGE("DRM_IOCTL_VS_GET_VCMD_EXCEPTION failed");
		}
	}
}

static void __dc_exception_signal_handler(int sig, siginfo_t *info, void *context)
{
	struct drm_vs_dc_exception excep;
	memset(&excep, 0, sizeof(struct drm_vs_dc_exception));

#define PRINT_EXCP(value, bit)                           \
	do {                                             \
		if (value & (1 << (bit)))                \
			DTESTLOGE("received: %s", #bit); \
	} while (0)

	if (sig == VS_DC_EXCEPTION_SIGNAL) {
		if (drmIoctl(device_fd, DRM_IOCTL_VS_GET_DC_EXCEPTION, &excep) == 0) {
			DTESTLOGE("exception:");
			DTESTLOGE("error_code: 0x%08x", excep.error_code);
			PRINT_EXCP(excep.error_code, VS_DC_FE0_PVRIC_DECODE_ERROR);
			PRINT_EXCP(excep.error_code, VS_DC_FE0_AXI_HANG);
			PRINT_EXCP(excep.error_code, VS_DC_FE0_AXI_BUS_ERROR);
			PRINT_EXCP(excep.error_code, VS_DC_FE0_APB_HANG);
			PRINT_EXCP(excep.error_code, VS_DC_FE1_PVRIC_DECODE_ERROR);
			PRINT_EXCP(excep.error_code, VS_DC_FE1_AXI_HANG);
			PRINT_EXCP(excep.error_code, VS_DC_FE1_AXI_BUS_ERROR);
			PRINT_EXCP(excep.error_code, VS_DC_FE1_APB_HANG);
			PRINT_EXCP(excep.error_code, VS_DC_BE_UNDERRUN);
			PRINT_EXCP(excep.error_code, VS_DC_BE_DATALOST);
			PRINT_EXCP(excep.error_code, VS_DC_BE_APB_HANG);
			PRINT_EXCP(excep.error_code, VS_DC_BE_AXI_BUS_ERROR);
			PRINT_EXCP(excep.error_code, VS_DC_BE_AXI_RD_BUS_HANG);
			PRINT_EXCP(excep.error_code, VS_DC_BE_AXI_WR_BUS_HANG);
			PRINT_EXCP(excep.error_code, VS_DC_BE_AXI_WRITE_BUS_ERROR);
			PRINT_EXCP(excep.error_code, VS_DC_BE_AXI_WRITE_BUS_HANG);
			PRINT_EXCP(excep.error_code, VS_DC_BE_BLEND_WRITE_BACK_UNDERFLOW);
			PRINT_EXCP(excep.error_code, VS_DC_BE_OUTIF_WRITE_BACK_UNDERFLOW);
		} else {
			DTESTLOGE("DRM_IOCTL_VS_GET_DC_EXCEPTION failed");
		}
	}
}

int dtest_init_excp_notif(struct device *dev)
{
	struct sigaction sa;
	struct drm_vs_vcmd_exception vcmd_excep;
	struct drm_vs_dc_exception dc_excep;

	device_fd = dev->fd;

	if (dev->use_vcmd) {
		sa.sa_sigaction = __vcmd_exception_signal_handler;
		sa.sa_flags = SA_SIGINFO;
		sigemptyset(&sa.sa_mask);

		if (sigaction(VS_VCMD_EXCEPTION_SIGNAL, &sa, NULL) == -1) {
			DTESTLOGE("signal %d register failed.", VS_VCMD_EXCEPTION_SIGNAL);
			return -1;
		} else {
			memset(&vcmd_excep, 0, sizeof(struct drm_vs_vcmd_exception));
			vcmd_excep.sign_up = true;
			if (drmIoctl(dev->fd, DRM_IOCTL_VS_GET_VCMD_EXCEPTION, &vcmd_excep) != 0)
				return -1;
		}
	} else {
		sa.sa_sigaction = __dc_exception_signal_handler;
		sa.sa_flags = SA_SIGINFO;
		sigemptyset(&sa.sa_mask);

		if (sigaction(VS_DC_EXCEPTION_SIGNAL, &sa, NULL) == -1) {
			DTESTLOGE("signal %d register failed.", VS_DC_EXCEPTION_SIGNAL);
			return -1;
		} else {
			memset(&dc_excep, 0, sizeof(struct drm_vs_dc_exception));
			dc_excep.sign_up = true;
			if (drmIoctl(dev->fd, DRM_IOCTL_VS_GET_DC_EXCEPTION, &dc_excep) != 0)
				return -1;
		}
	}

	return 0;
}
