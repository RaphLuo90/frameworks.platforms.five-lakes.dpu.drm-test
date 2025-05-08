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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "case_builder.h"
#include "json/cJSON.h"
#include "drmtest_helper.h"
#include "prop_mapper.h"
#include "drm/drm_fourcc.h"
#include "util/log.h"
#include "util/error.h"
#include "util/format.h"

static uint32_t __get_prop_num(cJSON *obj)
{
	uint32_t size = 0;
	cJSON *props = cJSON_GetObjectItem(obj, "property");
	if (props) {
		size = cJSON_GetArraySize(props);
	}
	return size;
}

static dtest_status __alloc_builder(cJSON *root, dtest_pipe_t *pipes, dtest_plane_t *planes,
				    dtest_cursor_t *cursors, dtest_crtc_t *crtcs,
				    dtest_property_t *properties)
{
	cJSON *obj_planes, *obj_cursors, *obj_connectors, *obj_crtcs;
	cJSON *item;
	uint32_t i, total_prop_num;

	total_prop_num = 0; // total property numbers
	obj_planes = cJSON_GetObjectItem(root, "plane");
	if (obj_planes) {
		planes->num = cJSON_GetArraySize(obj_planes);
		planes->__property.num = 0;
		for (i = 0; i < planes->num; i++) {
			item = cJSON_GetArrayItem(obj_planes, i);
			planes->__property.num += __get_prop_num(item);
		}
		planes->planes = (plane_t *)malloc(sizeof(plane_t) * planes->num);
		memset(planes->planes, 0, sizeof(plane_t) * planes->num);

		planes->__property.properties =
			(property_t *)malloc(sizeof(property_t) * planes->__property.num);
		memset(planes->__property.properties, 0,
		       sizeof(property_t) * planes->__property.num);

		total_prop_num += planes->__property.num;
	}

	obj_cursors = cJSON_GetObjectItem(root, "cursor");
	if (obj_cursors) {
		cursors->num = cJSON_GetArraySize(obj_cursors);
		cursors->__property.num = 0;
		for (i = 0; i < cursors->num; i++) {
			item = cJSON_GetArrayItem(obj_cursors, i);
			cursors->__property.num += __get_prop_num(item);
		}
		cursors->cursors = (cursor_t *)malloc(sizeof(cursor_t) * cursors->num);
		memset(cursors->cursors, 0, sizeof(cursor_t) * cursors->num);

		cursors->__property.properties =
			(property_t *)malloc(sizeof(property_t) * cursors->__property.num);
		memset(cursors->__property.properties, 0,
		       sizeof(property_t) * cursors->__property.num);

		total_prop_num += cursors->__property.num;
	}

	obj_connectors = cJSON_GetObjectItem(root, "connector");
	if (obj_connectors) {
		pipes->num = cJSON_GetArraySize(obj_connectors);
		pipes->__property.num = 0;
		for (i = 0; i < pipes->num; i++) {
			item = cJSON_GetArrayItem(obj_connectors, i);
			pipes->__property.num += __get_prop_num(item);
		}
		pipes->pipes = (pipe_t *)malloc(sizeof(pipe_t) * pipes->num);
		memset(pipes->pipes, 0, sizeof(pipe_t) * pipes->num);

		pipes->__property.properties =
			(property_t *)malloc(sizeof(property_t) * pipes->__property.num);
		memset(pipes->__property.properties, 0, sizeof(property_t) * pipes->__property.num);

		total_prop_num += pipes->__property.num;
	}

	obj_crtcs = cJSON_GetObjectItem(root, "crtc");
	if (obj_crtcs) {
		crtcs->num = cJSON_GetArraySize(obj_crtcs);
		crtcs->__property.num = 0;
		for (i = 0; i < crtcs->num; i++) {
			item = cJSON_GetArrayItem(obj_crtcs, i);
			crtcs->__property.num += __get_prop_num(item);
		}

		total_prop_num += crtcs->__property.num;

		crtcs->__property.properties =
			(property_t *)malloc(sizeof(property_t) * crtcs->__property.num);
		memset(crtcs->__property.properties, 0, sizeof(property_t) * crtcs->__property.num);
	}
	// TODO: crtc pointer is a placeholder, need alloc struct in the future.

	properties->num = total_prop_num;
	properties->properties = (property_t *)malloc(sizeof(property_t) * properties->num);
	memset(properties->properties, 0, sizeof(property_t) * properties->num);
#if 0
	DTESTLOGI("planes[%u %u] connector[%u %u] crtc[%u %u] property[%u]",
		planes->num, planes->__property.num,
		pipes->num, pipes->__property.num,
		crtcs->num, crtcs->__property.num,
		properties->num
		);
#endif

	return DTEST_SUCCESS;
}

