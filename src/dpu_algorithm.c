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

#include <stdlib.h>
#include <string.h>
#include "dpu_algorithm.h"

dtest_status dpu_algorithm_lib_init(struct dpu_algorithm **algo)
{
	struct dpu_algorithm *dpu_algo = calloc(1, sizeof(struct dpu_algorithm));
	if (!dpu_algo)
		return DTEST_FAILURE;
	dpu_algo->funcs = calloc(1, sizeof(struct pq_helper_funcs));
	if (!dpu_algo->funcs) {
		printf("Memory allocation failure.\n");
		if (dpu_algo != NULL)
			free(dpu_algo);
		return DTEST_FAILURE;
	}

	dpu_algo->lib = dlopen(LIB_DPU_ALGORITHM_PATH, RTLD_LAZY | RTLD_LOCAL);

#define UPDATE_DLSYM(func_name) dlsym(dpu_algo->lib, func_name)

	struct pq_helper_funcs pq_funcs = {
		.dpu_algo_hdr = UPDATE_DLSYM("dpu_algo_api_hdr"),
		.dpu_algo_ltm = UPDATE_DLSYM("dpu_algo_api_ltm"),
		.dpu_algo_regamma = UPDATE_DLSYM("dpu_algo_api_regamma"),
		.dpu_algo_gtm = UPDATE_DLSYM("dpu_algo_api_gtm"),
		.dpu_algo_degamma = UPDATE_DLSYM("dpu_algo_api_degamma"),
		.dpu_algo_scl = UPDATE_DLSYM("dpu_algo_api_scl")
	};

	memcpy(dpu_algo->funcs, &pq_funcs, sizeof(struct pq_helper_funcs));
	*algo = dpu_algo;

	return DTEST_SUCCESS;
}

void dpu_algorithm_lib_deinit(struct dpu_algorithm *algo)
{
	if (!algo) {
		printf("algo pointer is NULL!\n");
		return;
	}
	if (algo->lib != NULL)
		dlclose(algo->lib);
	if (algo->funcs != NULL)
		free(algo->funcs);
	if (algo != NULL)
		free(algo);
}
