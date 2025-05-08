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

#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>

#include "viv_dc_functions.h"
#include "drmtest.h"
#include "prop_mapper.h"
#if CONFIG_VERISILICON_MD5_CHECK
#include <openssl/md5.h>
#endif
#define _DIV_ROUND_CLOSEST(x, divisor)                               \
	({                                                           \
		typeof(x) __x = x;                                   \
		typeof(divisor) __d = divisor;                       \
		(((typeof(x))-1) > 0 || ((typeof(divisor))-1) > 0 || \
		 (((__x) > 0) == ((__d) > 0))) ?                     \
			      (((__x) + ((__d) / 2)) / (__d)) :            \
			      (((__x) - ((__d) / 2)) / (__d));             \
	})

#define fourcc_mod_vs_get_type(val) (((val)&DRM_FORMAT_MOD_VS_TYPE_MASK) >> 53)
#define fourcc_mod_vs_get_tile_mode(val) ((val)&DRM_FORMAT_MOD_VS_DEC_TILE_MODE_MASK)

static enum util_fill_pattern primary_fill = UTIL_PATTERN_SMPTE;
static enum util_fill_pattern plain_fill = UTIL_PATTERN_PLAIN;

/* preprocessing of streams */
static int get_stream_num(struct stream_arg *p, const struct util_format_info *info, uint64_t mod)
{
	int ret = 0;
	char file_src[(RESOURCE_LEN + 4) * 2];
	struct stat stat_buf;
	uint32_t width = p->w, height = p->h;
	uint32_t size, frame_size, count;

	sprintf(file_src, "resource/%s", p->file_src[0]);
	stat(file_src, &stat_buf);
	size = stat_buf.st_size;

	drm_vs_get_align_size(&width, &height, info->format, mod);

	switch (info->format) {
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_NV16:
		frame_size = (width * height) * 3 / 2;
		break;
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_ARGB2101010:
		frame_size = (width * height) * 4;
		break;
	default:
		DTESTLOGE("Invalid format");
		ret = -1;
		goto exit;
	}
	count = size / frame_size;
	DTESTLOGI("total number of streams: %d", count);
	p->num_frames = count;
exit:
	return ret;
}

dtest_status initialize_virtual_display(device_t *dev, uint32_t vdp_out_addr)
{
	uint32_t vdp_fd = 0;

	vdp_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (vdp_fd < 0) {
		printf("open dev/mem failed.\n");
		return DTEST_FAILURE;
	}

	dev->vdp_buffer_logic =
		mmap(NULL, 0x200, PROT_READ | PROT_WRITE, MAP_SHARED, vdp_fd, vdp_out_addr);

	return DTEST_SUCCESS;
}

dtest_status dpu_load_frames(cJSON *root, dtest_frame_t *dtest_frames,
			     dtest_stream_t *dtest_streams, bool *is_stream)
{
	cJSON *obj_frames, *item;
	frame_t *frames = NULL;
	dtest_status status;
	int32_t size = 0, i = 0, j = 0;
	/* updata for stream */
	cJSON *obj_plane = NULL, *plane = NULL;
	cJSON *obj_streams = NULL;
	stream_t *streams = NULL;
	const struct util_format_info *info = NULL;
	DTEST_CHECK_PTR(root, error);

	obj_plane = cJSON_GetObjectItem(root, "plane");
	obj_frames = cJSON_GetObjectItem(root, "frame");
	obj_streams = cJSON_GetObjectItem(root, "stream");
	if (obj_frames) {
		size = cJSON_GetArraySize(obj_frames);
		frames = (frame_t *)calloc(size, sizeof(frame_t));
		if (!frames) {
			fprintf(stderr, "Memory allocation failed.\n");
			return -1;
		}
		for (i = 0; i < size; i++) {
			item = cJSON_GetArrayItem(obj_frames, i);
			status = build_frame(item, &frames[i]);
			DTEST_CHECK_STATUS(status, error);

			frames[i].frame_id = atoi(item->string);
		}
		dtest_frames->num = size;
		dtest_frames->run_stream = false;
	} else if (obj_streams) {
		size = cJSON_GetArraySize(obj_streams);
		streams = (stream_t *)calloc(size, sizeof(stream_t));
		if (!streams) {
			fprintf(stderr, "Memory allocation failed.\n");
			return -1;
		}

		for (i = 0; i < size; i++) {
			stream_arg_t stream = { 0 };
			dtest_frames = (dtest_frame_t *)calloc(1, sizeof(dtest_frame_t));
			item = cJSON_GetArrayItem(obj_streams, i);
			obj_plane = cJSON_GetObjectItem(item, "plane");
			plane = cJSON_GetArrayItem(obj_plane, 0);
			dtest_get_obj_str(plane, "resource", stream.file_src[0]);
			stream.w = dtest_get_obj_int(plane, "width");
			stream.h = dtest_get_obj_int(plane, "height");
			stream.fourcc = dtest_get_obj_format(plane, stream.format_str);
			stream.modifier = dtest_get_obj_dec_fb_modifier(plane, "decType");

			info = util_format_mod_info_find(stream.fourcc, stream.modifier);
			if (info == NULL) {
				fprintf(stderr,
					"stream with format %u has no matched format info\n",
					stream.fourcc);
				return -1;
			}

			if (!get_stream_num(&stream, info, stream.modifier))
				dtest_frames->num = stream.num_frames;
			frames = (frame_t *)calloc(dtest_frames->num, sizeof(frame_t));
			if (!frames) {
				fprintf(stderr, "Memory allocation failed.\n");
				return -1;
			}

			for (j = 0; j < dtest_frames->num; j++) {
				status = build_frame(item, &frames[j]);
				DTEST_CHECK_STATUS(status, error);
				for (int pipe_num = 0; pipe_num < frames[j].pipe.num; pipe_num++) {
					frames[j].pipe.pipes[pipe_num].stream = true;
					frames[j].pipe.pipes[pipe_num].frame_idx = j;
				}
				frames[j].frame_id = j;
			}
			dtest_frames->frames = frames;
			dtest_frames->run_stream = true;
			streams[i].stream_id = i;
			streams[i].dtest_frame = dtest_frames;
		}

		dtest_streams->num = size;
		dtest_streams->streams = streams;
		*is_stream = true;
	} else {
		frames = (frame_t *)calloc(1, sizeof(frame_t));
		if (!frames) {
			fprintf(stderr, "Memory allocation failed.\n");
			return -1;
		}
		status = build_frame(root, &frames[0]);
		DTEST_CHECK_STATUS(status, error);

		frames[0].frame_id = 0;
		dtest_frames->num = 1;
		dtest_frames->run_stream = false;
	}
	dtest_frames->frames = frames;
	return DTEST_SUCCESS;
error:
	if (root) {
		cJSON_Delete(root);
	}
	if (frames)
		free(frames);
	return DTEST_FAILURE;
}

int dpu_open_device(const char *device, const char *module)
{
	int ret;
	ret = util_open(device, module);

	return ret;
}

struct device_config *dpu_create_device_config(uint64_t type[], uint64_t value[], int len_type,
					       int len_value)
{
	int i;
	struct device_config *config;

	if (len_type != len_value)
		return NULL;

	config = calloc(len_type, sizeof(*config));
	if (config == 0)
		return NULL;

	for (i = 0; i < len_type; i++) {
		(config + i)->type = type[i];
		(config + i)->value = value[i];
	}

	return config;
}

int dpu_load_device_config(int fd, struct device_config *config, int len_device_config)
{
	int i;
	int ret = 0;

	for (i = 0; i < len_device_config; i++) {
		ret = drmSetClientCap(fd, (config + i)->type, (config + i)->value);
		if (ret) {
			DTESTLOGE("the config#%d not support: %s", i, strerror(errno));
			break;
		}
	}

	return ret;
}

struct resources *dpu_get_device_resources(struct device *dev)
{
	struct resources *res;
	int i;

	res = calloc(1, sizeof(*res));
	if (res == 0)
		return NULL;

