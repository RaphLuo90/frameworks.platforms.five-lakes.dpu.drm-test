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

#ifndef __DPU_ALGORITHM_H__
#define __DPU_ALGORITHM_H__

#include <stdio.h>
#include <stdint.h>
#include <dlfcn.h>

#include "type.h"

#define LIB_DPU_ALGORITHM_PATH "./../dpu-algo/lib/libdpualgorithm.so"

struct pq_helper_funcs {
	dtest_status (*dpu_algo_ltm)(uint32_t fr, uint32_t mode, uint32_t vid_mode,
				     uint32_t Strength, uint32_t layerWidth, uint32_t layerHeight,
				     uint32_t gridWidth, uint32_t gridHeight, double sensitivity,
				     double iir_max_wgt, uint16_t *spatial_kernal,
				     uint32_t (*hist_buf)[64], uint32_t (*cdf_crv_last)[65],
				     void *hw_config);
	dtest_status (*dpu_algo_gtm)(uint32_t fr, uint32_t mode, uint32_t vid_mode,
				     uint32_t Strength, uint32_t layerWidth, uint32_t layerHeight,
				     void *hw_config);
	dtest_status (*dpu_algo_hdr)(void *algo_config, void *hw_config);
	dtest_status (*dpu_algo_regamma)(uint32_t mode, char *lut_file_r, char *lut_file_g,
					 char *lut_file_b, void *hw_config, bool enable);
	dtest_status (*dpu_algo_degamma)(uint32_t mode, char *lut_file, void *hw_config,
					 bool enable);
	dtest_status (*dpu_algo_scl)(uint32_t scaler_mode, const uint32_t srcWidth,
				     const uint32_t srcHeight, const uint32_t dstWidth,
				     const uint32_t dstHeight, void *hw_config);
};

struct dpu_algorithm {
	void *lib;

	struct pq_helper_funcs *funcs;
};

dtest_status dpu_algorithm_lib_init(struct dpu_algorithm **algo);

void dpu_algorithm_lib_deinit(struct dpu_algorithm *algo);

#endif