static uint32_t __get_component_id(const char *str)
{
	// TODO: check string valid
	return atoi(str);
}

static void __dump_property(dtest_property_t *properties)
{
	uint32_t i;
	property_t *prop = properties->properties;
	DTESTLOGI("Total property num = %u", properties->num);
	for (i = 0; i < properties->num; i++) {
		DTESTLOGI("property obj_id[%u] obj_type[%u] name[%s] value[%lu]", prop->obj_id,
			  prop->obj_type, prop->name, prop->value);
		prop++;
	}
}

static dtest_status __parse_golden(cJSON *obj_golden, char **golden)
{
	*golden = dtest_get_obj_string(obj_golden, "md5sum");
	return DTEST_SUCCESS;
}

static uint32_t __parse_property(cJSON *obj_prop, uint32_t obj_id, uint32_t obj_type,
				 property_t *property)
{
	cJSON *item;
	uint32_t size, i;
	property_t *prop = property;
	size = cJSON_GetArraySize(obj_prop);
	for (i = 0; i < size; i++) {
		item = cJSON_GetArrayItem(obj_prop, i);

		memset(prop->name, 0, DRM_PROP_NAME_LEN + 1);
		if (sizeof(prop->name) - 1 < strlen(item->string)) {
			DTESTLOGE("property name overflow, %s", item->string);
			return 0;
		}
		strncpy(prop->name, item->string, sizeof(prop->name) - 1);
		prop->obj_id = obj_id;
		prop->obj_id_ori = prop->obj_id;
		prop->obj_type = obj_type;
		prop->value = 0; // value will be set after create fb
		prop->json_obj = item;

		prop++;
	}

	return size;
}

static dtest_status __parse_plane(cJSON *obj_plane, plane_t *plane)
{
	const struct util_format_info *info = NULL;
	uint32_t i = 0, plane_num = 0;
	char *tileStatus[3] = { "tileStatus", "tileStatus1", "tileStatus2" };

	plane->plane_id = __get_component_id(obj_plane->string);
	plane->plane_id_ori = plane->plane_id;
	plane->global_disable = dtest_get_obj_global_disable(obj_plane, "globalDisable");
	if (plane->global_disable)
		return DTEST_SUCCESS;

	plane->crtc_id = dtest_get_obj_int(obj_plane, "crtc");
	plane->crtc_id_ori = plane->crtc_id;
	plane->fourcc = dtest_get_obj_format(obj_plane, plane->format_str);
	plane->w = dtest_get_obj_int(obj_plane, "width");
	plane->h = dtest_get_obj_int(obj_plane, "height");
	plane->scale = 1.0;
	plane->crtc_x = dtest_get_obj_int(obj_plane, "crtcX");
	plane->crtc_y = dtest_get_obj_int(obj_plane, "crtcY");
	plane->crtc_w = dtest_get_obj_int(obj_plane, "crtcWidth");
	plane->crtc_h = dtest_get_obj_int(obj_plane, "crtcHeight");
	plane->rotation = dtest_get_obj_rotation(obj_plane, "rotation");
	plane->std_blend_mode = dtest_get_obj_std_blend_mode(obj_plane, "stdBlendMode");
	plane->std_blend_alpha = dtest_get_obj_std_blend_alpha(obj_plane, "stdBlendAlpha");
	plane->zpos = dtest_get_obj_zpos(obj_plane, "zpos");
	dtest_get_obj_str(obj_plane, "resource", plane->file_src[0]);
	plane->color_encoding = dtest_get_obj_color_encoding(obj_plane, "yuvColorEncoding");
	plane->color_range = dtest_get_obj_color_range(obj_plane, "yuvColorRange");
	plane->modifier = dtest_get_obj_dec_fb_modifier(obj_plane, "decType");

	if (fourcc_mod_vs_is_compressed(plane->modifier)) {
		info = util_format_info_find(plane->fourcc);
		plane_num = info->num_planes;

		for (i = 0; i < plane_num; i++)
			dtest_get_obj_str(obj_plane, tileStatus[i], plane->file_src[i + 1]);
	}
	return DTEST_SUCCESS;
}

