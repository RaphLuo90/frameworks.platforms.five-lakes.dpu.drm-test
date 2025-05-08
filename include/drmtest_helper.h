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

#ifndef __DRMTEST_HELPER_H__
#define __DRMTEST_HELPER_H__

#include "drmtest.h"
#include "drm/vs_drm.h"
#include <drm/vs_drm_fourcc.h>
#include "json/cJSON.h"
#include "case_builder.h"
#include <signal.h>
#define VS_VCMD_EXCEPTION_SIGNAL SIGUSR1
#define VS_DC_EXCEPTION_SIGNAL SIGUSR2

#define BLEND_PREMULTI 0
#define BLEND_COVERAGE 1
#define BLEND_PIXEL_NONE 2

#define BLEND_ALPHA_OPAQUE 0xffff

#define COLOR_YCBCR_BT601 0
#define COLOR_YCBCR_BT709 1
#define COLOR_YCBCR_BT2020 2

#define COLOR_YCBCR_LIMITED_RANGE 0
#define COLOR_YCBCR_FULL_RANGE 1

#define AXI_BUS_CLEAN_DONE 0
#define AXI_BUS_CLEAN_TIMEOUT 1
#define AXI_RD_BUS_HANG 2
#define AXI_RD_BUS_ERROR 3
#define AXI_WR_BUS_HANG 4
#define AXI_WR_BUS_ERROR 5
#define APB_BUS_HANG 6

#define VCMD_BUS_ERROR 8
#define VCMD_TIMEOUT 9
#define VCMD_CMD_ERR 10

#ifndef container_of
#define container_of(ptr, type, member)                            \
	({                                                         \
		const typeof(((type *)0)->member) *__mptr = (ptr); \
		(type *)((char *)__mptr - offsetof(type, member)); \
	})
#endif

dtest_status set_property(struct device *dev, struct property_arg *p);

dtest_status __remap_connector(const struct device *dev, uint32_t *pid);

dtest_status remap_plane_obj_id(const struct device *dev, struct plane_arg *args, uint32_t count);

dtest_status remap_pipe_obj_id(const struct device *dev, struct pipe_arg *args, uint32_t count);

dtest_status remap_resource_obj_id(const struct device *dev, struct resource_arg *args,
				   uint32_t count);

dtest_status remap_property_obj_id(const struct device *dev, struct property_arg *prop_args,
				   uint32_t prop_count);

dtest_status dtest_remap_obj_id(device_t *dev, frame_t *frame);

bool dtest_get_obj_stream(cJSON *obj, const char *name);

bool dtest_get_obj_global_disable(cJSON *obj, const char *name);

bool dtest_get_bool_obj_by_name(cJSON *obj, const char *name);

int dtest_get_obj_int(cJSON *obj, const char *name);

double dtest_get_obj_double(cJSON *obj, const char *name);

char *dtest_get_obj_string(cJSON *obj, const char *name);

bool dtest_get_obj_vrr_enable(cJSON *obj, const char *name);

int dtest_get_obj_vrr_refresh(cJSON *obj, const char *name);

uint8_t dtest_get_obj_dec_type(cJSON *obj, const char *name);

uint8_t dtest_get_obj_normal_tilemode(cJSON *obj, const char *name);

uint8_t dtest_get_obj_dec_tilemode(cJSON *obj, const char *name);

uint8_t dtest_get_obj_pvric_tilemode(cJSON *obj, const char *name);

uint8_t dtest_get_obj_decnano_tilemode(cJSON *obj, const char *name);

uint8_t dtest_get_obj_etc2_tilemode(cJSON *obj, const char *name);

void dtest_get_obj_normal_modifier(cJSON *obj, uint64_t *mod);

void dtest_get_obj_dec_modifier(cJSON *obj, uint64_t *mod);

void dtest_get_obj_pvric_modifier(cJSON *obj, uint64_t *mod);

void dtest_get_obj_decnano_modifier(cJSON *obj, uint64_t *mod);

void dtest_get_obj_etc2_modifier(cJSON *obj, uint64_t *mod);

uint64_t dtest_get_obj_dec_fb_modifier(cJSON *obj, const char *name);

uint16_t dtest_get_obj_std_blend_mode(cJSON *obj, const char *name);

uint32_t dtest_get_obj_std_blend_alpha(cJSON *obj, const char *name);

uint32_t dtest_get_obj_zpos(cJSON *obj, const char *name);

uint8_t dtest_get_obj_color_encoding(cJSON *obj, const char *name);

uint8_t dtest_get_obj_color_range(cJSON *obj, const char *name);

size_t dtest_get_obj_str(cJSON *obj, const char *name, char *buf);

uint32_t dtest_get_obj_format(cJSON *obj, char *format_str);

uint32_t dtest_get_obj_rotation(cJSON *obj, char *name);

int dtest_get_feature_cap(int fd, enum drm_vs_feature_cap_type cap_type, int32_t *cap);

int dtest_get_plane_valid_rot(device_t *dev, uint32_t id, uint32_t *cap);

int dtest_get_connector_possible_crtcs(device_t *dev, uint32_t id, uint32_t *cap);

void **dtest_get_crtc_info(device_t *dev, uint32_t id);

void **dtest_get_plane_info(device_t *dev, uint32_t id);

void dtest_free_info_blob(void **info);

int dtest_get_hist_info(device_t *dev, frame_t *frame);
int dtest_get_ops_result(device_t *dev, frame_t *frame);
int dtest_get_histogram_result(device_t *dev, frame_t *frame);

int dtest_soft_reset(int fd, enum drm_vs_reset_mode mode);

int dtest_get_csc_coef_from_csv(const char *fileName, uint32_t csc_bit, __s32 *coef);

cJSON *dtest_load_json(const char *path);

uint64_t ext_fb_create(struct device *dev, char file_src[4][RESOURCE_LEN + 1], unsigned int fourcc,
		       uint64_t modifier, uint32_t num_planes, uint32_t width, uint32_t height);
void dtest_get_dec_decoder_fc(plane_t *plane, uint32_t id, dtest_property_t properties);

int dtest_init_excp_notif(struct device *dev);

#endif
