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

#ifndef __DRM_TEST_H__
#define __DRM_TEST_H__

#include <sys/time.h>

#include "type.h"
#include "xf86drmMode.h"
#include "json/cJSON.h"

#include "dpu_algorithm.h"
#include "vs_bo_helper.h"
#include "util/format.h"

#define fourcc_mod_vs_get_type(val) (((val)&DRM_FORMAT_MOD_VS_TYPE_MASK) >> 53)
#define fourcc_mod_vs_is_compressed(val) \
	!!(fourcc_mod_vs_get_type(val) == DRM_FORMAT_MOD_VS_TYPE_COMPRESSED ? 1 : 0)

struct crtc {
	drmModeCrtc *crtc;
	drmModeObjectProperties *props;
	drmModeModeInfo *mode;
	drmModePropertyRes **props_info;
	struct drm_vs_dc_info info; /* to store the DC info */
};

struct encoder {
	drmModeEncoder *encoder;
};

struct connector {
	drmModeConnector *connector;
	drmModeObjectProperties *props;
	drmModePropertyRes **props_info;
	char *name;
};

struct fb {
	drmModeFB *fb;
};

struct plane {
	drmModePlane *plane;
	drmModeObjectProperties *props;
	drmModePropertyRes **props_info;
};

struct resources {
	drmModeRes *res;
	drmModePlaneRes *plane_res;

	struct crtc *crtcs;
	struct encoder *encoders;
	struct connector *connectors;
	struct fb *fbs;
	struct plane *planes;
};

/* RAM less releated args */
struct ramless {
	uint64_t ramless_fb_id;
	bool fb_inuse;
};

/* Writeback with stripe mode releated args */
struct wb_stripe {
	uint64_t fb_id;
	uint32_t num_planes;
	drm_vs_bo_param bo_param[4];
	struct bo *out_bo[4];
	uint64_t wb_modifier;
	unsigned int fourcc;
};

/* dbi releated bos */
struct dbi_bo {
	struct bo *dbi_lut_main_bo;
	struct bo *dbi_lut_udc0_bo;
	struct bo *dbi_lut_udc1_bo;
	struct bo *dbi_collect_bo;
};

typedef struct device {
	int fd;
	uint32_t frame_id;
	struct resources *resources;

	struct {
		unsigned int width;
		unsigned int height;

		unsigned int fb_id;
		struct bo *bo;
		struct bo *cursor_bo;
	} mode;

	drmModeAtomicReq *req;

	struct dpu_algorithm *dpu_algorithm;
	struct bo *ltm_hist_bo1;
	struct bo *ltm_hist_bo2;
	struct bo *hist_bo;
	struct bo *histogram_bo[VS_HIST_IDX_COUNT];
	struct ramless ramless;
	struct wb_stripe wb_stripe;
	struct dbi_bo dbi_bo;
	uint32_t *ctx_id_list;
	bool use_vcmd;
	bool is_qemu;
	bool use_vdp;
	void *vdp_buffer_logic;
} device_t;

typedef struct pipe_arg {
	char **cons;
	uint32_t *con_ids;
	uint32_t *con_ids_ori; /* for stream test: save the initial connector id after parsing case */
	unsigned int num_cons;
	uint32_t crtc_id;
	uint32_t crtc_id_ori; /* for stream test: save the initial crtc id after parsing case */
	bool global_disable;
	char mode_str[64];
	char format_str[15];
	unsigned int vrefresh;
	bool vrr_enable;
	unsigned int vrr_refresh;
	unsigned int fourcc;
	drmModeModeInfo *mode;
	struct crtc *crtc;
	unsigned int fb_id[2], current_fb_id;
	struct timeval start;

	int swap_count;

	/* for wb_connector */
	bool wb_yuv444_10bit;
	unsigned int out_fb_id;
	bool wb_for_ramless;
	bool wb_with_stripe;
	uint32_t num_planes;
	drm_vs_bo_param bo_param[4];
	struct bo *out_bo[4];
	bool dump;
	char *md5_golden;
	char *md5_result;
	uint32_t wb_w, wb_h;
	uint64_t wb_modifier;

	/* load stream */
	uint32_t frame_idx;
	enum util_yuv_order order;
	bool stream;
} pipe_t;

typedef struct plane_arg {
	uint32_t plane_id; /* the id of plane to use */
	uint32_t plane_id_ori; /* for stream test: save the initial plane id after parsing case */
	bool global_disable;
	uint32_t crtc_id; /* the id of CRTC to bind to */
	uint32_t crtc_id_ori; /* for stream test: save the initial crtc id after parsing case */
	bool has_position;
	int32_t x, y;
	uint32_t w, h;
	int32_t crtc_x, crtc_y;
	uint32_t crtc_w, crtc_h;
	double scale;
	unsigned int fb_id;
	unsigned int old_fb_id;
	char format_str[20]; /* need to leave room for terminating \0 */
	unsigned int fourcc;
	uint8_t color_encoding; /* just vaild for YUV format */
	uint8_t color_range; /* just vaild for YUV format */

	/* updated */
	struct bo *bo[4];
	struct bo *old_bo[4];
	uint64_t modifier;
	uint32_t num_planes;
	char file_src[4][RESOURCE_LEN + 1]; /* file_src[0] for resource filename,
										 file_src[1-3] for DEC400 tile status buffer filename */
	uint32_t rotation;
	uint32_t std_blend_mode;
	uint32_t std_blend_alpha;
	uint32_t zpos;
	struct dc_dec400_fc dec400_fc;
} plane_t;

typedef struct stream_arg {
	uint32_t w, h;
	char file_src[4][RESOURCE_LEN + 1];
	char format_str[5];
	unsigned int fourcc;
	uint64_t modifier;
	uint32_t num_frames;
} stream_arg_t;

typedef struct cursor_arg {
	uint32_t cursor_id; /* the id of cursor plane to use */
	uint32_t crtc_id; /* the id of CRTC to bind to */
	int32_t x, y;
	uint32_t w, h;
	uint32_t hot_x, hot_y;
	char format_str[5]; /* need to leave room for terminating \0 */
	unsigned int fourcc;
	struct bo *bo[4];
} cursor_t;

typedef struct resource_arg {
	uint32_t plane_id;
	char file_src[RESOURCE_LEN + 1];
} resource_t;

typedef struct property_arg {
	uint32_t obj_id;
	uint32_t obj_id_ori; /* for stream test: save the initial obj id after parsing case */
	uint32_t obj_type;
	char name[DRM_PROP_NAME_LEN + 1];
	uint32_t prop_id;
	uint64_t value;
	bool optional;
	cJSON *json_obj;
} property_t;

#endif