static dtest_status __parse_cursor(cJSON *obj_cursor, cursor_t *cursor)
{
	cursor->cursor_id = __get_component_id(obj_cursor->string);
	cursor->crtc_id = dtest_get_obj_int(obj_cursor, "crtc");
	cursor->fourcc = dtest_get_obj_format(obj_cursor, cursor->format_str);
	cursor->x = dtest_get_obj_int(obj_cursor, "cursorPosX");
	cursor->y = dtest_get_obj_int(obj_cursor, "cursorPosY");
	cursor->w = dtest_get_obj_int(obj_cursor, "cursorWidth");
	cursor->h = dtest_get_obj_int(obj_cursor, "cursorHeight");
	cursor->hot_x = dtest_get_obj_int(obj_cursor, "cursorHotX");
	cursor->hot_y = dtest_get_obj_int(obj_cursor, "cursorHotY");
	if (cursor->hot_x > cursor->w || cursor->hot_y > cursor->h) {
		DTESTLOGE(
			"cursor hot x:%u expand cursor width:%u or cursor hot y:%u expand cursor height:%u\n",
			cursor->hot_x, cursor->w, cursor->h, cursor->hot_y);
		return DTEST_FAILURE;
	}
	return DTEST_SUCCESS;
}

static dtest_status __parse_connector(cJSON *obj_connector, pipe_t *pipe)
{
	// TODO: refine num_cons, con_ids, cons
	uint32_t w, h;
	cJSON *obj_golden;
	cJSON *obj_wb_w = 0;
	cJSON *obj_wb_h = 0;

	pipe->num_cons = 1;
	pipe->con_ids = calloc(pipe->num_cons, sizeof(*pipe->con_ids));
	pipe->con_ids_ori = calloc(pipe->num_cons, sizeof(*pipe->con_ids_ori));
	if (!pipe->con_ids) {
		DTESTLOGE("calloc failed for con_ids");
		return DTEST_FAILURE;
	}
	pipe->cons = calloc(pipe->num_cons, sizeof(*pipe->cons));
	if (!pipe->cons) {
		DTESTLOGE("calloc failed for cons");
		return DTEST_FAILURE;
	}
	pipe->cons[0] = strdup(obj_connector->string); // TODO: need refine

	pipe->con_ids[0] = __get_component_id(pipe->cons[0]);
	pipe->con_ids_ori[0] = pipe->con_ids[0];
	pipe->crtc_id = dtest_get_obj_int(obj_connector, "crtc");
	pipe->crtc_id_ori = pipe->crtc_id;
	pipe->global_disable = dtest_get_obj_global_disable(obj_connector, "globalDisable");
	if (pipe->global_disable)
		return DTEST_SUCCESS;

	pipe->wb_for_ramless = dtest_get_bool_obj_by_name(obj_connector, "WBForRamLess");
	pipe->wb_with_stripe = dtest_get_bool_obj_by_name(obj_connector, "WBWithStripe");
	pipe->vrefresh = dtest_get_obj_int(obj_connector, "refresh");
	pipe->vrr_enable = dtest_get_obj_vrr_enable(obj_connector, "vrrEnable");
	pipe->fourcc = dtest_get_obj_format(obj_connector, pipe->format_str);
	pipe->wb_yuv444_10bit = (strcmp(pipe->format_str, "YUV444_10BIT")) ? false : true;
	pipe->wb_modifier = dtest_get_obj_dec_fb_modifier(obj_connector, "decType");

	w = dtest_get_obj_int(obj_connector, "width");
	h = dtest_get_obj_int(obj_connector, "height");

	if (pipe->vrr_enable)
		pipe->vrr_refresh = dtest_get_obj_vrr_refresh(obj_connector, "vrrRefresh");

	/* get wb size*/
	obj_wb_w = cJSON_GetObjectItem(obj_connector, "wb_width");
	obj_wb_h = cJSON_GetObjectItem(obj_connector, "wb_height");

	if (obj_wb_w)
		pipe->wb_w = obj_wb_w->valueint;
	else
		pipe->wb_w = w;

	if (obj_wb_h)
		pipe->wb_h = obj_wb_h->valueint;
	else
		pipe->wb_h = h;

	sprintf(pipe->mode_str, "%ux%u", w, h);

	obj_golden = cJSON_GetObjectItem(obj_connector, "golden");
	if (obj_golden) {
		__parse_golden(obj_golden, &pipe->md5_golden);
	}
	return DTEST_SUCCESS;
}

