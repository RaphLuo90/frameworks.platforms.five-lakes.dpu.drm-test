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

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include "type.h"
#include "util/common.h"
#include "util/format.h"
#include "util/kms.h"
#include "drm/vs_drm_fourcc.h"

#include "xf86drm.h"

#include "buffers.h"
#include "vs_bo_helper.h"
#include "drmtest.h"
#include "drmtest_helper.h"
#include "prop_mapper.h"
#include "case_builder.h"

static enum util_fill_pattern primary_fill = UTIL_PATTERN_SMPTE;
static enum util_fill_pattern secondary_fill = UTIL_PATTERN_TILES;
static enum util_fill_pattern plain_fill = UTIL_PATTERN_PLAIN;

static inline int64_t U642I64(uint64_t val)
{
	return (int64_t) * ((int64_t *)&val);
}

#define bit_name_fn(res)                                             \
	const char *res##_str(int type)                              \
	{                                                            \
		unsigned int i;                                      \
		const char *sep = "";                                \
		for (i = 0; i < ARRAY_SIZE(res##_names); i++) {      \
			if (type & (1 << i)) {                       \
				printf("%s%s", sep, res##_names[i]); \
				sep = ", ";                          \
			}                                            \
		}                                                    \
		return NULL;                                         \
	}

static const char *mode_type_names[] = {
	"builtin", "clock_c", "crtc_c", "preferred", "default", "userdef", "driver",
};

static bit_name_fn(mode_type)

	static const
	char *mode_flag_names[] = { "phsync",  "nhsync", "pvsync", "nvsync", "interlace",
				    "dblscan", "csync",	 "pcsync", "ncsync", "hskew",
				    "bcast",   "pixmux", "dblclk", "clkdiv2" };

static bit_name_fn(mode_flag)

	static void dump_fourcc(uint32_t fourcc)
{
	printf(" %c%c%c%c", fourcc, fourcc >> 8, fourcc >> 16, fourcc >> 24);
}

static void dump_encoders(struct device *dev)
{
	drmModeEncoder *encoder;
	int i;

	printf("Encoders:\n");
	printf("id\tcrtc\ttype\tpossible crtcs\tpossible clones\t\n");
	for (i = 0; i < dev->resources->res->count_encoders; i++) {
		encoder = dev->resources->encoders[i].encoder;
		if (!encoder)
			continue;

		printf("%d\t%d\t%s\t0x%08x\t0x%08x\n", encoder->encoder_id, encoder->crtc_id,
		       util_lookup_encoder_type_name(encoder->encoder_type),
		       encoder->possible_crtcs, encoder->possible_clones);
	}
	printf("\n");
}

static void dump_mode(drmModeModeInfo *mode)
{
	printf("  %s %d %d %d %d %d %d %d %d %d %d", mode->name, mode->vrefresh, mode->hdisplay,
	       mode->hsync_start, mode->hsync_end, mode->htotal, mode->vdisplay, mode->vsync_start,
	       mode->vsync_end, mode->vtotal, mode->clock);

	printf(" flags: ");
	mode_flag_str(mode->flags);
	printf("; type: ");
	mode_type_str(mode->type);
	printf("\n");
}

static void dump_blob(struct device *dev, uint32_t blob_id)
{
	uint32_t i;
	unsigned char *blob_data;
	drmModePropertyBlobPtr blob;

	blob = drmModeGetPropertyBlob(dev->fd, blob_id);
	if (!blob) {
		printf("\n");
		return;
	}
	if (!blob->data) {
		printf("\n");
		drmModeFreePropertyBlob(blob);
		return;
	}

	blob_data = blob->data;

	for (i = 0; i < blob->length; i++) {
		if (i % 16 == 0)
			printf("\n\t\t\t");
		printf("%.2hhx", blob_data[i]);
	}
	printf("\n");

	drmModeFreePropertyBlob(blob);
}

static const char *modifier_to_string(uint64_t modifier)
{
	switch (modifier) {
	case DRM_FORMAT_MOD_INVALID:
		return "INVALID";
	case DRM_FORMAT_MOD_LINEAR:
		return "LINEAR";
	case I915_FORMAT_MOD_X_TILED:
		return "X_TILED";
	case I915_FORMAT_MOD_Y_TILED:
		return "Y_TILED";
	case I915_FORMAT_MOD_Yf_TILED:
		return "Yf_TILED";
	case I915_FORMAT_MOD_Y_TILED_CCS:
		return "Y_TILED_CCS";
	case I915_FORMAT_MOD_Yf_TILED_CCS:
		return "Yf_TILED_CCS";
	case DRM_FORMAT_MOD_SAMSUNG_64_32_TILE:
		return "SAMSUNG_64_32_TILE";
	case DRM_FORMAT_MOD_VIVANTE_TILED:
		return "VIVANTE_TILED";
	case DRM_FORMAT_MOD_VIVANTE_SUPER_TILED:
		return "VIVANTE_SUPER_TILED";
	case DRM_FORMAT_MOD_VIVANTE_SPLIT_TILED:
		return "VIVANTE_SPLIT_TILED";
	case DRM_FORMAT_MOD_VIVANTE_SPLIT_SUPER_TILED:
		return "VIVANTE_SPLIT_SUPER_TILED";
	case DRM_FORMAT_MOD_NVIDIA_TEGRA_TILED:
		return "NVIDIA_TEGRA_TILED";
	case DRM_FORMAT_MOD_NVIDIA_16BX2_BLOCK(0):
		return "NVIDIA_16BX2_BLOCK(0)";
	case DRM_FORMAT_MOD_NVIDIA_16BX2_BLOCK(1):
		return "NVIDIA_16BX2_BLOCK(1)";
	case DRM_FORMAT_MOD_NVIDIA_16BX2_BLOCK(2):
		return "NVIDIA_16BX2_BLOCK(2)";
	case DRM_FORMAT_MOD_NVIDIA_16BX2_BLOCK(3):
		return "NVIDIA_16BX2_BLOCK(3)";
	case DRM_FORMAT_MOD_NVIDIA_16BX2_BLOCK(4):
		return "NVIDIA_16BX2_BLOCK(4)";
	case DRM_FORMAT_MOD_NVIDIA_16BX2_BLOCK(5):
		return "NVIDIA_16BX2_BLOCK(5)";
	case DRM_FORMAT_MOD_BROADCOM_VC4_T_TILED:
		return "MOD_BROADCOM_VC4_T_TILED";
	case DRM_FORMAT_MOD_QCOM_COMPRESSED:
		return "QCOM_COMPRESSED";
	default:
		return "(UNKNOWN MODIFIER)";
	}
}

static void dump_in_formats(struct device *dev, uint32_t blob_id)
{
	uint32_t i, j;
	drmModePropertyBlobPtr blob;
	struct drm_format_modifier_blob *header;
	uint32_t *formats;
	struct drm_format_modifier *modifiers;

	printf("\t\tin_formats blob decoded:\n");
	blob = drmModeGetPropertyBlob(dev->fd, blob_id);
	if (!blob) {
		printf("\n");
		return;
	}
	if (!blob->data) {
		printf("\n");
		drmModeFreePropertyBlob(blob);
		return;
	}

	header = blob->data;
	formats = (uint32_t *)((char *)header + (header->formats_offset));
	modifiers = (struct drm_format_modifier *)((char *)header + (header->modifiers_offset));

	for (i = 0; i < header->count_formats; i++) {
		printf("\t\t\t");
		dump_fourcc(formats[i]);
		printf(": ");
		for (j = 0; j < header->count_modifiers; j++) {
			uint64_t mask = 1ULL << i;
			if (modifiers[j].formats & mask)
				printf(" %s", modifier_to_string(modifiers[j].modifier));
		}
		printf("\n");
	}

	drmModeFreePropertyBlob(blob);
}

static void dump_prop(struct device *dev, drmModePropertyPtr prop, uint32_t prop_id, uint64_t value)
{
	int i;
	printf("\t%d", prop_id);
	if (!prop) {
		printf("\n");
		return;
	}

	printf(" %s:\n", prop->name);

	printf("\t\tflags:");
	if (prop->flags & DRM_MODE_PROP_PENDING)
		printf(" pending");
	if (prop->flags & DRM_MODE_PROP_IMMUTABLE)
		printf(" immutable");
	if (drm_property_type_is(prop, DRM_MODE_PROP_SIGNED_RANGE))
		printf(" signed range");
	if (drm_property_type_is(prop, DRM_MODE_PROP_RANGE))
		printf(" range");
	if (drm_property_type_is(prop, DRM_MODE_PROP_ENUM))
		printf(" enum");
	if (drm_property_type_is(prop, DRM_MODE_PROP_BITMASK))
		printf(" bitmask");
	if (drm_property_type_is(prop, DRM_MODE_PROP_BLOB))
		printf(" blob");
	if (drm_property_type_is(prop, DRM_MODE_PROP_OBJECT))
		printf(" object");
	printf("\n");

	if (drm_property_type_is(prop, DRM_MODE_PROP_SIGNED_RANGE)) {
		printf("\t\tvalues:");
		for (i = 0; i < prop->count_values; i++)
			printf(" %" PRId64, U642I64(prop->values[i]));
		printf("\n");
	}

	if (drm_property_type_is(prop, DRM_MODE_PROP_RANGE)) {
		printf("\t\tvalues:");
		for (i = 0; i < prop->count_values; i++)
			printf(" %" PRIu64, prop->values[i]);
		printf("\n");
	}

	if (drm_property_type_is(prop, DRM_MODE_PROP_ENUM)) {
		printf("\t\tenums:");
		for (i = 0; i < prop->count_enums; i++)
			printf(" %s=%llu", prop->enums[i].name, prop->enums[i].value);
		printf("\n");
	} else if (drm_property_type_is(prop, DRM_MODE_PROP_BITMASK)) {
		printf("\t\tvalues:");
		for (i = 0; i < prop->count_enums; i++)
			printf(" %s=0x%llx", prop->enums[i].name, (1LL << prop->enums[i].value));
		printf("\n");
	} else {
		assert(prop->count_enums == 0);
	}

	if (drm_property_type_is(prop, DRM_MODE_PROP_BLOB)) {
		printf("\t\tblobs:\n");
		for (i = 0; i < prop->count_blobs; i++)
			dump_blob(dev, prop->blob_ids[i]);
		printf("\n");
	} else {
		assert(prop->count_blobs == 0);
	}

	printf("\t\tvalue:");
	if (drm_property_type_is(prop, DRM_MODE_PROP_BLOB))
		dump_blob(dev, value);
	else if (drm_property_type_is(prop, DRM_MODE_PROP_SIGNED_RANGE))
		printf(" %" PRId64 "\n", value);
	else
		printf(" %" PRIu64 "\n", value);

	if (strcmp(prop->name, "IN_FORMATS") == 0)
		dump_in_formats(dev, value);
}

static void dump_connectors(struct device *dev)
{
	int i, j;
	const char *ret = NULL;

	printf("Connectors:\n");
	printf("id\tencoder\tstatus\t\tname\t\tsize (mm)\tmodes\tencoders\n");
	for (i = 0; i < dev->resources->res->count_connectors; i++) {
		struct connector *_connector = &dev->resources->connectors[i];
		drmModeConnector *connector = _connector->connector;
		if (!connector)
			continue;

		ret = util_lookup_connector_status_name(connector->connection);
		if (!ret)
			printf("Failed to lookup connector status name\n");
		else
			printf("%d\t%d\t%s\t%-15s\t%dx%d\t\t%d\t", connector->connector_id,
			       connector->encoder_id, ret, _connector->name, connector->mmWidth,
			       connector->mmHeight, connector->count_modes);

		for (j = 0; j < connector->count_encoders; j++)
			printf("%s%d", j > 0 ? ", " : "", connector->encoders[j]);
		printf("\n");

		if (connector->count_modes) {
			printf("  modes:\n");
			printf("\tname refresh (Hz) hdisp hss hse htot vdisp "
			       "vss vse vtot)\n");
			for (j = 0; j < connector->count_modes; j++)
				dump_mode(&connector->modes[j]);
		}

		if (_connector->props) {
			printf("  props:\n");
			for (j = 0; j < (int)_connector->props->count_props; j++)
				dump_prop(dev, _connector->props_info[j],
					  _connector->props->props[j],
					  _connector->props->prop_values[j]);
		}
	}
	printf("\n");
}

static void dump_crtcs(struct device *dev)
{
	int i;
	uint32_t j;

	printf("CRTCs:\n");
	printf("id\tfb\tpos\tsize\n");
	for (i = 0; i < dev->resources->res->count_crtcs; i++) {
		struct crtc *_crtc = &dev->resources->crtcs[i];
		drmModeCrtc *crtc = _crtc->crtc;
		if (!crtc)
			continue;

		printf("%d\t%d\t(%d,%d)\t(%dx%d)\n", crtc->crtc_id, crtc->buffer_id, crtc->x,
		       crtc->y, crtc->width, crtc->height);
		dump_mode(&crtc->mode);

		if (_crtc->props) {
			printf("  props:\n");
			for (j = 0; j < _crtc->props->count_props; j++)
				dump_prop(dev, _crtc->props_info[j], _crtc->props->props[j],
					  _crtc->props->prop_values[j]);
		} else {
			printf("  no properties found\n");
		}
	}
	printf("\n");
}

static void dump_framebuffers(struct device *dev)
{
	drmModeFB *fb;
	int i;

	printf("Frame buffers:\n");
	printf("id\tsize\tpitch\n");
	for (i = 0; i < dev->resources->res->count_fbs; i++) {
		fb = dev->resources->fbs[i].fb;
		if (!fb)
			continue;

		printf("%u\t(%ux%u)\t%u\n", fb->fb_id, fb->width, fb->height, fb->pitch);
	}
	printf("\n");
}

static void dump_planes(struct device *dev)
{
	unsigned int i, j;

	printf("Planes:\n");
	printf("id\tcrtc\tfb\tCRTC x,y\tx,y\tgamma size\tpossible crtcs\n");

	if (!dev->resources->plane_res)
		return;

	for (i = 0; i < dev->resources->plane_res->count_planes; i++) {
		struct plane *plane = &dev->resources->planes[i];
		drmModePlane *ovr = plane->plane;
		if (!ovr)
			continue;

		printf("%d\t%d\t%d\t%d,%d\t\t%d,%d\t%-8d\t0x%08x\n", ovr->plane_id, ovr->crtc_id,
		       ovr->fb_id, ovr->crtc_x, ovr->crtc_y, ovr->x, ovr->y, ovr->gamma_size,
		       ovr->possible_crtcs);

		if (!ovr->count_formats)
			continue;

		printf("  formats:");
		for (j = 0; j < ovr->count_formats; j++)
			dump_fourcc(ovr->formats[j]);
		printf("\n");

		if (plane->props) {
			printf("  props:\n");
			for (j = 0; j < plane->props->count_props; j++)
				dump_prop(dev, plane->props_info[j], plane->props->props[j],
					  plane->props->prop_values[j]);
		} else {
			printf("  no properties found\n");
		}
	}
	printf("\n");

	return;
}

static void free_resources(struct resources *res)
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

static struct resources *get_resources(struct device *dev)
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

	for (i = 0; i < res->res->count_crtcs; ++i)
		res->crtcs[i].mode = &res->crtcs[i].crtc->mode;

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
	free_resources(res);
	return NULL;
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

static int set_resource(struct resource_arg *resource, struct plane_arg *plane,
			unsigned int plane_count)
{
	unsigned int i;

	for (i = 0; i < plane_count; i++) {
		if (resource->plane_id == plane[i].plane_id) {
			memcpy(plane[i].file_src, resource->file_src, RESOURCE_LEN + 1);
			return 0;
		}
	}

	fprintf(stderr, "invalid plane id %u for source: %s\n", resource->plane_id,
		resource->file_src);
	return -1;
}

static int parse_connector(struct pipe_arg *pipe, const char *arg)
{
	unsigned int i;
	const char *p;
	char *endp = NULL;

	pipe->vrefresh = 0;
	pipe->crtc_id = (uint32_t)-1;
	strcpy(pipe->format_str, "XR24");

	/* Count the number of connectors and allocate them. */
	pipe->num_cons = 1;
	for (p = arg; *p && *p != ':' && *p != '@'; ++p) {
		if (*p == ',')
			pipe->num_cons++;
	}

	pipe->con_ids = calloc(pipe->num_cons, sizeof(*pipe->con_ids));
	pipe->cons = calloc(pipe->num_cons, sizeof(*pipe->cons));
	if (pipe->con_ids == NULL || pipe->cons == NULL)
		return -1;

	/* Parse the connectors. */
	for (i = 0, p = arg; i < pipe->num_cons; ++i, p = endp + 1) {
		endp = strpbrk(p, ",@:");
		if (!endp)
			break;

		pipe->cons[i] = strndup(p, endp - p);

		if (*endp != ',')
			break;
	}
	if (!endp) {
		printf("endp points to NULL.\n");
		return -1;
	}

	if (i != pipe->num_cons - 1)
		return -1;

	/* Parse the remaining parameters. */
	if (*endp == '@') {
		arg = endp + 1;
		pipe->crtc_id = strtoul(arg, &endp, 10);
	}
	if (*endp != ':')
		return -1;

	arg = endp + 1;

	/* Search for the vertical refresh or the format. */
	p = strpbrk(arg, "-@");
	if (p == NULL)
		p = arg + strlen(arg);

	snprintf(pipe->mode_str, sizeof(pipe->mode_str), "%s", arg);

	if (*p == '-') {
		pipe->vrefresh = strtoul(p + 1, &endp, 10);
		p = endp;
	}

	if (*p == '@') {
		strncpy(pipe->format_str, p + 1, 4);
		pipe->format_str[4] = '\0';
	}

	pipe->fourcc = util_format_fourcc(pipe->format_str);
	if (pipe->fourcc == 0) {
		fprintf(stderr, "unknown format %s\n", pipe->format_str);
		return -1;
	}

	return 0;
}

static int parse_plane(struct plane_arg *plane, const char *p)
{
	char *end;
	unsigned int tmp;

	plane->plane_id = strtoul(p, &end, 10);
	if (*end != '@')
		return -EINVAL;

	p = end + 1;
	plane->crtc_id = strtoul(p, &end, 10);
	if (*end != ':')
		return -EINVAL;

	p = end + 1;
	plane->w = strtoul(p, &end, 10);
	if (*end != 'x')
		return -EINVAL;

	p = end + 1;
	plane->h = strtoul(p, &end, 10);

	if (*end == '+' || *end == '-') {
		plane->x = strtol(end, &end, 10);
		if (*end != '+' && *end != '-')
			return -EINVAL;
		plane->y = strtol(end, &end, 10);

		plane->has_position = true;
	}

	if (*end == '*') {
		p = end + 1;
		plane->scale = strtod(p, &end);
		if (plane->scale <= 0.0)
			return -EINVAL;
	} else {
		plane->scale = 1.0;
	}

	if (*end == '@') {
		strncpy(plane->format_str, end + 1, 4);
		plane->format_str[4] = '\0';
		end += 5;
	} else {
		strncpy(plane->format_str, "XR24", 5);
	}

	/* VSI updated */
	if (*end == '-') {
		p = end + 1;
		tmp = strtol(p, &end, 10);

		if (tmp >= 256 && tmp < 512) {
			plane->modifier =
				fourcc_mod_vs_pvric_code((tmp & 0xFF), DRM_FORMAT_MOD_VS_DEC_LOSSY);
		} else if (tmp >= 512) {
			plane->modifier = fourcc_mod_vs_pvric_code((tmp & 0xFF),
								   DRM_FORMAT_MOD_VS_DEC_LOSSLESS);
		} else {
			plane->modifier = fourcc_mod_vs_norm_code(tmp & 0xFF);
		}
	} else {
		plane->modifier = DRM_FORMAT_MOD_LINEAR;
	}
	/* updated end */

	plane->fourcc = util_format_fourcc(plane->format_str);
	if (plane->fourcc == 0) {
		fprintf(stderr, "unknown format %s\n", plane->format_str);
		return -EINVAL;
	}
	return 0;
}

static int parse_property(struct property_arg *p, const char *arg)
{
	if (sscanf(arg, "%u:%32[^:]:%" SCNu64, &p->obj_id, p->name, &p->value) != 3)
		return -1;

	p->obj_type = 0;
	p->name[DRM_PROP_NAME_LEN] = '\0';

	return 0;
}

static int parse_resource(struct resource_arg *p, const char *arg)
{
	if (sscanf(arg, "%u:%32s" SCNu64, &p->plane_id, p->file_src) != 2) {
		printf("parse_resource failed\n");
		return -1;
	}

	p->file_src[DRM_PROP_NAME_LEN] = '\0';

	return 0;
}

static void parse_fill_patterns(char *arg)
{
	char *fill = strtok(arg, ",");
	if (!fill)
		return;
	primary_fill = util_pattern_enum(fill);
	fill = strtok(NULL, ",");
	if (!fill)
		return;
	secondary_fill = util_pattern_enum(fill);
}

static int page_flipping_supported(void)
{
	/*FIXME: generic ioctl needed? */
	return 1;
#if 0
	int ret, value;
	struct drm_i915_getparam gp;

	gp.param = I915_PARAM_HAS_PAGEFLIPPING;
	gp.value = &value;

	ret = drmCommandWriteRead(fd, DRM_I915_GETPARAM, &gp, sizeof(gp));
	if (ret) {
		fprintf(stderr, "drm_i915_getparam: %m\n");
		return 0;
	}

	return *gp.value;
#endif
}

static int cursor_supported(void)
{
	/*FIXME: generic ioctl needed? */
	return 1;
}

static drmModeModeInfo *connector_find_mode(struct device *dev, uint32_t con_id,
					    const char *mode_str, const unsigned int vrefresh)
{
	drmModeConnector *connector;
	drmModeModeInfo *mode;
	int i;

	connector = get_connector_by_id(dev, con_id);
	if (!connector || !connector->count_modes)
		return NULL;

	for (i = 0; i < connector->count_modes; i++) {
		mode = &connector->modes[i];
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

	return NULL;
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

static int pipe_find_crtc_and_mode(struct device *dev, struct pipe_arg *pipe)
{
	drmModeModeInfo *mode = NULL;
	int i;

	pipe->mode = NULL;

	for (i = 0; i < (int)pipe->num_cons; i++) {
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

static void add_property(struct device *dev, uint32_t obj_id, const char *name, uint64_t value)
{
	struct property_arg p;
	int ret = 0;

	p.obj_id = obj_id;

	ret = snprintf(p.name, sizeof(p.name), "%s", name);
	/* check */
	if (ret >= sizeof(p.name))
		printf("buffer overflow occurred.\n");

	p.value = value;

	set_property(dev, &p);
}

static void atomic_set_mode(struct device *dev, struct pipe_arg *pipes, unsigned int count)
{
	unsigned int i;
	unsigned int j;
	int ret;

	for (i = 0; i < count; i++) {
		struct pipe_arg *pipe = &pipes[i];

		ret = pipe_find_crtc_and_mode(dev, pipe);
		if (ret < 0)
			continue;
	}

	for (i = 0; i < count; i++) {
		struct pipe_arg *pipe = &pipes[i];
		uint32_t blob_id;

		if (pipe->mode == NULL)
			continue;

		printf("setting mode %s-%dHz on connectors ", pipe->mode_str, pipe->mode->vrefresh);
		for (j = 0; j < pipe->num_cons; ++j) {
			printf("%s, ", pipe->cons[j]);
			add_property(dev, pipe->con_ids[j], "CRTC_ID", pipe->crtc->crtc->crtc_id);
		}
		printf("crtc %d\n", pipe->crtc->crtc->crtc_id);

		drmModeCreatePropertyBlob(dev->fd, pipe->mode, sizeof(*pipe->mode), &blob_id);
		add_property(dev, pipe->crtc->crtc->crtc_id, "MODE_ID", blob_id);
		add_property(dev, pipe->crtc->crtc->crtc_id, "ACTIVE", 1);
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

static int atomic_set_plane(struct device *dev, struct plane_arg *p, int pattern, bool update)
{
	uint32_t handles[4] = { 0 }, pitches[4] = { 0 }, offsets[4] = { 0 };
	/* VSI updated */
	uint64_t modifiers[4] = { 0 };
	struct bo *plane_bo[4] = { NULL, NULL, NULL, NULL };
	uint32_t align_w, align_h;
	const struct util_format_info *info = NULL;
	int ret = 0;
	/* updated end */
	int crtc_x, crtc_y, crtc_w, crtc_h;
	struct crtc *crtc = NULL;
	unsigned int i;
	unsigned int old_fb_id;

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
	info = util_format_info_find(p->fourcc);
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
				   pattern, modifiers, plane_bo, NULL, 0);

		if (ret) {
			fprintf(stderr, "failed to create bo for plane %u\n", p->plane_id);
			return -1;
		}

		align_w = p->w;
		align_h = p->h;
		drm_vs_get_align_size(&align_w, &align_h, p->fourcc, p->modifier);

		for (i = 0; i < p->num_planes; i++) {
			handles[i] = plane_bo[i]->handle;
			pitches[i] = plane_bo[i]->pitch;
			offsets[i] = plane_bo[i]->offset;
		}

		if (drmModeAddFB2WithModifiers(dev->fd, align_w, align_h, p->fourcc, handles,
					       pitches, offsets, modifiers, &p->fb_id,
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

	return 0;
}

static void atomic_set_planes(struct device *dev, struct plane_arg *p, unsigned int count,
			      bool update)
{
	unsigned int i, pattern = primary_fill;

	/* set up planes */
	for (i = 0; i < count; i++) {
		if (atomic_set_plane(dev, &p[i], pattern, update))
			return;
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
	}
}

static void atomic_clear_FB(struct device *dev, struct plane_arg *p, unsigned int count)
{
	unsigned int i, j;

	for (i = 0; i < count; i++) {
		if (p[i].fb_id) {
			drmModeRmFB(dev->fd, p[i].fb_id);
			p[i].fb_id = 0;
		}
		if (p[i].old_fb_id) {
			drmModeRmFB(dev->fd, p[i].old_fb_id);
			p[i].old_fb_id = 0;
		}

		/* VSI updated */
		for (j = 0; j < p[i].num_planes; j++) {
			if (p[i].bo[j]) {
				bo_destroy(p[i].bo[j]);
				p[i].bo[j] = NULL;
			}

			if (p[i].old_bo[j]) {
				bo_destroy(p[i].old_bo[j]);
				p[i].old_bo[j] = NULL;
			}
		}
		/* updated end */
	}
}

static int bo_fb_create(int fd, unsigned int fourcc, const uint32_t w, const uint32_t h,
			enum util_fill_pattern pat, struct bo **out_bo, unsigned int *out_fb_id)
{
	uint32_t handles[4] = { 0 }, pitches[4] = { 0 }, offsets[4] = { 0 };
	struct bo *bo;
	unsigned int fb_id;

	bo = bo_create(fd, fourcc, w, h, handles, pitches, offsets, pat, 0);

	if (bo == NULL)
		return -1;

	if (drmModeAddFB2(fd, w, h, fourcc, handles, pitches, offsets, &fb_id, 0)) {
		fprintf(stderr, "failed to add fb (%ux%u): %s\n", w, h, strerror(errno));
		bo_destroy(bo);
		return -1;
	}
	*out_bo = bo;
	*out_fb_id = fb_id;
	return 0;
}

static void writeback_config(struct device *dev, struct pipe_arg *pipes, unsigned int count)
{
	drmModeConnector *connector = NULL;
	unsigned int i, j;

	for (j = 0; j < count; j++) {
		struct pipe_arg *pipe = &pipes[j];

		for (i = 0; i < pipe->num_cons; i++) {
			connector = get_connector_by_id(dev, pipe->con_ids[i]);
			if (!connector) {
				printf("The connector is not found by id.\n");
				return;
			}
			if (connector->connector_type == DRM_MODE_CONNECTOR_WRITEBACK) {
				bo_fb_create(dev->fd, pipes[j].fourcc, pipe->mode->hdisplay,
					     pipe->mode->vdisplay, plain_fill, pipe->out_bo,
					     &pipe->out_fb_id);
				add_property(dev, pipe->con_ids[i], "WRITEBACK_FB_ID",
					     pipe->out_fb_id);
			}
		}
	}
}

static void dump_output_fb(struct device *dev, struct pipe_arg *pipes, char *dump_path,
			   unsigned int count)
{
	drmModeConnector *connector;
	unsigned int i, j;

	for (j = 0; j < count; j++) {
		struct pipe_arg *pipe = &pipes[j];

		for (i = 0; i < pipe->num_cons; i++) {
			connector = get_connector_by_id(dev, pipe->con_ids[i]);
			if (!connector) {
				fprintf(stderr, "get_connector_by_id: %u failed\n",
					pipe->con_ids[i]);
				return;
			}
			if (connector->connector_type == DRM_MODE_CONNECTOR_WRITEBACK)
				bo_dump(pipe, dump_path, NULL, 0, false, false);
		}
	}
}

static void usage(char *name)
{
	fprintf(stderr, "usage: %s [-cDdefPpsCvw]\n", name);

	fprintf(stderr, "\n Query options:\n\n");
	fprintf(stderr, "\t-c\tlist connectors\n");
	fprintf(stderr, "\t-e\tlist encoders\n");
	fprintf(stderr, "\t-f\tlist framebuffers\n");
	fprintf(stderr, "\t-p\tlist CRTCs and planes (pipes)\n");

	fprintf(stderr, "\n Test options:\n\n");
	fprintf(stderr,
		"\t-P <plane_id>@<crtc_id>:<w>x<h>[+<x>+<y>][*<scale>][@<format>]\tset a plane\n");
	fprintf(stderr,
		"\t-s <connector_id>[,<connector_id>][@<crtc_id>]:<mode>[-<vrefresh>][@<format>]\tset a mode\n");
	/* VSI: for wb connector */
	fprintf(stderr, "\t-o <desired file path> \t Dump output buffer to file\n");
	/* VSI: json file with blob parameter */
	fprintf(stderr, "\t-j <json file path> \t Load a json file with blob parameter\n");
	/* end */
	fprintf(stderr, "\t-C\ttest hw cursor\n");
	fprintf(stderr, "\t-v\ttest vsynced page flipping\n");
	fprintf(stderr, "\t-planew <obj_id>:<prop_name>:<value>\tset property\n");
	fprintf(stderr, "\t-crtcw <obj_id>:<prop_name>:<value>\tset property\n");
	fprintf(stderr, "\t-F pattern1,pattern2\tspecify fill patterns\n");

	fprintf(stderr, "\n Generic options:\n\n");
	fprintf(stderr, "\t-d\tdrop master after mode set\n");
	fprintf(stderr, "\t-D device\tuse the given device\n");

	fprintf(stderr, "\n\tDefault is to dump all info.\n");
	exit(0);
}

int main(int argc, char **argv)
{
	struct device dev;
	int encoders = 0, connectors = 0, crtcs = 0, planes = 0, framebuffers = 0;
	int drop_master = 0;
	int test_vsync = 0;
	int test_cursor = 0;
	char *device = NULL;
	unsigned int i;
	unsigned int count = 0, plane_count = 0;
	unsigned int prop_count = 0, source_count = 0;
	struct pipe_arg *pipe_args = NULL;
	struct plane_arg *plane_args = NULL;
	struct property_arg *prop_args = NULL;
	struct resource_arg *source_args = NULL;
	unsigned int args = 0;
	int ret;
	int opt = 1;
	char *json_path = NULL;
	char *dump_path = NULL;

	memset(&dev, 0, sizeof dev);

	while (opt < argc) {
		const char *arg = argv[opt];
		args++;
		if (strcmp("-c", arg) == 0) {
			opt += 1;
		} else if (strcmp("-D", arg) == 0 && ((opt + 1) < argc)) {
			device = argv[opt + 1];
			opt += 2;
		} else if (strcmp("-d", arg) == 0) {
			drop_master = 1;
			opt += 1;
		} else if (strcmp("-e", arg) == 0) {
			encoders = 1;
			opt += 1;
		} else if (strcmp("-f", arg) == 0) {
			framebuffers = 1;
			opt += 1;
		} else if (strcmp("-F", arg) == 0 && ((opt + 1) < argc)) {
			parse_fill_patterns(argv[opt + 1]);
			opt += 2;
		} else if (strcmp("-o", arg) == 0 && ((opt + 1) < argc)) {
			/* VSI: for wb connector */
			//pipe_args->dump = true;
			dump_path = argv[opt + 1];
			opt += 2;
		} else if (strcmp("-j", arg) == 0 && ((opt + 1) < argc)) {
			/* VSI: json file with blob parameter */
			json_path = argv[opt + 1];
			opt += 2;
		} else if (strcmp("-P", arg) == 0 && ((opt + 1) < argc)) {
			plane_args = realloc(plane_args, (plane_count + 1) * sizeof *plane_args);
			if (plane_args == NULL) {
				fprintf(stderr, "memory allocation failed\n");
				return 1;
			}
			memset(&plane_args[plane_count], 0, sizeof(*plane_args));

			if (parse_plane(&plane_args[plane_count], argv[opt + 1]) < 0)
				usage(argv[0]);

			plane_count++;
			opt += 2;
		} else if (strcmp("-p", arg) == 0) {
			crtcs = 1;
			planes = 1;
			opt += 1;
		} else if (strcmp("-s", arg) == 0 && ((opt + 1) < argc)) {
			pipe_args = realloc(pipe_args, (count + 1) * sizeof *pipe_args);
			if (pipe_args == NULL) {
				fprintf(stderr, "memory allocation failed\n");
				return 1;
			}
			memset(&pipe_args[count], 0, sizeof(*pipe_args));
			pipe_args->dump = false;

			if (parse_connector(&pipe_args[count], argv[opt + 1]) < 0)
				usage(argv[0]);
			count++;
			opt += 2;
		} else if (strcmp("-C", arg) == 0) {
			test_cursor = 1;
			opt += 1;
		} else if (strcmp("-v", arg) == 0) {
			test_vsync = 1;
			opt += 1;
		} else if (strcmp("-planew", arg) == 0 && ((opt + 1) < argc)) {
			prop_args = realloc(prop_args, (prop_count + 1) * sizeof *prop_args);
			if (prop_args == NULL) {
				fprintf(stderr, "memory allocation failed\n");
				return 1;
			}
			memset(&prop_args[prop_count], 0, sizeof(*prop_args));

			if (parse_property(&prop_args[prop_count], argv[opt + 1]) < 0) {
				usage(argv[0]);
				fprintf(stderr, "Error: Parse property %s fail\n", argv[opt + 1]);
				return -1;
			}
			prop_count++;
			prop_args[prop_count - 1].obj_type = DRM_MODE_OBJECT_PLANE;
			opt += 2;
		} else if (strcmp("-crtcw", arg) == 0 && ((opt + 1) < argc)) {
			prop_args = realloc(prop_args, (prop_count + 1) * sizeof *prop_args);
			if (prop_args == NULL) {
				fprintf(stderr, "memory allocation failed\n");
				return 1;
			}
			memset(&prop_args[prop_count], 0, sizeof(*prop_args));

			if (parse_property(&prop_args[prop_count], argv[opt + 1]) < 0) {
				usage(argv[0]);
				fprintf(stderr, "Error: Parse property %s fail\n", argv[opt + 1]);
				return -1;
			}
			prop_count++;
			prop_args[prop_count - 1].obj_type = DRM_MODE_OBJECT_CRTC;
			opt += 2;
		} else if (strcmp("-connw", arg) == 0 && ((opt + 1) < argc)) {
			prop_args = realloc(prop_args, (prop_count + 1) * sizeof *prop_args);
			if (prop_args == NULL) {
				fprintf(stderr, "memory allocation failed\n");
				return 1;
			}
			memset(&prop_args[prop_count], 0, sizeof(*prop_args));

			if (parse_property(&prop_args[prop_count], argv[opt + 1]) < 0) {
				usage(argv[0]);
				fprintf(stderr, "Error: Parse property %s fail\n", argv[opt + 1]);
				return -1;
			}
			prop_count++;
			prop_args[prop_count - 1].obj_type = DRM_MODE_OBJECT_CONNECTOR;
			opt += 2;
		} else if (strcmp("-r", arg) == 0 && ((opt + 1) < argc)) {
			source_args =
				realloc(source_args, (source_count + 1) * sizeof(*source_args));
			if (source_args == NULL) {
				fprintf(stderr, "memory allocation failed\n");
				return 1;
			}
			memset(&source_args[source_count], 0, sizeof(*source_args));

			if (parse_resource(&source_args[source_count], argv[opt + 1]) < 0)
				usage(argv[0]);

			source_count++;
			opt += 2;
		} else if (strcmp("-h", arg) == 0) {
			usage(argv[0]);
			return 0;
		} else {
			fprintf(stderr, "Unknown argument %s\n", argv[opt]);
			if (plane_args)
				free(plane_args);
			if (pipe_args)
				free(pipe_args);
			if (prop_args)
				free(prop_args);
			if (source_args)
				free(source_args);
			return -1;
		}
	}

	if (!args || args == 1)
		encoders = connectors = crtcs = planes = framebuffers = 1;

	dev.fd = util_open(device, "vs-drm");
	if (dev.fd < 0) {
		if (plane_args)
			free(plane_args);
		if (pipe_args)
			free(pipe_args);
		if (prop_args)
			free(prop_args);
		if (source_args)
			free(source_args);
		return -1;
	}

	ret = drmSetClientCap(dev.fd, DRM_CLIENT_CAP_ATOMIC, 1);
	if (ret) {
		fprintf(stderr, "no atomic modesetting support: %s\n", strerror(errno));
		drmClose(dev.fd);
		if (plane_args)
			free(plane_args);
		if (pipe_args)
			free(pipe_args);
		if (prop_args)
			free(prop_args);
		if (source_args)
			free(source_args);
		return -1;
	}

	ret = drmSetClientCap(dev.fd, DRM_CLIENT_CAP_WRITEBACK_CONNECTORS, 1);
	if (ret) {
		fprintf(stderr, "no writeback connector support: %s\n", strerror(errno));
		drmClose(dev.fd);
		if (plane_args)
			free(plane_args);
		if (pipe_args)
			free(pipe_args);
		if (prop_args)
			free(prop_args);
		if (source_args)
			free(source_args);
		return -1;
	}

	if (test_vsync && !page_flipping_supported()) {
		fprintf(stderr, "page flipping not supported by drm.\n");
		drmClose(dev.fd);
		if (plane_args)
			free(plane_args);
		if (pipe_args)
			free(pipe_args);
		if (prop_args)
			free(prop_args);
		if (source_args)
			free(source_args);
		return -1;
	}

	if (test_vsync && !count) {
		fprintf(stderr, "page flipping requires at least one -s option.\n");
		drmClose(dev.fd);
		if (plane_args)
			free(plane_args);
		if (pipe_args)
			free(pipe_args);
		if (prop_args)
			free(prop_args);
		if (source_args)
			free(source_args);
		return -1;
	}

	if (test_cursor && !cursor_supported()) {
		fprintf(stderr, "hw cursor not supported by drm.\n");
		drmClose(dev.fd);
		if (plane_args)
			free(plane_args);
		if (pipe_args)
			free(pipe_args);
		if (prop_args)
			free(prop_args);
		if (source_args)
			free(source_args);
		return -1;
	}

	if (dump_path && pipe_args) {
		pipe_args->dump = true;
	}

	dev.resources = get_resources(&dev);
	if (!dev.resources) {
		drmClose(dev.fd);
		if (plane_args)
			free(plane_args);
		if (pipe_args)
			free(pipe_args);
		if (prop_args)
			free(prop_args);
		if (source_args)
			free(source_args);
		return 1;
	}

	for (i = 0; i < count; i++) {
		if (pipe_resolve_connectors(&dev, &pipe_args[i]) < 0) {
			free_resources(dev.resources);
			drmClose(dev.fd);
			if (plane_args)
				free(plane_args);
			if (pipe_args)
				free(pipe_args);
			if (prop_args)
				free(prop_args);
			if (source_args)
				free(source_args);
			return 1;
		}
	}

	remap_plane_obj_id(&dev, plane_args, plane_count);
	remap_pipe_obj_id(&dev, pipe_args, count);
	remap_property_obj_id(&dev, prop_args, prop_count);
	remap_resource_obj_id(&dev, source_args, source_count);

#define dump_resource(dev, res) \
	if (res)                \
	dump_##res(dev)

	dump_resource(&dev, encoders);
	dump_resource(&dev, connectors);
	dump_resource(&dev, crtcs);
	dump_resource(&dev, planes);
	dump_resource(&dev, framebuffers);

	dev.req = drmModeAtomicAlloc();

	for (i = 0; i < source_count; i++) {
		ret = set_resource(&source_args[i], plane_args, plane_count);
		if (ret) {
			free_resources(dev.resources);
			drmModeAtomicFree(dev.req);
			drmClose(dev.fd);
			if (source_args)
				free(source_args);
			return 1;
		}
	}

	if (json_path && prop_count > 0) {
		if (access(json_path, F_OK) != 0) {
			drmModeAtomicFree(dev.req);
			free_resources(dev.resources);
			drmClose(dev.fd);
			fprintf(stderr, "json_path is not exist\n");
			return 1;
		}
		property_blob_load(&dev, json_path, prop_count, prop_args);
	}

	if (count && plane_count) {
		uint64_t cap = 0;

		ret = drmGetCap(dev.fd, DRM_CAP_DUMB_BUFFER, &cap);
		if (ret || cap == 0) {
			drmModeAtomicFree(dev.req);
			free_resources(dev.resources);
			drmClose(dev.fd);
			fprintf(stderr, "driver doesn't support the dumb buffer API\n");
			return 1;
		}

		atomic_set_mode(&dev, pipe_args, count);

		/* VSI: for wb connector */
		writeback_config(&dev, pipe_args, count);

		atomic_set_planes(&dev, plane_args, plane_count, false);

		fprintf(stdout, "If need set test pattern or CRC through debugfs, please set.\n");
		getchar();

		ret = drmModeAtomicCommit(dev.fd, dev.req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
		if (ret) {
			fprintf(stderr, "Atomic Commit failed [1]\n");
			drmModeAtomicFree(dev.req);
			free_resources(dev.resources);
			drmClose(dev.fd);
			return 1;
		}

		gettimeofday(&pipe_args->start, NULL);
		pipe_args->swap_count = 0;

		while (test_vsync) {
			drmModeAtomicFree(dev.req);
			dev.req = drmModeAtomicAlloc();
			atomic_set_planes(&dev, plane_args, plane_count, true);

			ret = drmModeAtomicCommit(dev.fd, dev.req, DRM_MODE_ATOMIC_ALLOW_MODESET,
						  NULL);
			if (ret) {
				fprintf(stderr, "Atomic Commit failed [2]\n");
				drmModeAtomicFree(dev.req);
				free_resources(dev.resources);
				drmClose(dev.fd);
				return 1;
			}

			pipe_args->swap_count++;
			if (pipe_args->swap_count == 60) {
				struct timeval end;
				double t;

				gettimeofday(&end, NULL);
				t = end.tv_sec + end.tv_usec * 1e-6 -
				    (pipe_args->start.tv_sec + pipe_args->start.tv_usec * 1e-6);
				fprintf(stderr, "freq: %.02fHz\n", pipe_args->swap_count / t);
				pipe_args->swap_count = 0;
				pipe_args->start = end;
			}
		}

		if (drop_master)
			drmDropMaster(dev.fd);

		/* VSI: for wb connector */
		sleep(10);
		if (pipe_args->dump) {
			if (access(dump_path, F_OK) != 0) {
				drmModeAtomicFree(dev.req);
				free_resources(dev.resources);
				drmClose(dev.fd);
				fprintf(stderr, "dump_path is not exist\n");
				return 1;
			}
			dump_output_fb(&dev, pipe_args, dump_path, count);
		}
		getchar();

		drmModeAtomicFree(dev.req);
		dev.req = drmModeAtomicAlloc();

		atomic_clear_mode(&dev, pipe_args, count);
		atomic_clear_planes(&dev, plane_args, plane_count);
		ret = drmModeAtomicCommit(dev.fd, dev.req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
		if (ret) {
			drmModeAtomicFree(dev.req);
			fprintf(stderr, "Atomic Commit failed\n");
			free_resources(dev.resources);
			drmClose(dev.fd);
			return 1;
		}

		atomic_clear_FB(&dev, plane_args, plane_count);
	}

	drmModeAtomicFree(dev.req);
	free_resources(dev.resources);
	drmClose(dev.fd);
	if (prop_args)
		free(prop_args);
	if (plane_args)
		free(plane_args);
	if (pipe_args)
		free(pipe_args);
	if (source_args)
		free(source_args);

	return 0;
}
