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
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "util/common.h"
#include "util/format.h"
#include "util/kms.h"
#include "util/log.h"
#include "util/error.h"

#include "xf86drm.h"

#include "buffers.h"
#include "vs_bo_helper.h"
#include "drmtest_helper.h"

struct device_config {
	uint64_t type;
	uint64_t value;
};

dtest_status dpu_load_frames(cJSON *root, dtest_frame_t *dtest_frames,
			     dtest_stream_t *dtest_streams, bool *is_stream);

dtest_status initialize_virtual_display(device_t *dev, uint32_t vdp_out_addr);

dtest_status dpu_clear_all_pipes(device_t *dev, dtest_frame_t *dtest_frames);

uint64_t get_crtc_property_init_value(device_t *dev, char *props_name);

uint64_t get_crtc_property_default_value(device_t *dev, uint32_t crtc_id, char *props_name);

uint64_t get_plane_property_default_value(device_t *dev, uint32_t plane_id, char *props_name);

uint64_t get_connector_property_default_value(device_t *dev, uint32_t connector_id,
					      char *props_name);

dtest_status dpu_clear_all_properties(device_t *dev, dtest_frame_t *dtest_frames);

dtest_status dpu_set_frame(device_t *dev, frame_t *frame);

int dpu_open_device(const char *device, const char *module);

struct device_config *dpu_create_device_config(uint64_t type[], uint64_t value[], int len_type,
					       int len_value);

int dpu_load_device_config(int fd, struct device_config *config, int len_device_config);

struct resources *dpu_get_device_resources(device_t *dev);

int dpu_get_time_of_day(dtest_pipe_t *pipes);

dtest_status dpu_wait_wb_frm_done(device_t *dev, int conn_index);

dtest_status dpu_wait_vblank(device_t *dev, int crtc_index);

dtest_status dpu_wait_signal(device_t *dev, dtest_pipe_t *dtest_pipe);

dtest_status dpu_dump_output_fbs(device_t *dev, dtest_pipe_t *dtest_pipe, const char *dump_folder,
				 const char *case_name, FILE **stream_fp, uint32_t *pos_frame,
				 bool save_raw, uint32_t frame_id, bool check, uint32_t *run_count,
				 uint32_t *stream_id);

dtest_status dpu_clear_frame(device_t *dev, frame_t *frame);

void dpu_clear_frame_FB(struct device *dev, frame_t *frame);

int dpu_device_software_reset(int fd, enum drm_vs_reset_mode mode);
int dpu_device_software_clean(int fd, enum drm_vs_sw_reset_mode mode);

void dpu_free_frames(dtest_frame_t *dtest_frames);

void dpu_free_resources(struct resources *res);

int dpu_close_device(int fd);

void dpu_free_device(struct device *dev);

dtest_status dpu_create_wb_plane_buffer(struct device *dev, frame_t *frame);

void calcu_md5(const char *filename, char *md5);