static dtest_status __build_crtcs(cJSON *obj_crtcs, dtest_crtc_t *crtcs)
{
	int32_t size, i;
	dtest_status ret;
	cJSON *obj_crtc, *obj_prop;
	property_t *property;
	if (NULL == obj_crtcs) {
		DTESTLOGE("build_crtcs invalid value");
		return DTEST_FAILURE;
	}

	ret = DTEST_SUCCESS;
	property = crtcs->__property.properties;
	size = cJSON_GetArraySize(obj_crtcs);
	for (i = 0; i < size; i++) {
		obj_crtc = cJSON_GetArrayItem(obj_crtcs, i);
		uint32_t crtc_id = __get_component_id(obj_crtc->string);
		// TODO: no need parse crtc's attribute

		obj_prop = cJSON_GetObjectItem(obj_crtc, "property");
		if (obj_prop) {
			ret = __parse_property(obj_prop, crtc_id, DRM_MODE_OBJECT_CRTC, property);
			if (ret <= 0) {
				DTESTLOGE("parse_property fail");
				return DTEST_FAILURE;
			}
			property += ret;
		}
		// crtc++;  // TODO: no structure here
	}

	return DTEST_SUCCESS;
}

static dtest_status __build_connectors(cJSON *obj_connectors, dtest_pipe_t *pipes)
{
	pipe_t *pipe;
	int32_t size, i;
	dtest_status ret;
	cJSON *obj_connector, *obj_prop;
	property_t *property;
	if (NULL == obj_connectors) {
		DTESTLOGE("build_connectors invalid value");
		return DTEST_FAILURE;
	}

	ret = DTEST_SUCCESS;
	pipe = pipes->pipes;
	property = pipes->__property.properties;
	size = cJSON_GetArraySize(obj_connectors);
	for (i = 0; i < size; i++) {
		obj_connector = cJSON_GetArrayItem(obj_connectors, i);
		ret = __parse_connector(obj_connector, pipe);
		if (ret != DTEST_SUCCESS) {
			DTESTLOGE("parse_connector fail");
			return ret;
		}

		obj_prop = cJSON_GetObjectItem(obj_connector, "property");
		if (obj_prop) {
			ret = __parse_property(obj_prop, pipe->con_ids[0],
					       DRM_MODE_OBJECT_CONNECTOR, property);
			if (ret <= 0) {
				DTESTLOGE("parse_property fail");
				return DTEST_FAILURE;
			}

			property += ret;
		}
		pipe++;
	}

#if 0 // for debug
	pipe = pipes->pipes;
	printf("=============connector parser==============\n");
	printf("pipe num = %d\n", pipes->num);
	for(i = 0; i < size; i++) {
		printf("pipe id: %d\n", pipe->con_ids[0]);
		printf("crtc id: %d\n", pipe->crtc_id);
		printf("fourcc: %s %d\n", pipe->format_str, pipe->fourcc);
		pipe++;
	}
	__dump_property(&pipes->__property);
#endif
	return DTEST_SUCCESS;
}

static dtest_status __build_planes(cJSON *obj_planes, dtest_plane_t *planes)
{
	plane_t *plane;
	int32_t size, i;
	dtest_status ret;
	cJSON *obj_plane, *obj_prop;
	property_t *property;
	if (NULL == obj_planes) {
		DTESTLOGE("build_planes invalid value");
		return DTEST_FAILURE;
	}

	ret = DTEST_SUCCESS;
	plane = planes->planes;
	property = planes->__property.properties;
	size = cJSON_GetArraySize(obj_planes);
	for (i = 0; i < size; i++) {
		obj_plane = cJSON_GetArrayItem(obj_planes, i);
		ret = __parse_plane(obj_plane, plane);
		if (ret != DTEST_SUCCESS) {
			DTESTLOGE("parse_plane fail");
			return ret;
		}

		obj_prop = cJSON_GetObjectItem(obj_plane, "property");
		if (obj_prop) {
			ret = __parse_property(obj_prop, plane->plane_id, DRM_MODE_OBJECT_PLANE,
					       property);
			if (ret <= 0) {
				DTESTLOGE("parse_property fail");
				return DTEST_FAILURE;
			}
			property += ret;
		}

		dtest_get_dec_decoder_fc(plane, plane->plane_id, planes->__property);
		plane++;
	}

#if 0 // for debug
	plane = planes->planes;
	printf("=============plane parser==============\n");
	printf("plane num = %d\n", planes->num);
	for(i = 0; i < size; i++) {
		printf("plane id: %d\n", plane->plane_id);
		printf("crtc id: %d\n", plane->crtc_id);
		printf("WxH: %dx%d\n", plane->w, plane->h);
		printf("fourcc: %s %d\n", plane->format_str, plane->fourcc);
		plane++;
	}
	__dump_property(&planes->__property);
#endif

	return DTEST_SUCCESS;
}