	drmSetClientCap(dev->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

	res->res = drmModeGetResources(dev->fd);
	if (!res->res) {
		fprintf(stderr, "drmModeGetResources failed: %s\n", strerror(errno));
		goto error;
	}

	res->crtcs = calloc(res->res->count_crtcs, sizeof(*res->crtcs));
	res->encoders = calloc(res->res->count_encoders, sizeof(*res->encoders));
	res->connectors = calloc(res->res->count_connectors, sizeof(*res->connectors));
	res->fbs = calloc(res->res->count_fbs, sizeof(*res->fbs));

	if (!res->crtcs || !res->encoders || !res->connectors || !res->fbs)
		goto error;

#define get_resource(_res, __res, type, Type)                                         \
	do {                                                                          \
		for (i = 0; i < (int)(_res)->__res->count_##type##s; ++i) {           \
			(_res)->type##s[i].type =                                     \
				drmModeGet##Type(dev->fd, (_res)->__res->type##s[i]); \
			if (!(_res)->type##s[i].type)                                 \
				fprintf(stderr, "could not get %s %i: %s\n", #type,   \
					(_res)->__res->type##s[i], strerror(errno));  \
		}                                                                     \
	} while (0)

	get_resource(res, res, crtc, Crtc);
	get_resource(res, res, encoder, Encoder);
	get_resource(res, res, connector, Connector);
	get_resource(res, res, fb, FB);

	/* Set the name of all connectors based on the type name and the per-type ID. */
	for (i = 0; i < res->res->count_connectors; i++) {
		struct connector *connector = &res->connectors[i];
		drmModeConnector *conn = connector->connector;
		int num;

		num = asprintf(&connector->name, "%s-%u",
			       util_lookup_connector_type_name(conn->connector_type),
			       conn->connector_type_id);
		if (num < 0)
			goto error;
	}

#define get_properties(_res, __res, type, Type)                                                \
	do {                                                                                   \
		for (i = 0; i < (int)(_res)->__res->count_##type##s; ++i) {                    \
			struct type *obj = &res->type##s[i];                                   \
			unsigned int j;                                                        \
			obj->props = drmModeObjectGetProperties(dev->fd, obj->type->type##_id, \
								DRM_MODE_OBJECT_##Type);       \
			if (!obj->props) {                                                     \
				fprintf(stderr, "could not get %s %i properties: %s\n", #type, \
					obj->type->type##_id, strerror(errno));                \
				continue;                                                      \
			}                                                                      \
			obj->props_info =                                                      \
				calloc(obj->props->count_props, sizeof(*obj->props_info));     \
			if (!obj->props_info)                                                  \
				continue;                                                      \
			for (j = 0; j < obj->props->count_props; ++j)                          \
				obj->props_info[j] =                                           \
					drmModeGetProperty(dev->fd, obj->props->props[j]);     \
		}                                                                              \
	} while (0)

	get_properties(res, res, crtc, CRTC);
	get_properties(res, res, connector, CONNECTOR);

	for (i = 0; i < res->res->count_crtcs; ++i) {
		uint32_t blob_id = 0;
		drmModePropertyBlobPtr blob_ptr = NULL;

		res->crtcs[i].mode = &res->crtcs[i].crtc->mode;

		/* Get the DC info */
		dev->resources = res;
		blob_id = get_crtc_property_default_value(dev, res->crtcs[i].crtc->crtc_id,
							  "DC_INFO");
		dev->resources = NULL;
		if (!blob_id)
			DTESTLOGE("Cannot get the DC info for CRTC %d.\n",
				  res->crtcs[i].crtc->crtc_id);
		else {
			blob_ptr = drmModeGetPropertyBlob(dev->fd, blob_id);
			if (!blob_ptr || !blob_ptr->data) {
				DTESTLOGE("Failed to get the BLOB data for %d.\n", blob_id);
				drmModeFreePropertyBlob(blob_ptr);
				continue;
			}

			memcpy(&res->crtcs[i].info, blob_ptr->data, sizeof(struct drm_vs_dc_info));
			drmModeFreePropertyBlob(blob_ptr);
		}
	}

	res->plane_res = drmModeGetPlaneResources(dev->fd);
	if (!res->plane_res) {
		fprintf(stderr, "drmModeGetPlaneResources failed: %s\n", strerror(errno));
		return res;
	}

	res->planes = calloc(res->plane_res->count_planes, sizeof(*res->planes));
	if (!res->planes)
		goto error;

	get_resource(res, plane_res, plane, Plane);
	get_properties(res, plane_res, plane, PLANE);

	return res;

error:
	dpu_free_resources(res);
	return NULL;
}

static int bo_fb_create(int fd, unsigned int fourcc, const uint32_t w, const uint32_t h,
			enum util_fill_pattern pat, struct bo *out_bo[4], unsigned int *out_fb_id,
			uint64_t modifier, uint32_t *num_planes, drm_vs_bo_param bo_param[4])
{
	uint32_t handles[4] = { 0 }, pitches[4] = { 0 }, offsets[4] = { 0 };
	const struct util_format_info *info = NULL;
	uint64_t bo_modifiers[4] = { 0 };
	uint8_t i = 0;
	struct bo *bo[4];
	unsigned int fb_id;

	info = util_format_mod_info_find(fourcc, modifier);
	if (info == NULL) {
		printf("connector has no matched format info\n");
		return -1;
	}

	for (i = 0; i < info->num_planes; i++)
		bo_modifiers[i] = modifier;

	if (vs_bo_create(fd, fourcc, modifier, w, h, NULL, pat, bo_modifiers, bo, bo_param, 0)) {
		DTESTLOGE("Create bo fail");
		return -1;
	}

	for (i = 0; i < info->num_planes; i++) {
		handles[i] = bo[i]->handle;
		pitches[i] = bo[i]->pitch;
		offsets[i] = bo[i]->offset;
	}

	if (drmModeAddFB2WithModifiers(fd, w, h, fourcc, handles, pitches, offsets, bo_modifiers,
				       &fb_id, DRM_MODE_FB_MODIFIERS)) {
		fprintf(stderr, "failed to add fb (%ux%u): %s\n", w, h, strerror(errno));
		return -1;
	}

	for (i = 0; i < info->num_planes; i++)
		out_bo[i] = bo[i];

	*out_fb_id = fb_id;
	*num_planes = info->num_planes;
	return 0;
}

static void _adjust_mode_for_vrr(drmModeModeInfo *mode, uint16_t vrr_refresh)
{
	uint32_t new_vtotal = 0, new_vfp = 0, vsync_width = 0, vbp = 0;
	drmModeModeInfo old_mode = *mode;

	/* For debug: clock = v_total * h_total * refresh , not the real hardware situation */
	vsync_width = old_mode.vsync_end - old_mode.vsync_start;
	vbp = old_mode.vtotal - old_mode.vsync_end;
	new_vtotal = _DIV_ROUND_CLOSEST(old_mode.clock * 1000, old_mode.htotal * vrr_refresh);
	new_vfp = new_vtotal - old_mode.vdisplay - vsync_width - vbp;

	mode->vsync_start = mode->vdisplay + new_vfp;
	mode->vsync_end = mode->vsync_start + vsync_width;
	mode->vtotal = new_vtotal;

	return;
}

static void add_property(struct device *dev, uint32_t obj_id, const char *name, uint64_t value)
{
	struct property_arg p;

	p.obj_id = obj_id;
	if (sizeof(p.name) - 1 < strlen(name)) {
		fprintf(stderr, "property name overflow, %s", name);
		return;
	}
	strncpy(p.name, name, sizeof(p.name) - 1);
	p.value = value;

	set_property(dev, &p);
}

static drmModeConnector *get_connector_by_id(struct device *dev, uint32_t id)
{
	drmModeConnector *connector;
	int i;

	for (i = 0; i < dev->resources->res->count_connectors; i++) {
		connector = dev->resources->connectors[i].connector;
		if (connector && connector->connector_id == id)
			return connector;
	}

	return NULL;
}

static int writeback_config_no_buffer(struct device *dev, struct pipe_arg *pipes,
				      unsigned int count)
{
	drmModeConnector *connector;
	unsigned int i;

	for (i = 0; i < count; i++) {
		struct pipe_arg *pipe = &pipes[i];
		connector = get_connector_by_id(dev, pipe->con_ids[0]);
		if (connector->connector_type == DRM_MODE_CONNECTOR_WRITEBACK) {
			add_property(dev, pipe->con_ids[0], "WRITEBACK_FB_ID", pipe->out_fb_id);
		}
	}
	return 0;
}

static int stripe_writeback_config(struct device *dev, struct pipe_arg *pipe)
{
	/* Writeback stripe mode, just need create in the first time */
	unsigned int i;
	if (dev->wb_stripe.fb_id == 0) {
		if (bo_fb_create(dev->fd, pipe->fourcc, pipe->wb_w, pipe->wb_h, plain_fill,
				 pipe->out_bo, &pipe->out_fb_id, pipe->wb_modifier,
				 &pipe->num_planes, pipe->bo_param))
			return -1;

		dev->wb_stripe.fb_id = pipe->out_fb_id;
		dev->wb_stripe.num_planes = pipe->num_planes;
		dev->wb_stripe.wb_modifier = pipe->wb_modifier;
		dev->wb_stripe.fourcc = pipe->fourcc;
		for (i = 0; i < pipe->num_planes; i++) {
			dev->wb_stripe.out_bo[i] = pipe->out_bo[i];
			dev->wb_stripe.bo_param[i] = pipe->bo_param[i];
		}
	} else {
		pipe->out_fb_id = dev->wb_stripe.fb_id;
		pipe->num_planes = dev->wb_stripe.num_planes;
		pipe->wb_modifier = dev->wb_stripe.wb_modifier;
		pipe->fourcc = dev->wb_stripe.fourcc;
		for (i = 0; i < pipe->num_planes; i++) {
			pipe->out_bo[i] = dev->wb_stripe.out_bo[i];
			pipe->bo_param[i] = dev->wb_stripe.bo_param[i];
		}
	}

	add_property(dev, pipe->con_ids[0], "WRITEBACK_FB_ID", pipe->out_fb_id);

	return 0;
}

static int writeback_config(struct device *dev, struct pipe_arg *pipes, unsigned int count)
{
	drmModeConnector *connector;
	unsigned int i;

	for (i = 0; i < count; i++) {
		struct pipe_arg *pipe = &pipes[i];

		connector = get_connector_by_id(dev, pipe->con_ids[0]);
		if (!connector)
			return -1;
		if (connector->connector_type == DRM_MODE_CONNECTOR_WRITEBACK) {
			if (pipe->global_disable)
				add_property(dev, pipe->con_ids[0], "WRITEBACK_FB_ID", 0);
			else {
				if (pipe->wb_with_stripe) {
					if (stripe_writeback_config(dev, pipe))
						return -1;
				} else {
					if (bo_fb_create(dev->fd, pipes[i].fourcc, pipe->wb_w,
							 pipe->wb_h, plain_fill, pipe->out_bo,
							 &pipe->out_fb_id, pipe->wb_modifier,
							 &pipe->num_planes, pipe->bo_param))
						return -1;

					add_property(dev, pipe->con_ids[0], "WRITEBACK_FB_ID",
						     pipe->out_fb_id);
					if (pipe->wb_for_ramless)
						dev->ramless.ramless_fb_id = pipe->out_fb_id;
				}
			}
		}
	}
	return 0;
}

static int create_plane_buffer(struct device *dev, struct plane_arg *p, int pattern, bool update,
			       struct pipe_arg *pipe)
{
	uint32_t handles[4] = { 0 }, pitches[4] = { 0 }, offsets[4] = { 0 };
	/* VSI updated */
	uint64_t modifiers[4] = { 0 };

	struct bo *plane_bo[4] = { NULL, NULL, NULL, NULL };
	const struct util_format_info *info = NULL;
	int ret = 0;
	unsigned int i;
	unsigned int old_fb_id;

	info = util_format_mod_info_find(p->fourcc, p->modifier);
	if (info == NULL) {
		fprintf(stderr, "plane %u with format %u has no matched format info\n", p->plane_id,
			p->fourcc);
		return -1;
	}

	p->num_planes = info->num_planes;
	for (i = 0; i < p->num_planes; i++) {
		plane_bo[i] = p->old_bo[i];
		p->old_bo[i] = p->bo[i];
	}

	if (!plane_bo[0]) {
		ret = vs_bo_create(dev->fd, p->fourcc, p->modifier, p->w, p->h, p->file_src,
				   pattern, modifiers, plane_bo, NULL, dev->frame_id);

		if (ret) {
			fprintf(stderr, "failed to create bo for plane %u\n", p->plane_id);
			return -1;
		}

		for (i = 0; i < p->num_planes; i++) {
			handles[i] = plane_bo[i]->handle;
			pitches[i] = plane_bo[i]->pitch;
			offsets[i] = plane_bo[i]->offset;
		}

		if (drmModeAddFB2WithModifiers(dev->fd, p->w, p->h, p->fourcc, handles, pitches,
					       offsets, modifiers, &p->fb_id,
					       DRM_MODE_FB_MODIFIERS)) {
			fprintf(stderr, "failed to add fb: %s\n", strerror(errno));
			return -1;
		}
	}

	for (i = 0; i < p->num_planes; i++)
		p->bo[i] = plane_bo[i];
	/* updated end */

	old_fb_id = p->fb_id;
	p->old_fb_id = old_fb_id;

	return 0;
}

static int atomic_set_plane_no_buffer(struct device *dev, struct plane_arg *p, int pattern,
				      bool update)
{
	const struct util_format_info *info = NULL;
	/* updated end */
	int crtc_x, crtc_y, crtc_w, crtc_h;
	struct crtc *crtc = NULL;
	unsigned int i;

	if (p->global_disable) {
		add_property(dev, p->plane_id, "FB_ID", 0);
		add_property(dev, p->plane_id, "CRTC_ID", 0);
		printf("atomic_set_plane: set the plane %d disable.\n", p->plane_id);

		return 0;
	}

	/* Find an unused plane which can be connected to our CRTC. Find the
	 * CRTC index first, then iterate over available planes.
	 */
	for (i = 0; i < (unsigned int)dev->resources->res->count_crtcs; i++) {
		if (p->crtc_id == dev->resources->res->crtcs[i]) {
			crtc = &dev->resources->crtcs[i];
			break;
		}
	}

	if (!crtc) {
		fprintf(stderr, "CRTC %u not found\n", p->crtc_id);
		return -1;
	}

	if (!update)
		fprintf(stderr, "testing %dx%d@%s on plane %u, crtc %u\n", p->w, p->h,
			p->format_str, p->plane_id, p->crtc_id);

	/* VSI updated */
	info = util_format_mod_info_find(p->fourcc, p->modifier);
	if (info == NULL) {
		fprintf(stderr, "plane %u with format %u has no matched format info\n", p->plane_id,
			p->fourcc);
		return -1;
	}

	if (p->crtc_w == 0) {
		crtc_w = p->w * p->scale;
		crtc_h = p->h * p->scale;

		if (!p->has_position) {
			/* Default to the middle of the screen */
			crtc_x = (crtc->mode->hdisplay - crtc_w) / 2;
			crtc_y = (crtc->mode->vdisplay - crtc_h) / 2;
		} else {
			crtc_x = p->x;
			crtc_y = p->y;
		}
	} else {
		crtc_x = p->crtc_x;
		crtc_y = p->crtc_y;
		crtc_w = p->crtc_w;
		crtc_h = p->crtc_h;
	}

	add_property(dev, p->plane_id, "FB_ID", p->fb_id);
	add_property(dev, p->plane_id, "CRTC_ID", p->crtc_id);
	add_property(dev, p->plane_id, "SRC_X", 0);
	add_property(dev, p->plane_id, "SRC_Y", 0);
	add_property(dev, p->plane_id, "SRC_W", p->w << 16);
	add_property(dev, p->plane_id, "SRC_H", p->h << 16);
	add_property(dev, p->plane_id, "CRTC_X", crtc_x);
	add_property(dev, p->plane_id, "CRTC_Y", crtc_y);
	add_property(dev, p->plane_id, "CRTC_W", crtc_w);
	add_property(dev, p->plane_id, "CRTC_H", crtc_h);
	add_property(dev, p->plane_id, "rotation", p->rotation);
	add_property(dev, p->plane_id, "pixel blend mode", p->std_blend_mode);
	add_property(dev, p->plane_id, "alpha", p->std_blend_alpha);
	add_property(dev, p->plane_id, "zpos", p->zpos);

	if (!is_rgb(info->format)) {
		add_property(dev, p->plane_id, "COLOR_ENCODING", p->color_encoding);
		add_property(dev, p->plane_id, "COLOR_RANGE", p->color_range);
	}

	return 0;
}

static int atomic_set_plane(struct device *dev, struct plane_arg *p, int pattern, bool update)
{
	uint32_t handles[4] = { 0, 0, 0, 0 };
	uint32_t pitches[4] = { 0, 0, 0, 0 };
	uint32_t offsets[4] = { 0, 0, 0, 0 };
	/* VSI updated */
	uint64_t modifiers[4] = { 0, 0, 0, 0 };
	struct bo *plane_bo[4] = { NULL, NULL, NULL, NULL };
	const struct util_format_info *info = NULL;
	int ret = 0;
	/* updated end */
	int crtc_x, crtc_y, crtc_w, crtc_h;
	struct crtc *crtc = NULL;
	unsigned int i;
	unsigned int old_fb_id;

	if (p->global_disable) {
		add_property(dev, p->plane_id, "FB_ID", 0);
		add_property(dev, p->plane_id, "CRTC_ID", 0);
		printf("atomic_set_plane: set the plane %d disable.\n", p->plane_id);

		return 0;
	}

	/* Find an unused plane which can be connected to our CRTC. Find the
	 * CRTC index first, then iterate over available planes.
	 */
	for (i = 0; i < (unsigned int)dev->resources->res->count_crtcs; i++) {
		if (p->crtc_id == dev->resources->res->crtcs[i]) {
			crtc = &dev->resources->crtcs[i];
			break;
		}
	}

	if (!crtc) {
		fprintf(stderr, "CRTC %u not found\n", p->crtc_id);
		return -1;
	}

	if (!update)
		fprintf(stderr, "testing %dx%d@%s on plane %u, crtc %u\n", p->w, p->h,
			p->format_str, p->plane_id, p->crtc_id);

	/* VSI updated */
	info = util_format_mod_info_find(p->fourcc, p->modifier);
	if (info == NULL) {
		fprintf(stderr, "plane %u with format %u has no matched format info\n", p->plane_id,
			p->fourcc);
		return -1;
	}

	p->num_planes = info->num_planes;
	for (i = 0; i < p->num_planes; i++) {
		plane_bo[i] = p->old_bo[i];
		p->old_bo[i] = p->bo[i];
	}

	if (!plane_bo[0]) {
		ret = vs_bo_create(dev->fd, p->fourcc, p->modifier, p->w, p->h, p->file_src,
				   pattern, modifiers, plane_bo, NULL, dev->frame_id);

		if (ret) {
			fprintf(stderr, "failed to create bo for plane %u\n", p->plane_id);
			return -1;
		}

		for (i = 0; i < p->num_planes; i++) {
			handles[i] = plane_bo[i]->handle;
			pitches[i] = plane_bo[i]->pitch;
			offsets[i] = plane_bo[i]->offset;
		}

		if (drmModeAddFB2WithModifiers(dev->fd, p->w, p->h, p->fourcc, handles, pitches,
					       offsets, modifiers, &p->fb_id,
					       DRM_MODE_FB_MODIFIERS)) {
			fprintf(stderr, "failed to add fb: %s\n", strerror(errno));
			return -1;
		}
	}

	for (i = 0; i < p->num_planes; i++)
		p->bo[i] = plane_bo[i];
	/* updated end */

	old_fb_id = p->fb_id;
	p->old_fb_id = old_fb_id;

	if (p->crtc_w == 0) {
		crtc_w = p->w * p->scale;
		crtc_h = p->h * p->scale;

		if (!p->has_position) {
			/* Default to the middle of the screen */
			crtc_x = (crtc->mode->hdisplay - crtc_w) / 2;
			crtc_y = (crtc->mode->vdisplay - crtc_h) / 2;
		} else {
			crtc_x = p->x;
			crtc_y = p->y;
		}
	} else {
		crtc_x = p->crtc_x;
		crtc_y = p->crtc_y;
		crtc_w = p->crtc_w;
		crtc_h = p->crtc_h;
	}

	add_property(dev, p->plane_id, "FB_ID", p->fb_id);
	add_property(dev, p->plane_id, "CRTC_ID", p->crtc_id);
	add_property(dev, p->plane_id, "SRC_X", 0);
	add_property(dev, p->plane_id, "SRC_Y", 0);
	add_property(dev, p->plane_id, "SRC_W", p->w << 16);
	add_property(dev, p->plane_id, "SRC_H", p->h << 16);
	add_property(dev, p->plane_id, "CRTC_X", crtc_x);
	add_property(dev, p->plane_id, "CRTC_Y", crtc_y);
	add_property(dev, p->plane_id, "CRTC_W", crtc_w);
	add_property(dev, p->plane_id, "CRTC_H", crtc_h);
	add_property(dev, p->plane_id, "rotation", p->rotation);
	add_property(dev, p->plane_id, "pixel blend mode", p->std_blend_mode);
	add_property(dev, p->plane_id, "alpha", p->std_blend_alpha);
	add_property(dev, p->plane_id, "zpos", p->zpos);

	if (!is_rgb(info->format)) {
		add_property(dev, p->plane_id, "COLOR_ENCODING", p->color_encoding);
		add_property(dev, p->plane_id, "COLOR_RANGE", p->color_range);
	}

	return 0;
}

static int atomic_set_planes(struct device *dev, struct plane_arg *p, unsigned int count,
			     bool update)
{
	unsigned int i, pattern = primary_fill;

	/* set up planes */
	for (i = 0; i < count; i++) {
		if (!dev->use_vcmd) {
			if (atomic_set_plane(dev, &p[i], pattern, update) != 0)
				return -1;
		} else {
			if (atomic_set_plane_no_buffer(dev, &p[i], pattern, update) != 0)
				return -1;
		}
	}
	return 0;
}

uint64_t get_crtc_property_default_value(device_t *dev, uint32_t crtc_id, char *props_name)
{
	struct crtc *crtc = NULL;
	drmModeObjectProperties *props = NULL;
	drmModePropertyRes **props_info = NULL;
	uint32_t i, j;

	for (i = 0; i < (int)dev->resources->res->count_crtcs; ++i) {
		crtc = &dev->resources->crtcs[i];
		props = crtc->props;
		props_info = crtc->props_info;

		if (crtc->crtc->crtc_id != crtc_id)
			continue;

		for (j = 0; j < (int)props->count_props; ++j) {
			if (!props_info[j])
				continue;
			if (strcmp(props_info[j]->name, props_name) == 0)
				return props->prop_values[j];
		}
	}

	fprintf(stderr, "failed to get crtc feature '%s' init value\n", props_name);

	return 0;
}

static int atomic_set_cursors(struct device *dev, struct cursor_arg *cursors, unsigned int count)
{
	unsigned int i;
	struct cursor_arg *cursor;
	uint32_t handles[4] = { 0 }, pitches[4] = { 0 }, offsets[4] = { 0 };
	struct bo *bo;

	/* set up cursors */
	for (i = 0; i < count; i++) {
		cursor = &cursors[i];
		/* create cursor bo just using PATTERN_PLAIN as it has
		 * translucent alpha
		 */
		bo = bo_create(dev->fd, cursor->fourcc, cursor->w, cursor->h, handles, pitches,
			       offsets, UTIL_PATTERN_SMPTE, 0);
		if (bo == NULL)
			return -1;
		else
			cursor->bo[0] = bo;

		if (drmModeSetCursor2(dev->fd, cursor->crtc_id, handles[0], cursor->w, cursor->h,
				      cursor->hot_x, cursor->hot_y)) {
			fprintf(stderr, "failed to set cursor: %s\n", strerror(errno));
			return -1;
		}

		if (drmModeMoveCursor(dev->fd, cursor->crtc_id, cursor->x, cursor->y)) {
			fprintf(stderr, "failed to move cursor: %s\n", strerror(errno));
			return -1;
		}
	}

	return 0;
}

static int leagcy_disable_cursors(struct device *dev, struct cursor_arg *cursors,
				  unsigned int count)
{
	unsigned int i;
	struct cursor_arg *cursor;

	/* set up cursors */
	for (i = 0; i < count; i++) {
		cursor = &cursors[i];

		if (drmModeSetCursor2(dev->fd, cursor->crtc_id, 0, cursor->w, cursor->h,
				      cursor->hot_x, cursor->hot_y)) {
			fprintf(stderr, "failed to set cursor: %s\n", strerror(errno));
			return -1;
		}
	}

	return 0;
}

uint64_t get_crtc_property_init_value(device_t *dev, char *props_name)
{
	struct crtc *crtc = NULL;
	drmModeObjectProperties *props = NULL;
	drmModePropertyRes **props_info = NULL;
	uint32_t i, j;

	for (i = 0; i < (int)dev->resources->res->count_crtcs; ++i) {
		crtc = &dev->resources->crtcs[i];
		props = crtc->props;
		props_info = crtc->props_info;

		for (j = 0; j < (int)props->count_props; ++j) {
			if (!props_info[j])
				continue;
			if (strcmp(props_info[j]->name, props_name) == 0)
				return props->prop_values[j];
		}
	}

	fprintf(stderr, "failed to get crtc feature '%s' init value\n", props_name);

	return 0;
}

uint64_t get_plane_property_default_value(device_t *dev, uint32_t plane_id, char *props_name)
{
	struct plane *plane = NULL;
	drmModeObjectProperties *props = NULL;
	drmModePropertyRes **props_info = NULL;
	uint32_t i, j;

	for (i = 0; i < (int)dev->resources->plane_res->count_planes; ++i) {
		plane = &dev->resources->planes[i];
		props = plane->props;
		props_info = plane->props_info;

		if (plane->plane->plane_id != plane_id)
			continue;

		for (j = 0; j < (int)props->count_props; ++j) {
			if (!props_info[j])
				continue;
			if (strcmp(props_info[j]->name, props_name) == 0)
				return props->prop_values[j];
		}
	}

	fprintf(stderr, "failed to get plane %u feature '%s' defaule value\n", plane_id,
		props_name);

	return 0;
}

uint64_t get_connector_property_default_value(device_t *dev, uint32_t connector_id,
					      char *props_name)
{
	struct connector *connector = NULL;
	drmModeObjectProperties *props = NULL;
	drmModePropertyRes **props_info = NULL;
	uint32_t i, j;

	for (i = 0; i < (int)dev->resources->res->count_connectors; ++i) {
		connector = &dev->resources->connectors[i];
		props = connector->props;
		props_info = connector->props_info;

		if (connector->connector->connector_id != connector_id)
			continue;

		for (j = 0; j < (int)props->count_props; ++j) {
			if (!props_info[j])
				continue;
			if (strcmp(props_info[j]->name, props_name) == 0)
				return props->prop_values[j];
		}
	}

	fprintf(stderr, "failed to get connector %u feature '%s' defaule value\n", connector_id,
		props_name);

	return 0;
}

static drmModeEncoder *get_encoder_by_id(struct device *dev, uint32_t id)
{
	drmModeEncoder *encoder;
	int i;

	for (i = 0; i < dev->resources->res->count_encoders; i++) {
		encoder = dev->resources->encoders[i].encoder;
		if (encoder && encoder->encoder_id == id)
			return encoder;
	}

	return NULL;
}

static int get_connector_index(struct device *dev, uint32_t id)
{
	int i;

	for (i = 0; i < dev->resources->res->count_connectors; ++i) {
		drmModeConnector *conn = dev->resources->connectors[i].connector;
		if (conn && conn->connector_id == id)
			return i;
	}

	return -1;
}

static int get_crtc_index(struct device *dev, uint32_t id)
{
	int i;

	for (i = 0; i < dev->resources->res->count_crtcs; ++i) {
		drmModeCrtc *crtc = dev->resources->crtcs[i].crtc;
		if (crtc && crtc->crtc_id == id)
			return i;
	}

	return -1;
}

static struct crtc *pipe_find_crtc(struct device *dev, struct pipe_arg *pipe)
{
	uint32_t possible_crtcs = ~0;
	uint32_t active_crtcs = 0;
	unsigned int crtc_idx;
	unsigned int i;
	int j;

	for (i = 0; i < pipe->num_cons; ++i) {
		uint32_t crtcs_for_connector = 0;
		drmModeConnector *connector;
		drmModeEncoder *encoder;
		int idx;

		connector = get_connector_by_id(dev, pipe->con_ids[i]);
		if (!connector)
			return NULL;

		for (j = 0; j < connector->count_encoders; ++j) {
			encoder = get_encoder_by_id(dev, connector->encoders[j]);
			if (!encoder)
				continue;

			crtcs_for_connector |= encoder->possible_crtcs;

			idx = get_crtc_index(dev, encoder->crtc_id);
			if (idx >= 0)
				active_crtcs |= 1 << idx;
		}

		possible_crtcs &= crtcs_for_connector;
	}

	if (!possible_crtcs)
		return NULL;

	/* Return the first possible and active CRTC if one exists, or the first
	 * possible CRTC otherwise.
	 */
	if (possible_crtcs & active_crtcs)
		crtc_idx = ffs(possible_crtcs & active_crtcs);
	else
		crtc_idx = ffs(possible_crtcs);

	return &dev->resources->crtcs[crtc_idx - 1];
}

static drmModeModeInfo *connector_find_mode(struct device *dev, uint32_t con_id,
					    const char *mode_str, const unsigned int vrefresh)
{
	drmModeConnector *connector;
	drmModeModeInfo *mode = NULL;
	int i;
	uint8_t cap_any_resolution = 0;
	uint16_t max_width = 0, max_height = 0;
	unsigned int in_w = 0, in_h = 0;
	unsigned int j = 0;

	cap_any_resolution = dev->resources->crtcs->info.any_resolution;

	connector = get_connector_by_id(dev, con_id);
	if (!connector || !connector->count_modes)
		return NULL;

	for (i = 0; i < connector->count_modes; i++) {
		mode = &connector->modes[i];

		if (cap_any_resolution) {
			if (mode->htotal >= max_width && mode->vtotal >= max_height) {
				max_width = mode->htotal;
				max_height = mode->vtotal;
				j = i;
			}
		}

		if (!strcmp(mode->name, mode_str)) {
			/* If the vertical refresh frequency is not specified then return the
			 * first mode that match with the name. Else, return the mode that match
			 * the name and the specified vertical refresh frequency.
			 */
			if (vrefresh == 0)
				return mode;
			else if (mode->vrefresh == vrefresh)
				return mode;
		}
	}

	if (cap_any_resolution) {
		sscanf(mode_str, "%ux%u", &in_w, &in_h);
		if (in_w <= max_width && in_h <= max_height)
			mode = &connector->modes[j];
		return mode;
	}

	return NULL;
}

static int pipe_find_crtc_and_mode(struct device *dev, struct pipe_arg *pipe)
{
	drmModeModeInfo *mode = NULL;
	int i;

	pipe->mode = NULL;

	for (i = 0; i < (int)pipe->num_cons; i++) {
		if (pipe->global_disable)
			continue;

		mode = connector_find_mode(dev, pipe->con_ids[i], pipe->mode_str, pipe->vrefresh);
		if (mode == NULL) {
			fprintf(stderr, "failed to find mode \"%s\" for connector %s\n",
				pipe->mode_str, pipe->cons[i]);
			return -EINVAL;
		}
	}

	/* If the CRTC ID was specified, get the corresponding CRTC. Otherwise
	 * locate a CRTC that can be attached to all the connectors.
	 */
	if (pipe->crtc_id != (uint32_t)-1) {
		for (i = 0; i < dev->resources->res->count_crtcs; i++) {
			struct crtc *crtc = &dev->resources->crtcs[i];

			if (pipe->crtc_id == crtc->crtc->crtc_id) {
				pipe->crtc = crtc;
				break;
			}
		}
	} else {
		pipe->crtc = pipe_find_crtc(dev, pipe);
	}

	if (!pipe->crtc) {
		fprintf(stderr, "failed to find CRTC for pipe\n");
		return -EINVAL;
	}

	pipe->mode = mode;
	pipe->crtc->mode = mode;

	return 0;
}

static int atomic_set_mode(struct device *dev, struct pipe_arg *pipes, unsigned int count)
{
	unsigned int i;
	unsigned int j;
	int ret = 0;

	for (i = 0; i < count; i++) {
		struct pipe_arg *pipe = &pipes[i];

		ret = pipe_find_crtc_and_mode(dev, pipe);
		if (ret < 0)
			continue;
	}
	if (ret < 0) {
		return -1;
	}
	for (i = 0; i < count; i++) {
		struct pipe_arg *pipe = &pipes[i];
		uint32_t blob_id;

		if (pipe->global_disable) {
			/* Disable the CRTC */
			for (j = 0; j < pipe->num_cons; ++j)
				add_property(dev, pipe->con_ids[j], "CRTC_ID", 0);

			add_property(dev, pipe->crtc->crtc->crtc_id, "MODE_ID", 0);
			add_property(dev, pipe->crtc->crtc->crtc_id, "ACTIVE", 0);
			printf("atomic_set_mode: set the conntor[%d]--crtc[%d] disable.\n",
			       pipe->con_ids[0], pipe->crtc->crtc->crtc_id);
		} else {
			/* If the VRR enabled, need to calcuate the mode info(will change the vfp value) by the VRR refresh setted */
			if (pipe->vrr_enable) {
				_adjust_mode_for_vrr(pipe->mode, pipe->vrr_refresh);
				add_property(dev, pipe->crtc->crtc->crtc_id, "VRR_ENABLED", 1);
			} else
				add_property(dev, pipe->crtc->crtc->crtc_id, "VRR_ENABLED", 0);

			printf("setting mode %s-%dHz on connectors ", pipe->mode_str,
			       pipe->mode->vrefresh);
			for (j = 0; j < pipe->num_cons; ++j) {
				printf("%s, ", pipe->cons[j]);
				add_property(dev, pipe->con_ids[j], "CRTC_ID",
					     pipe->crtc->crtc->crtc_id);
			}
			printf("crtc %d\n", pipe->crtc->crtc->crtc_id);

			drmModeCreatePropertyBlob(dev->fd, pipe->mode, sizeof(*pipe->mode),
						  &blob_id);
			add_property(dev, pipe->crtc->crtc->crtc_id, "MODE_ID", blob_id);
			add_property(dev, pipe->crtc->crtc->crtc_id, "ACTIVE", 1);
		}
	}
	return 0;
}

static drmModeConnector *get_connector_by_name(struct device *dev, const char *name)
{
	struct connector *connector;
	int i;

	for (i = 0; i < dev->resources->res->count_connectors; i++) {
		connector = &dev->resources->connectors[i];

		if (strcmp(connector->name, name) == 0)
			return connector->connector;
	}

	return NULL;
}

static int pipe_resolve_connectors(struct device *dev, struct pipe_arg *pipe)
{
	drmModeConnector *connector;
	unsigned int i;
	uint32_t id;
	char *endp;

	for (i = 0; i < pipe->num_cons; i++) {
		id = strtoul(pipe->cons[i], &endp, 10);
		if (endp == pipe->cons[i]) {
			connector = get_connector_by_name(dev, pipe->cons[i]);
			if (!connector) {
				fprintf(stderr, "no connector named '%s'\n", pipe->cons[i]);
				return -ENODEV;
			}

			id = connector->connector_id;
		}

		pipe->con_ids[i] = id;
	}

	return 0;
}

dtest_status dpu_create_wb_plane_buffer(struct device *dev, frame_t *frame)
{
	struct pipe_arg *pipes = frame->pipe.pipes;
	uint32_t count = frame->pipe.num;
	drmModeConnector *connector;
	uint32_t i, j, pattern = primary_fill;
	dtest_status status;
	dtest_plane_t *dtest_planes = &frame->plane;
	dtest_pipe_t *dtest_pipes = &frame->pipe;
	int32_t plane_id = 0, crtc_id = 0, connector_id = 0, encoder_id = 0;
	drmModeEncoder *encoder;
	bool crtc_id_match = true;
	int32_t pipe_format;
	struct pipe_arg *pipe = NULL;

	if (dtest_pipes->num == 0 || dtest_planes->num == 0) {
		DTESTLOGE("Invalid args, please set pipes or planes.");
		return DTEST_FAILURE;
	}

	for (i = 0; i < dtest_planes->num; i++) {
		plane_id = dtest_planes->planes[i].plane_id;
		drmModePlane *modeplane = dev->resources->planes[plane_id].plane;
		crtc_id = dtest_planes->planes[i].crtc_id;

		if (plane_id >= dev->resources->plane_res->count_planes) {
			DTESTLOGE("The plane %i is Unsupport", plane_id);
			return DTEST_NOT_SUPPORT;
		}

		if (!(1 << crtc_id & modeplane->possible_crtcs)) {
			DTESTLOGE("The plane %i is Unsupport crtc %i\n", plane_id, crtc_id);
			return DTEST_NOT_SUPPORT;
		}
	}

	for (i = 0; i < dtest_pipes->num; i++) {
		connector_id = dtest_pipes->pipes[i].con_ids[0];
		crtc_id = dtest_pipes->pipes[i].crtc_id;

		if (connector_id >= dev->resources->res->count_connectors) {
			DTESTLOGE("The connector %i is Unsupport", connector_id);
			return DTEST_NOT_SUPPORT;
		}

		if (crtc_id >= dev->resources->res->count_crtcs) {
			DTESTLOGE("The crtc %i is Unsupport", dtest_pipes->pipes[i].crtc_id);
			return DTEST_NOT_SUPPORT;
		}

		encoder_id = dev->resources->connectors[connector_id].connector->encoder_id;

		if (encoder_id == 0)
			continue;

		encoder = get_encoder_by_id(dev, encoder_id);
		if (!encoder)
			return DTEST_NOT_SUPPORT;
		crtc_id_match = !!(encoder->possible_crtcs & (1 << crtc_id));

		if (!crtc_id_match) {
			DTESTLOGE("The crtc %i is Unsupport connector %i ", crtc_id, connector_id);
			return DTEST_NOT_SUPPORT;
		}

		if (encoder->encoder_type == DRM_MODE_ENCODER_DPMST) {
			pipe_format = dtest_pipes->pipes[i].fourcc;
			/* for DP encoder, pipe_format should be XRGB16161616F (DP_RGB) or Y210 (DP_YUV) */
			if (!(pipe_format == DRM_FORMAT_XRGB16161616F ||
			      pipe_format == DRM_FORMAT_Y210)) {
				DTESTLOGE("The connector %i has Unsupport pipe format",
					  connector_id);
				return DTEST_NOT_SUPPORT;
			}
		}

		if (pipe_resolve_connectors(dev, &dtest_pipes->pipes[i]) < 0) {
			DTESTLOGE("pipe_resolve_connectors fail");
			return DTEST_FAILURE;
		}
	}

	status = dtest_remap_obj_id(dev, frame);
	DTEST_CHECK_STATUS(status, error);

	/*set writeback buffer*/
	for (i = 0; i < count; i++) {
		struct pipe_arg *pipe = &pipes[i];
		connector = get_connector_by_id(dev, pipe->con_ids[0]);
		if (!connector)
			return DTEST_NOT_SUPPORT;
		if (connector->connector_type == DRM_MODE_CONNECTOR_WRITEBACK) {
			if (pipe->global_disable)
				pipe->out_fb_id = 0;
			else {
				if (bo_fb_create(dev->fd, pipes[i].fourcc, pipe->wb_w, pipe->wb_h,
						 plain_fill, pipe->out_bo, &pipe->out_fb_id,
						 pipe->wb_modifier, &pipe->num_planes,
						 pipe->bo_param))
					return DTEST_FAILURE;
			}
		}
	}

	/* set up planes */
	dev->frame_id = frame->frame_id;
	for (i = 0; i < count; i++) {
		for (j = 0; j < dtest_pipes->num; j++) {
			if (dtest_planes->planes[i].crtc_id == dtest_pipes->pipes[j].crtc_id) {
				pipe = &dtest_pipes->pipes[j];
				break;
			}
		}
		if (create_plane_buffer(dev, &dtest_planes->planes[i], pattern, false, pipe) != 0)
			return DTEST_FAILURE;
	}

	return status;
error:
	return DTEST_FAILURE;
}

static bool check_in_formats(struct device *dev, uint32_t blob_id, uint32_t plane_format,
			     uint64_t plane_modifier)
{
	uint32_t i, j;
	drmModePropertyBlobPtr blob;
	struct drm_format_modifier_blob *header;
	uint32_t *formats;
	struct drm_format_modifier *modifiers;

	blob = drmModeGetPropertyBlob(dev->fd, blob_id);
	if (!blob) {
		printf("drmModeGetPropertyBlob fail.\n");
		return false;
	}

	header = blob->data;
	if (!header) {
		printf("blob->data is NULL.\n");
		drmModeFreePropertyBlob(blob);
		return false;
	}
	formats = (uint32_t *)((char *)header + header->formats_offset);
	modifiers = (struct drm_format_modifier *)((char *)header + header->modifiers_offset);

	for (i = 0; i < header->count_formats; i++) {
		for (j = 0; j < header->count_modifiers; j++) {
			uint64_t mask = 1ULL << i;
			if ((formats[i] == plane_format) &&
			    (modifiers[j].modifier == plane_modifier) &&
			    (modifiers[j].formats & mask)) {
				drmModeFreePropertyBlob(blob);
				return true;
			}
		}
	}

	drmModeFreePropertyBlob(blob);
	return false;
}

static bool plane_format_mod_supported(device_t *dev, uint32_t plane_id, uint32_t plane_format,
				       uint64_t plane_modifier)
{
	struct plane *plane = NULL;
	drmModeObjectProperties *props = NULL;
	drmModePropertyRes **props_info = NULL;
	const char *obj_type = "plane";
	uint32_t i;
	uint64_t fc_mask = 0;

	for (i = 0; i < (int)dev->resources->plane_res->count_planes; ++i) {
		plane = &dev->resources->planes[i];
		if (plane->plane->plane_id == plane_id) {
			props = plane->props;
			props_info = plane->props_info;
			break;
		}
	}

	if (!plane) {
		fprintf(stderr, "no plane found.\n");
		return false;
	}

	if (!props) {
		fprintf(stderr, "%s %i has no properties\n", obj_type, plane->plane->plane_id);
		return false;
	}

	for (i = 0; i < (int)props->count_props; ++i) {
		if (!props_info[i])
			continue;
		if (strcmp(props_info[i]->name, "IN_FORMATS") == 0) {
			fc_mask |= (uint64_t)DRM_FORMAT_MOD_VS_DEC_FC_MASK |
				   (uint64_t)DRM_FORMAT_MOD_VS_DEC_FC_SIZE_MASK;
			plane_modifier &= ~fc_mask;
			return check_in_formats(dev, props->prop_values[i], plane_format,
						plane_modifier);
		}
	}
	return false;
}

dtest_status dpu_set_frame(device_t *dev, frame_t *frame)
{
	int32_t i, ret;
	dtest_status status;
	dtest_property_t *dtest_properties = &frame->property;
	dtest_plane_t *dtest_planes = &frame->plane;
	dtest_cursor_t *dtest_cursors = &frame->cursor;
	dtest_pipe_t *dtest_pipes = &frame->pipe;
	int32_t plane_id = 0, crtc_id = 0, connector_id = 0, encoder_id = 0;
	int32_t cap_fbc = 0, cursor_w = 0, cursor_h = 0, cap_rotation = 0;
	uint32_t cap_possiable_crtc = 0;
	uint32_t valid_rotation = 0;
	uint64_t cap = 0;
	drmModeEncoder *encoder;
	bool crtc_id_match = true;
	int32_t max_blend_layer = 0, pipe_format;
	uint32_t maped_connector_id = 0;

	/* Get the DC info */
	cap_fbc = dev->resources->crtcs->info.cap_dec;
	max_blend_layer = dev->resources->crtcs->info.max_blend_layer;
	cap_rotation = dev->resources->crtcs->info.linear_yuv_rotation;

	if (!dev->use_vcmd) {
		if (dtest_pipes->num == 0 || dtest_planes->num == 0) {
			DTESTLOGE("Invalid args, please set pipes or planes.");
			return DTEST_FAILURE;
		}

		for (i = 0; i < dtest_planes->num; i++) {
			plane_id = dtest_planes->planes[i].plane_id_ori;
			dtest_planes->planes[i].plane_id = plane_id;

			drmModePlane *modeplane = dev->resources->planes[plane_id].plane;
			dtest_planes->planes[i].crtc_id = dtest_planes->planes[i].crtc_id_ori;
			crtc_id = dtest_planes->planes[i].crtc_id;

			if (plane_id >= dev->resources->plane_res->count_planes) {
				DTESTLOGE("The plane %i is Unsupport", plane_id);
				return DTEST_NOT_SUPPORT;
			}

			if (!(1 << crtc_id & modeplane->possible_crtcs)) {
				DTESTLOGE("The plane %i is Unsupport crtc %i\n", plane_id, crtc_id);
				return DTEST_NOT_SUPPORT;
			}
		}

		for (i = 0; i < dtest_pipes->num; i++) {
			dtest_pipes->pipes[i].con_ids[0] = dtest_pipes->pipes[i].con_ids_ori[0];
			connector_id = dtest_pipes->pipes[i].con_ids[0];
			dtest_pipes->pipes[i].crtc_id = dtest_pipes->pipes[i].crtc_id_ori;
			crtc_id = dtest_pipes->pipes[i].crtc_id;

			if (connector_id >= dev->resources->res->count_connectors) {
				DTESTLOGE("The connector %i is Unsupport", connector_id);
				return DTEST_NOT_SUPPORT;
			}

			if (crtc_id >= dev->resources->res->count_crtcs) {
				DTESTLOGE("The crtc %i is Unsupport",
					  dtest_pipes->pipes[i].crtc_id);
				return DTEST_NOT_SUPPORT;
			}

			maped_connector_id = connector_id;
			status = __remap_connector(dev, &maped_connector_id);
			DTEST_CHECK_STATUS(status, error);

			if (dtest_get_connector_possible_crtcs(dev, maped_connector_id,
							       &cap_possiable_crtc)) {
				DTESTLOGE("failed to get connector %d info", maped_connector_id);
				return DTEST_FAILURE;
			}

			crtc_id_match = !!(cap_possiable_crtc & (1 << crtc_id));
			if (!crtc_id_match) {
				DTESTLOGE("The crtc %i is Unsupport connector %i ", crtc_id,
					  connector_id);
				return DTEST_NOT_SUPPORT;
			}

			encoder_id = dev->resources->connectors[connector_id].connector->encoder_id;
			if (encoder_id == 0)
				continue;

			encoder = get_encoder_by_id(dev, encoder_id);
			if (!encoder) {
				DTESTLOGE("failed to get encoder by id: %d", encoder_id);
				return DTEST_FAILURE;
			}

			if (encoder->encoder_type == DRM_MODE_ENCODER_DPMST) {
				pipe_format = dtest_pipes->pipes[i].fourcc;
				/* for DP encoder, pipe_format should be XRGB16161616F (DP_RGB) or Y210 (DP_YUV) */
				if (!(pipe_format == DRM_FORMAT_XRGB16161616F ||
				      pipe_format == DRM_FORMAT_Y210)) {
					DTESTLOGE("The connector %i has Unsupport pipe format",
						  connector_id);
					return DTEST_NOT_SUPPORT;
				}
			}

			if (pipe_resolve_connectors(dev, &dtest_pipes->pipes[i]) < 0) {
				DTESTLOGE("pipe_resolve_connectors fail");
				return DTEST_FAILURE;
			}
		}

		status = dtest_remap_obj_id(dev, frame);
		DTEST_CHECK_STATUS(status, error);
	}

	dev->req = drmModeAtomicAlloc();
	DTEST_CHECK_PTR(dev->req, error);

	/*the ltm soft-algo need frame id */
	dev->frame_id = frame->frame_id;
	/*the sr2000 need layer width and height*/
	dev->mode.width = frame->plane.planes->w;
	dev->mode.height = frame->plane.planes->h;
	status = dtest_case_props_map(dev, dtest_properties);
	if (status == DTEST_NOT_SUPPORT) {
		DTESTLOGE("The feature %s is Unsupport", dtest_properties->properties->name);
		return DTEST_NOT_SUPPORT;
	}
	DTEST_CHECK_STATUS(status, error);

	for (i = 0; i < dtest_planes->num; i++) {
		if (dtest_planes->planes[i].global_disable)
			continue;

		if (!cap_fbc && (fourcc_mod_vs_get_type(dtest_planes->planes[i].modifier) !=
				 DRM_FORMAT_MOD_VS_TYPE_NORMAL)) {
			DTESTLOGE("FBC is Unsupport");
			return DTEST_NOT_SUPPORT;
		}

		/* check PVRIC support */
		if (fourcc_mod_vs_get_type(dtest_planes->planes[i].modifier) ==
		    DRM_FORMAT_MOD_VS_TYPE_PVRIC) {
			if (!(cap_fbc & (1 << DRM_FORMAT_MOD_VS_TYPE_PVRIC))) {
				DTESTLOGE("PVRIC is Unsupport");
				return DTEST_NOT_SUPPORT;
			}
		}

		if (!plane_format_mod_supported(dev, dtest_planes->planes[i].plane_id,
						dtest_planes->planes[i].fourcc,
						dtest_planes->planes[i].modifier)) {
			DTESTLOGE("This plane format: %c%c%c%c, modifier: 0x%lx is Unsupport",
				  dtest_planes->planes[i].fourcc,
				  dtest_planes->planes[i].fourcc >> 8,
				  dtest_planes->planes[i].fourcc >> 16,
				  dtest_planes->planes[i].fourcc >> 24,
				  dtest_planes->planes[i].modifier);
			return DTEST_NOT_SUPPORT;
		}

		if (dtest_planes->planes[i].zpos >= max_blend_layer) {
			DTESTLOGE("zpos id %d is Unsupport, max is %d",
				  dtest_planes->planes[i].zpos, max_blend_layer - 1);
			return DTEST_NOT_SUPPORT;
		}

		if (!cap_rotation && !is_rgb(dtest_planes->planes[i].fourcc) &&
		    (dtest_planes->planes[i].rotation == DRM_MODE_ROTATE_90 ||
		     dtest_planes->planes[i].rotation == DRM_MODE_ROTATE_270) &&
		    (fourcc_mod_vs_get_tile_mode(dtest_planes->planes[i].modifier) ==
			     DRM_FORMAT_MOD_VS_LINEAR ||
		     fourcc_mod_vs_get_tile_mode(dtest_planes->planes[i].modifier) ==
			     DRM_FORMAT_MOD_VS_DEC_LINEAR)) {
			DTESTLOGE("linear yuv rotation is Unsupport");
			return DTEST_NOT_SUPPORT;
		}

		if (dtest_planes->planes[i].rotation != DRM_MODE_ROTATE_0) {
			if (dtest_get_plane_valid_rot(dev, dtest_planes->planes[i].plane_id,
						      &valid_rotation)) {
				DTESTLOGE("failed to get plane %d info",
					  dtest_planes->planes[i].plane_id);
				return DTEST_FAILURE;
			}

			if (dtest_planes->planes[i].rotation & ~valid_rotation) {
				DTESTLOGE("plane %d not support rotation: 0x%x, valid bit: 0x%x",
					  dtest_planes->planes[i].plane_id,
					  dtest_planes->planes[i].rotation, valid_rotation);
				return DTEST_NOT_SUPPORT;
			}
		}
	}

	for (i = 0; i < dtest_cursors->num; i++) {
		ret = dtest_get_feature_cap(dev->fd, VS_FEATURE_CAP_CURSOR_WIDTH, &cursor_w) &
		      dtest_get_feature_cap(dev->fd, VS_FEATURE_CAP_CURSOR_HEIGHT, &cursor_h);
		if (ret || cursor_w < dtest_cursors->cursors[i].w ||
		    cursor_h < dtest_cursors->cursors[i].h) {
			DTESTLOGE("CURSOR  %d x %d is Unsupport ", dtest_cursors->cursors[i].w,
				  dtest_cursors->cursors[i].h);
			return DTEST_NOT_SUPPORT;
		}
	}

	ret = drmGetCap(dev->fd, DRM_CAP_DUMB_BUFFER, &cap);
	if (ret || cap == 0) {
		DTESTLOGE("driver doesn't support the dumb buffer API");
		return DTEST_FAILURE;
	}

	ret = atomic_set_mode(dev, dtest_pipes->pipes, dtest_pipes->num);
	if (ret) {
		DTESTLOGE("timing mode is Unsupport");
		return DTEST_NOT_SUPPORT;
	}

	if (dev->use_vdp) {
		if (vs_vdp_config(dev, dtest_pipes->pipes, dtest_pipes->num))
			return DTEST_FAILURE;
	}

	if (!dev->use_vcmd) {
		if (writeback_config(dev, dtest_pipes->pipes, dtest_pipes->num)) {
			return DTEST_FAILURE;
		}
	} else {
		if (writeback_config_no_buffer(dev, dtest_pipes->pipes, dtest_pipes->num)) {
			return DTEST_FAILURE;
		}
	}

	ret = atomic_set_planes(dev, dtest_planes->planes, dtest_planes->num, false);
	if (ret) {
		DTESTLOGE("Atomic Set planes failed");
		return DTEST_FAILURE;
	}

	ret = atomic_set_cursors(dev, dtest_cursors->cursors, dtest_cursors->num);
	if (ret) {
		DTESTLOGE("Atomic Set cursors failed");
		return DTEST_FAILURE;
	}

	ret = drmModeAtomicCommit(dev->fd, dev->req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
	if (ret) {
		DTESTLOGE("Atomic commit set frame %u fail", frame->frame_id);
		if (ret == -ENOTSUP)
			return DTEST_NOT_SUPPORT;
		else
			return DTEST_FAILURE;
	}

	drmModeAtomicFree(dev->req);
	return status;
error:
	return DTEST_FAILURE;
}

int dpu_get_time_of_day(dtest_pipe_t *pipes)
{
	int ret;
	ret = gettimeofday(&pipes->pipes->start, NULL);

	return ret;
}

static bool support_program_wb_point(device_t *dev, uint32_t connector_id)
{
	struct connector *connector = NULL;
	drmModeObjectProperties *props = NULL;
	drmModePropertyRes **props_info = NULL;
	const char *obj_type = "wb connector";
	uint32_t i;

	for (i = 0; i < (int)dev->resources->res->count_connectors; ++i) {
		connector = &dev->resources->connectors[i];
		if (connector->connector->connector_id == connector_id) {
			props = connector->props;
			props_info = connector->props_info;
			break;
		}
	}

	if (!props) {
		fprintf(stderr, "%s %i has no properties\n", obj_type,
			connector->connector->connector_id);
		return false;
	}

	for (i = 0; i < (int)props->count_props; ++i) {
		if (!props_info[i])
			continue;
		if (strcmp(props_info[i]->name, "WB_POINT") == 0)
			return true;
	}

	return false;
}

/**
 * Wait for wb frm done.
 *
 * \param fd file descriptor.
 * \param wb_done pointer to a drm_vs_wb_frm_done structure.
 *
 * \return zero on success, or a negative value on failure.
 *
 * \internal
 * This function is a wrapper around the DRM_IOCTL_VS_GET_WB_FRM_DONE ioctl.
 */
int drmWaitWbFrmDone(int fd, struct drm_vs_wb_frm_done *wb_done)
{
	struct timespec timeout, cur;
	int ret;

	ret = clock_gettime(CLOCK_MONOTONIC, &timeout);
	if (ret < 0) {
		fprintf(stderr, "clock_gettime failed: %s\n", strerror(errno));
		goto out;
	}
	timeout.tv_sec++;

	do {
		ret = drmIoctl(fd, DRM_IOCTL_VS_GET_WB_FRM_DONE, wb_done);
		if (ret && errno != 0) {
			clock_gettime(CLOCK_MONOTONIC, &cur);
			/* Timeout after 60s */
			if (cur.tv_sec > timeout.tv_sec + 60) {
				errno = EBUSY;
				ret = -1;
				break;
			}
			sleep(1);
		}
	} while (ret && errno != 0);

out:
	return ret;
}

dtest_status dpu_wait_wb_frm_done(device_t *dev, int conn_index)
{
	int ret;
	struct drm_vs_wb_frm_done wb_done;

	wb_done.wb_id = conn_index;

	ret = drmWaitWbFrmDone(dev->fd, &wb_done);
	if (ret != 0) {
		printf("drmWaitWbFrmDone time out on wb id: %i.\n", conn_index);
	}

	return DTEST_SUCCESS;
}

dtest_status dpu_wait_vblank(device_t *dev, int crtc_index)
{
	int ret;
	drmVBlank vbl;

	vbl.request.type = DRM_VBLANK_RELATIVE | (crtc_index << DRM_VBLANK_HIGH_CRTC_SHIFT);
	vbl.request.sequence = 1;

	ret = drmWaitVBlank(dev->fd, &vbl);
	if (ret != 0) {
		printf("drmWaitVBlank (relative) time out on crtc id: %i, sleep 10 seconds.\n",
		       crtc_index);
		sleep(10);
	}

	return DTEST_SUCCESS;
}

dtest_status dpu_wait_signal(device_t *dev, dtest_pipe_t *dtest_pipe)
{
	uint32_t i, crtc_index, conn_index;
	drmModeConnector *connector = NULL;
	pipe_t *pipe = dtest_pipe->pipes;
	int ret = DTEST_SUCCESS;

	for (i = 0; i < dtest_pipe->num; i++) {
		if (pipe[i].global_disable)
			continue;

		connector = get_connector_by_id(dev, pipe[i].con_ids[0]);
		if (!connector) {
			DTESTLOGE("get_connector_by_id fail");
			ret = DTEST_FAILURE;
			return ret;
		}

		if (connector->connector_type == DRM_MODE_CONNECTOR_WRITEBACK) {
			/* For every pipe, we choose wait vblank
			 * if it not support wb point cause there isn't wb frm done */
			if (!support_program_wb_point(dev, connector->connector_id)) {
				crtc_index = get_crtc_index(dev, pipe[i].crtc_id);
				ret |= dpu_wait_vblank(dev, crtc_index);
			} else {
				conn_index = get_connector_index(dev, connector->connector_id);
				ret |= dpu_wait_wb_frm_done(dev, conn_index);
			}
		}
	}

	return ret;
}

dtest_status dpu_dump_output_fbs(device_t *dev, dtest_pipe_t *dtest_pipe, const char *dump_folder,
				 const char *case_name, FILE **stream_fp, uint32_t *pos_frame,
				 bool save_raw, uint32_t frame_id, bool check, uint32_t *run_count,
				 uint32_t *stream_id)
{
	uint32_t i, j;
	drmModeConnector *connector;
	pipe_t *pipe = dtest_pipe->pipes;
	char filename[512] = { 0 };
	bool dump_flag = false;

	for (i = 0; i < dtest_pipe->num; i++) {
		if (!pipe[i].global_disable) {
			memset(filename, 0, sizeof(filename));
			if (frame_id > 0 || pipe[0].stream)
#if CONFIG_VERISILICON_STREAM_DEBUG
				if (run_count)
					snprintf(filename, sizeof(filename),
						 "%s/rt%u_%s_stream%u_wb%s_frame%d", dump_folder,
						 *run_count, case_name, *stream_id, pipe[i].cons[0],
						 frame_id);
				else
					snprintf(filename, sizeof(filename), "%s/%s_%s_frame%d",
						 dump_folder, case_name, pipe[i].cons[0], frame_id);
#else
				snprintf(filename, sizeof(filename), "%s/%s_%s_frame%d",
					 dump_folder, case_name, pipe[i].cons[0], frame_id);
#endif
			else
				snprintf(filename, sizeof(filename), "%s/%s_%s", dump_folder,
					 case_name, pipe[i].cons[0]);
			connector = get_connector_by_id(dev, pipe[i].con_ids[0]);
			if (!connector) {
				DTESTLOGE("get_connector_by_id: %u failed.", pipe[i].con_ids[0]);
				return DTEST_FAILURE;
			}
			if (connector->connector_type == DRM_MODE_CONNECTOR_WRITEBACK) {
				dump_flag = true;
				/* if hw doesn't support program_wb_point feature, stride of the destinatin buffer needs to be non-aligned. */
				if (!support_program_wb_point(dev, connector->connector_id)) {
					for (j = 0; j < pipe[i].num_planes; j++) {
						pipe[i].out_bo[j]->pitch =
							pipe[i].out_bo[j]->width *
							__get_bpp(pipe[i].fourcc,
								  pipe[i].wb_modifier) /
							8;
					}
				}
				if (bo_dump(&pipe[i], filename, stream_fp[i], pos_frame[i],
					    save_raw, check) != DTEST_SUCCESS) {
					DTESTLOGE("bo_dump fail");
					return DTEST_FAILURE;
				}
			} else if (dev->use_vdp) {
				dump_flag = true;
				uint32_t crtc_index = 0;
				crtc_index = get_crtc_index(dev, pipe[i].crtc_id);
				dpu_wait_vblank(dev, crtc_index);
				sleep(4);
				if (bo_dump(&pipe[i], filename, stream_fp[i], pos_frame[i],
					    save_raw, check) != DTEST_SUCCESS) {
					DTESTLOGE("bo_dump fail");
					return DTEST_FAILURE;
				}
			}
		}
	}
	/* for some chip, there might be no wb and no vdp output*/
	if (i > 0 && !dump_flag)
		DTESTLOGE("no wb for bo_dump");
	return DTEST_SUCCESS;
}

void dpu_clear_frame_FB(struct device *dev, frame_t *frame)
{
	unsigned int i, j;
	if (!frame)
		return;
	dtest_plane_t *dtest_planes = &frame->plane;
	dtest_cursor_t *dtest_cursors = &frame->cursor;
	dtest_pipe_t *dtest_pipes = &frame->pipe;
	plane_t *plane = dtest_planes->planes;
	cursor_t *cursor = dtest_cursors->cursors;
	unsigned int plane_count = dtest_planes->num;
	unsigned int cursor_count = dtest_cursors->num;
	unsigned int pipe_count = dtest_pipes->num;
	pipe_t *pipe = dtest_pipes->pipes;

	for (i = 0; i < cursor_count; i++) {
		if (cursor[i].bo[0]) {
			bo_destroy(cursor[i].bo[0]);
			cursor[i].bo[0] = NULL;
		}
	}

	for (i = 0; i < plane_count; i++) {
		if (plane[i].fb_id) {
			drmModeRmFB(dev->fd, plane[i].fb_id);
			plane[i].fb_id = 0;
		}
		if (plane[i].old_fb_id) {
			drmModeRmFB(dev->fd, plane[i].old_fb_id);
			plane[i].old_fb_id = 0;
		}

		/* VSI updated */
		for (j = 0; j < plane[i].num_planes; j++) {
			if (plane[i].bo[j]) {
				bo_destroy(plane[i].bo[j]);
				plane[i].bo[j] = NULL;
			}

			if (plane[i].old_bo[j]) {
				bo_destroy(plane[i].old_bo[j]);
				plane[i].old_bo[j] = NULL;
			}
		}
		/* updated end */
	}

	/* writeback FB clear */
	for (i = 0; i < pipe_count; i++) {
		/* writeback with stripe means the writeback buffer need use in other commit */
		if (pipe[i].wb_with_stripe)
			continue;

		if (pipe[i].out_fb_id && (pipe[i].out_fb_id != dev->ramless.ramless_fb_id)) {
			drmModeRmFB(dev->fd, pipe[i].out_fb_id);
			pipe[i].out_fb_id = 0;
		}

		for (j = 0; j < pipe[i].num_planes; j++) {
			if (pipe[i].out_bo[j]) {
				bo_destroy(pipe[i].out_bo[j]);
				pipe[i].out_bo[j] = NULL;
			}
		}
	}

	/* ramless Framebuffer clear */
	if (!dev->ramless.fb_inuse && dev->ramless.ramless_fb_id) {
		drmModeRmFB(dev->fd, dev->ramless.ramless_fb_id);
		dev->ramless.ramless_fb_id = 0;
	}
}

static void atomic_clear_planes(struct device *dev, struct plane_arg *p, unsigned int count)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		add_property(dev, p[i].plane_id, "FB_ID", 0);
		add_property(dev, p[i].plane_id, "CRTC_ID", 0);
		add_property(dev, p[i].plane_id, "SRC_X", 0);
		add_property(dev, p[i].plane_id, "SRC_Y", 0);
		add_property(dev, p[i].plane_id, "SRC_W", 0);
		add_property(dev, p[i].plane_id, "SRC_H", 0);
		add_property(dev, p[i].plane_id, "CRTC_X", 0);
		add_property(dev, p[i].plane_id, "CRTC_Y", 0);
		add_property(dev, p[i].plane_id, "CRTC_W", 0);
		add_property(dev, p[i].plane_id, "CRTC_H", 0);
		add_property(dev, p[i].plane_id, "rotation", DRM_MODE_ROTATE_0);
	}
}

static void atomic_clear_mode(struct device *dev, struct pipe_arg *pipes, unsigned int count)
{
	unsigned int i;
	unsigned int j;

	for (i = 0; i < count; i++) {
		struct pipe_arg *pipe = &pipes[i];

		if (pipe->mode == NULL)
			continue;

		for (j = 0; j < pipe->num_cons; ++j)
			add_property(dev, pipe->con_ids[j], "CRTC_ID", 0);

		add_property(dev, pipe->crtc->crtc->crtc_id, "MODE_ID", 0);
		add_property(dev, pipe->crtc->crtc->crtc_id, "ACTIVE", 0);
	}
}

dtest_status dpu_clear_all_pipes(device_t *dev, dtest_frame_t *dtest_frames)
{
	/* disable all the planes and crtcs */
	int ret;
	uint i;

	for (i = 0; i < dtest_frames->num; i++) {
		printf("Disabling frame%d all the planes and crtcs.\n", i);
		dev->req = drmModeAtomicAlloc();

		frame_t frame = dtest_frames->frames[i];
		dtest_cursor_t *dtest_cursors = &frame.cursor;

		leagcy_disable_cursors(dev, dtest_cursors->cursors, dtest_cursors->num);
		atomic_clear_mode(dev, frame.pipe.pipes, frame.pipe.num);
		atomic_clear_planes(dev, frame.plane.planes, frame.plane.num);

		ret = drmModeAtomicCommit(dev->fd, dev->req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
		if (ret) {
			DTESTLOGE("Atomic commit disable all planes and crtcs fail.\n");
			drmModeAtomicFree(dev->req);
			return DTEST_FAILURE;
		}
		drmModeAtomicFree(dev->req);
	}

	return DTEST_SUCCESS;
}

dtest_status dpu_clear_all_properties(device_t *dev, dtest_frame_t *dtest_frames)
{
	uint32_t i, j;
	int ret;
	dtest_status status = DTEST_SUCCESS;

	for (i = 0; i < dtest_frames->num; i++) {
		printf("Disabling frame%d all the properties.\n", i);

		dev->req = drmModeAtomicAlloc();
		dtest_property_t dtest_property = dtest_frames->frames[i].property;
		property_t *prop = dtest_property.properties;
		dtest_pipe_t *dtest_pipes = &dtest_frames->frames[i].pipe;

		if (writeback_config(dev, dtest_pipes->pipes, dtest_pipes->num)) {
			ret = DTEST_FAILURE;
			goto FREE;
		}

		for (j = 0; j < dtest_property.num; j++) {
			prop->value = 0;
			status = set_property(dev, prop);
			if (status == DTEST_SUCCESS) {
				DTESTLOGI(
					"frame%d property obj_id[%u] obj_type[%u] name[%s] value[%lu]",
					i, prop->obj_id, prop->obj_type, prop->name, prop->value);
				prop++;
			} else if (status == DTEST_FAILURE) {
				DTESTLOGE("frame%d clear_property fail!\n", i);
				ret = DTEST_FAILURE;
				goto FREE;
			} else {
				DTESTLOGI("frame%d has unsupport property!\n", i);
				ret = DTEST_NOT_SUPPORT;
				goto FREE;
			}
		}

		if (drmModeAtomicCommit(dev->fd, dev->req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL)) {
			DTESTLOGE("frame%d Atomic commit disable all properties fail.\n", i);
			continue;
		}
		drmModeAtomicFree(dev->req);
	}
	return DTEST_SUCCESS;
FREE:
	drmModeAtomicFree(dev->req);
	return ret;
}

dtest_status dpu_clear_frame(device_t *dev, frame_t *frame)
{
	int32_t ret;
	dtest_plane_t *dtest_planes = &frame->plane;
	dtest_pipe_t *dtest_pipes = &frame->pipe;

	dev->req = drmModeAtomicAlloc();
	DTEST_CHECK_PTR(dev->req, error);

	atomic_clear_mode(dev, dtest_pipes->pipes, dtest_pipes->num);
	atomic_clear_planes(dev, dtest_planes->planes, dtest_planes->num);

	ret = drmModeAtomicCommit(dev->fd, dev->req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
	if (ret) {
		DTESTLOGE("Atomic commit clear frame %u fail", frame->frame_id);
		return DTEST_FAILURE;
	}

	drmModeAtomicFree(dev->req);

	return DTEST_SUCCESS;

error:
	return DTEST_FAILURE;
}

int dpu_device_software_reset(int fd, enum drm_vs_reset_mode mode)
{
	int errcode;
	errcode = dtest_soft_reset(fd, mode);

	return errcode;
}

int dpu_device_software_clean(int fd, enum drm_vs_sw_reset_mode mode)
{
	struct drm_vs_sw_reset arg;
	arg.reset_mode = mode;
	return drmIoctl(fd, DRM_IOCTL_VS_CLEAN_SW_RESET, &arg);
}

void dpu_free_frames(dtest_frame_t *dtest_frames)
{
	dtest_case_delete(dtest_frames);
}

void dpu_free_resources(struct resources *res)
{
	int i;

	if (!res)
		return;

#define free_resource(_res, __res, type, Type)                              \
	do {                                                                \
		if (!(_res)->type##s)                                       \
			break;                                              \
		for (i = 0; i < (int)(_res)->__res->count_##type##s; ++i) { \
			if (!(_res)->type##s[i].type)                       \
				break;                                      \
			drmModeFree##Type((_res)->type##s[i].type);         \
		}                                                           \
		free((_res)->type##s);                                      \
	} while (0)

#define free_properties(_res, __res, type)                                  \
	do {                                                                \
		for (i = 0; i < (int)(_res)->__res->count_##type##s; ++i) { \
			drmModeFreeObjectProperties(res->type##s[i].props); \
			free(res->type##s[i].props_info);                   \
		}                                                           \
	} while (0)

	if (res->res) {
		free_properties(res, res, crtc);

		free_resource(res, res, crtc, Crtc);
		free_resource(res, res, encoder, Encoder);

		for (i = 0; i < res->res->count_connectors; i++)
			free(res->connectors[i].name);

		free_resource(res, res, connector, Connector);
		free_resource(res, res, fb, FB);

		drmModeFreeResources(res->res);
	}

	if (res->plane_res) {
		free_properties(res, plane_res, plane);

		free_resource(res, plane_res, plane, Plane);

		drmModeFreePlaneResources(res->plane_res);
	}

	free(res);
}

int dpu_close_device(int fd)
{
	int ret;
	ret = drmClose(fd);

	return ret;
}

void dpu_free_device(struct device *dev)
{
	if (dev->resources)
		dpu_free_resources(dev->resources);

	if (dev->ltm_hist_bo1)
		bo_destroy(dev->ltm_hist_bo1);

	if (dev->ltm_hist_bo2)
		bo_destroy(dev->ltm_hist_bo2);

	if (dev->hist_bo)
		bo_destroy(dev->hist_bo);

	if (dev->dpu_algorithm)
		dpu_algorithm_lib_deinit(dev->dpu_algorithm);

	if (dev->fd > 0)
		dpu_close_device(dev->fd);
}

#if CONFIG_VERISILICON_MD5_CHECK
void calcu_md5(const char *filename, char *md5)
{
	uint8_t md5_hash[MD5_DIGEST_LEN];
	vs_calcu_md5(filename, md5_hash);
	memset(md5, 0, MD5_BUFFER_SIZE);
	for (int i = 0; i < MD5_DIGEST_LEN; i++) {
		vs_fill_sprint(&md5[i * 2], "%02x", md5_hash[i]);
	}
}
#endif