static dtest_status __build_cursors(cJSON *obj_cursors, dtest_cursor_t *cursors)
{
	cursor_t *cursor;
	int32_t size, i;
	dtest_status ret;
	cJSON *obj_cursor, *obj_prop;
	property_t *property;
	if (NULL == obj_cursors) {
		DTESTLOGE("build_cursors invalid value");
		return DTEST_FAILURE;
	}

	ret = DTEST_SUCCESS;
	cursor = cursors->cursors;
	property = cursors->__property.properties;
	size = cJSON_GetArraySize(obj_cursors);
	for (i = 0; i < size; i++) {
		obj_cursor = cJSON_GetArrayItem(obj_cursors, i);
		ret = __parse_cursor(obj_cursor, cursor);
		if (ret != DTEST_SUCCESS) {
			DTESTLOGE("parse_cursor fail");
			return ret;
		}

		obj_prop = cJSON_GetObjectItem(obj_cursor, "property");
		if (obj_prop) {
			ret = __parse_property(obj_prop, cursor->cursor_id, DRM_MODE_OBJECT_PLANE,
					       property);
			if (ret <= 0) {
				DTESTLOGE("parse_property fail");
				return DTEST_FAILURE;
			}
			property += ret;
		}
		cursor++;
	}

	return DTEST_SUCCESS;
}

dtest_status __splice_property(dtest_pipe_t *pipes, dtest_plane_t *planes, dtest_cursor_t *cursors,
			       dtest_crtc_t *crtcs, dtest_property_t *properties)
{
	uint32_t total_property;
	property_t *prop = properties->properties;

	total_property = pipes->__property.num + planes->__property.num + cursors->__property.num +
			 crtcs->__property.num;
	if (total_property != properties->num) {
		DTESTLOGE("property num %u -- %u, error", total_property, properties->num);
		return DTEST_FAILURE;
	}
	memcpy(prop, planes->__property.properties, sizeof(property_t) * planes->__property.num);
	prop += planes->__property.num;
	memcpy(prop, cursors->__property.properties, sizeof(property_t) * cursors->__property.num);
	prop += cursors->__property.num;
	memcpy(prop, pipes->__property.properties, sizeof(property_t) * pipes->__property.num);
	prop += pipes->__property.num;
	memcpy(prop, crtcs->__property.properties, sizeof(property_t) * crtcs->__property.num);

#if 0 // for debug
	__dump_property(properties);
#endif
	return DTEST_SUCCESS;
}

dtest_status build_frame(cJSON *obj_frame, frame_t *frame)
{
	dtest_status status;
	cJSON *obj_planes, *obj_cursors, *obj_connectors, *obj_crtcs, *obj_random;
	dtest_pipe_t *pipes = &frame->pipe;
	dtest_plane_t *planes = &frame->plane;
	dtest_cursor_t *cursors = &frame->cursor;
	dtest_crtc_t *crtcs = &frame->crtc;
	dtest_property_t *properties = &frame->property;

	status = __alloc_builder(obj_frame, pipes, planes, cursors, crtcs, properties);
	DTEST_CHECK_STATUS(status, final);

	obj_random = cJSON_GetObjectItem(obj_frame, "random");
	if (obj_random) {
		frame->random = obj_random->valueint;
	}

	obj_planes = cJSON_GetObjectItem(obj_frame, "plane");
	if (obj_planes) {
		status = __build_planes(obj_planes, planes);
		DTEST_CHECK_STATUS(status, final);
	}

	obj_cursors = cJSON_GetObjectItem(obj_frame, "cursor");
	if (obj_cursors) {
		status = __build_cursors(obj_cursors, cursors);
		DTEST_CHECK_STATUS(status, final);
	}

	obj_connectors = cJSON_GetObjectItem(obj_frame, "connector");
	if (obj_connectors) {
		status = __build_connectors(obj_connectors, pipes);
		DTEST_CHECK_STATUS(status, final);
	}

	obj_crtcs = cJSON_GetObjectItem(obj_frame, "crtc");
	if (obj_crtcs) {
		status = __build_crtcs(obj_crtcs, crtcs);
		DTEST_CHECK_STATUS(status, final);
	}

	status = __splice_property(pipes, planes, cursors, crtcs, properties);
	DTEST_CHECK_STATUS(status, final);

final:
	return status;
}

dtest_status dtest_case_props_map(device_t *dev, dtest_property_t *props)
{
	uint32_t i;
	property_t *prop = props->properties;
	dtest_status status = DTEST_SUCCESS;

	for (i = 0; i < props->num; i++) {
		status = dtest_prop_map(dev, prop);
		if (status == DTEST_SUCCESS)
			prop++;
		else
			return status;
	}
#if 1
	__dump_property(props);
#endif
	return DTEST_SUCCESS;
}

void dtest_case_delete(dtest_frame_t *dtest_frames)
{
	uint32_t i;
	dtest_pipe_t *pipes;
	dtest_plane_t *planes;
	dtest_cursor_t *cursors;
	dtest_crtc_t *crtcs;
	dtest_property_t *properties;
	frame_t *frame = dtest_frames->frames;
	for (i = 0; i < dtest_frames->num; i++) {
		if (NULL == frame) {
			return;
		}
		pipes = &frame->pipe;
		planes = &frame->plane;
		cursors = &frame->cursor;
		crtcs = &frame->crtc;
		properties = &frame->property;
		if (pipes && pipes->pipes) {
			free(pipes->pipes);
			pipes->pipes = NULL;
			if (pipes->__property.properties) {
				free(pipes->__property.properties);
				pipes->__property.properties = NULL;
			}
		}

		if (planes && planes->planes) {
			free(planes->planes);
			planes->planes = NULL;
			if (planes->__property.properties) {
				free(planes->__property.properties);
				planes->__property.properties = NULL;
			}
		}

		if (cursors && cursors->cursors) {
			free(cursors->cursors);
			cursors->cursors = NULL;
			if (cursors->__property.properties) {
				free(cursors->__property.properties);
				cursors->__property.properties = NULL;
			}
		}

		if (crtcs) {
			// crtcs->crtc is a placeholder here.
			if (crtcs->__property.properties) {
				free(crtcs->__property.properties);
				crtcs->__property.properties = NULL;
			}
		}

		if (properties && properties->properties) {
			free(properties->properties);
			properties->properties = NULL;
		}

		frame++;
	}

	if (dtest_frames->frames) {
		free(dtest_frames->frames);
		dtest_frames->frames = NULL;
	}
}

cJSON *dtest_case_setup(const char *path, dtest_frame_t *dtest_frames)
{
	cJSON *obj_frames, *item;
	frame_t *frames = NULL;
	dtest_status status;
	int32_t size, i;
	cJSON *root = dtest_load_json(path);
	DTEST_CHECK_PTR(root, error);

	obj_frames = cJSON_GetObjectItem(root, "frame");
	if (obj_frames) {
		size = cJSON_GetArraySize(obj_frames);
		frames = (frame_t *)malloc(sizeof(frame_t) * size);
		if (!frames) {
			fprintf(stderr, "Memory allocation failed.");
			if (root) {
				cJSON_Delete(root);
			}
			return NULL;
		}
		for (i = 0; i < size; i++) {
			item = cJSON_GetArrayItem(obj_frames, i);
			status = build_frame(item, &frames[i]);
			DTEST_CHECK_STATUS(status, error);

			frames[i].frame_id = __get_component_id(item->string);
		}
		dtest_frames->num = size;
	} else {
		frames = (frame_t *)malloc(sizeof(frame_t));
		if (!frames) {
			fprintf(stderr, "Memory allocation failed.");
			if (root) {
				cJSON_Delete(root);
			}
			return NULL;
		}
		status = build_frame(root, &frames[0]);
		DTEST_CHECK_STATUS(status, error);

		frames[0].frame_id = 0;
		dtest_frames->num = 1;
	}
	dtest_frames->frames = frames;

	return root;
error:
	free(frames);
	if (root) {
		cJSON_Delete(root);
	}
	return NULL;
}
