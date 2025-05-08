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
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "util/common.h"
#include "util/log.h"
#include "drmtest_helper.h"
#include "prop_mapper.h"
#include "map_funcs/prop_map_funcs.h"

#define PROP_ITEM(_name, _type, _fmap)                      \
	{                                                   \
		.name = _name, .type = _type, .map = _fmap, \
	}

#define PROP_BOOL_ITEM(name) PROP_ITEM(name, PROP_TYPE_BOOL, map_bool)
#define PROP_BLOB_ITEM(name, fmap) PROP_ITEM(name, PROP_TYPE_BLOB, fmap)
#define PROP_VALUE_U64_ITEM(name, fmap) PROP_ITEM(name, PROP_TYPE_UINT64, fmap)
#define PROP_ENUM_ITEM(name, fmap) PROP_ITEM(name, PROP_TYPE_ENUM, fmap)

enum prop_type {
	PROP_TYPE_BLOB = 0,
	PROP_TYPE_BOOL = 1,
	PROP_TYPE_ENUM = 2,
	PROP_TYPE_INT32 = 3,
	PROP_TYPE_UINT32 = 4,
	PROP_TYPE_UINT64 = 5,
};

struct prop_map {
	char name[DRM_PROP_NAME_LEN];
	enum prop_type type;
	uint64_t (*map)(cJSON *obj, struct device *dev);
};

static const struct prop_map prop_table[] = {
	PROP_VALUE_U64_ITEM("EXT_LAYER_FB", map_ext_layer_fb),
	PROP_BLOB_ITEM("DMA_CONFIG", map_dma_config),
	PROP_BLOB_ITEM("LINE_PADDING", map_line_padding),
	PROP_VALUE_U64_ITEM("UP_SAMPLE", map_up_sample),
	PROP_BLOB_ITEM("Y2R_CONFIG", map_y2r_csc),
	PROP_BOOL_ITEM("HDR"),
	PROP_BOOL_ITEM("SYNC_ENABLED"),
	PROP_BOOL_ITEM("LTM_ENABLE"),
	PROP_BOOL_ITEM("GTM_ENABLE"),
	PROP_BOOL_ITEM("DE_MULTIPLY"),
	PROP_BOOL_ITEM("DITHER_ENABLED"),
	PROP_BLOB_ITEM("EOTF", map_xstep_lut),
	PROP_BLOB_ITEM("OETF", map_xstep_lut),
	PROP_BLOB_ITEM("BLD_EOTF", map_xstep_lut),
	PROP_BLOB_ITEM("BLD_OETF", map_xstep_lut),
	PROP_BLOB_ITEM("DEGAMMA", map_xstep_lut),
	PROP_BLOB_ITEM("DEGAMMA_SR", map_drm_degamma),
	PROP_BLOB_ITEM("TONE_MAP", map_tone_map),
	PROP_BLOB_ITEM("HDR_UNION", map_drm_hdr),
	PROP_BLOB_ITEM("GAMMA_LUT", map_gamma_lut),
	PROP_BLOB_ITEM("PRIOR_GAMMA", map_regamma_lut),
	PROP_BLOB_ITEM("ROI0_GAMMA", map_regamma_lut),
	PROP_BLOB_ITEM("PRIOR_3DLUT", map_prior_3dlut),
	PROP_BLOB_ITEM("ROI0_3DLUT", map_roi_3dlut),
	PROP_BLOB_ITEM("ROI1_3DLUT", map_roi_3dlut),
	PROP_BLOB_ITEM("BLD_GAMUT_MAP", map_bld_lnr_ccm),
	PROP_BLOB_ITEM("BLD_NONLINEAR_GAMUT_MAP", map_bld_nonlnr_ccm),
	PROP_BLOB_ITEM("GAMUT_MAP", map_fe_ccm),
	PROP_BLOB_ITEM("R2Y", map_r2y_config),
	PROP_BLOB_ITEM("DOWN_SAMPLE", map_ds_config),
	PROP_BLOB_ITEM("SCALER", map_scale_config),
	PROP_BLOB_ITEM("CCM_LNR", map_be_lnr_ccm),
	PROP_BLOB_ITEM("CCM_NON_LNR", map_be_nonlnr_ccm),
	PROP_BLOB_ITEM("LAYER_CCM", map_layer_ccm),
	PROP_BLOB_ITEM("BRIGHTNESS", map_brightness),
	PROP_BLOB_ITEM("BLD_DITHER", map_blender_dither),
	PROP_BLOB_ITEM("LLV_DITHER", map_llv_dither),
	PROP_BLOB_ITEM("GAMMA_DITHER", map_dither),
	PROP_BLOB_ITEM("WB_GAMMA_DITHER", map_wb_dither),
	PROP_BLOB_ITEM("PANEL_DITHER", map_panel_dither),
	PROP_BLOB_ITEM("BLUR", map_drm_blur),
	PROP_BLOB_ITEM("RCD", map_drm_rcd),
	PROP_BLOB_ITEM("LTM_DEGAMMA", map_drm_ltm_degamma),
	PROP_BLOB_ITEM("LTM_GAMMA", map_drm_ltm_gamma),
	PROP_BLOB_ITEM("LTM_LUMA", map_drm_ltm_luma),
	PROP_BLOB_ITEM("LTM_FREQ_DECOMP", map_drm_ltm_freq_decomp),
	PROP_BLOB_ITEM("LTM_LUMA_ADJ", map_drm_ltm_luma_adj),
	PROP_BLOB_ITEM("LTM_GRID", map_drm_ltm_grid),
	PROP_BLOB_ITEM("LTM_AF_FILTER", map_drm_ltm_af_filter),
	PROP_BLOB_ITEM("LTM_AF_SLICE", map_drm_ltm_af_slice),
	PROP_BLOB_ITEM("LTM_AF_TRANS", map_drm_ltm_af_trans),
	PROP_BLOB_ITEM("LTM_TONE_ADJ", map_drm_ltm_tone_adj),
	PROP_BLOB_ITEM("LTM_COLOR", map_drm_ltm_color),
	PROP_BLOB_ITEM("LTM_DITHER", map_drm_ltm_dither),
	PROP_BLOB_ITEM("LTM_LUMA_AVE_SET", map_drm_ltm_luma_ave_set),
	PROP_BLOB_ITEM("LTM_HIST_CD_SET", map_drm_ltm_hist_cd_set),
	PROP_BLOB_ITEM("LTM_LOCAL_HIST_SET", map_drm_ltm_local_hist_set),
	PROP_BLOB_ITEM("LTM_DOWN_SCALER", map_drm_ltm_down_scaler),
	PROP_BLOB_ITEM("LTM", map_drm_ltm),
	PROP_BLOB_ITEM("GTM_DEGAMMA", map_drm_ltm_degamma),
	PROP_BLOB_ITEM("GTM_GAMMA", map_drm_ltm_gamma),
	PROP_BLOB_ITEM("GTM_LUMA", map_drm_ltm_luma),
	PROP_BLOB_ITEM("GTM_TONE_ADJ", map_drm_ltm_tone_adj),
	PROP_BLOB_ITEM("GTM_COLOR", map_drm_ltm_color),
	PROP_BLOB_ITEM("GTM_DITHER", map_drm_ltm_dither),
	PROP_BLOB_ITEM("GTM_LUMA_AVE_SET", map_drm_ltm_luma_ave_set),
	PROP_BLOB_ITEM("GTM_DOWN_SCALER", map_drm_ltm_down_scaler),
	PROP_BLOB_ITEM("GTM", map_drm_gtm),
	PROP_BLOB_ITEM("SHARPNESS", map_drm_sharpness),
	PROP_BLOB_ITEM("SHARPNESS_CSC", map_drm_sharpness_csc),
	PROP_BLOB_ITEM("SHARPNESS_LUMA_GAIN", map_drm_sharpness_luma_gain),
	PROP_BLOB_ITEM("SHARPNESS_LPF", map_drm_sharpness_lpf),
	PROP_BLOB_ITEM("SHARPNESS_LPF_NOISE", map_drm_sharpness_lpf_noise),
	PROP_BLOB_ITEM("SHARPNESS_LPF_CURVE", map_drm_sharpness_lpf_curve),
	PROP_BLOB_ITEM("SHARPNESS_COLOR_ADAPTIVE", map_drm_sharpness_color_adaptive),
	PROP_BLOB_ITEM("SHARPNESS_COLOR_BOOST", map_drm_sharpness_color_boost),
	PROP_BLOB_ITEM("SHARPNESS_SOFT_CLIP", map_drm_sharpness_soft_clip),
	PROP_BLOB_ITEM("SHARPNESS_DITHER", map_drm_sharpness_dither),
	PROP_BLOB_ITEM("CLEAR", map_color),
	PROP_BLOB_ITEM("BG_COLOR", map_color),
	PROP_BLOB_ITEM("COLORKEY", map_colorkey),
	PROP_BLOB_ITEM("SBS", map_side_by_side),
	PROP_BLOB_ITEM("BLEND_MODE", map_blend_mode),
	PROP_BLOB_ITEM("HIST", map_hist),
	PROP_BLOB_ITEM("HIST_ROI", map_hist_roi),
	PROP_BLOB_ITEM("HIST_BLOCK", map_hist_block),
	PROP_BLOB_ITEM("HIST_PROT", map_hist_prot),
	PROP_BLOB_ITEM("HIST_INFO", map_hist_info),
	PROP_BLOB_ITEM("OPS", map_ops),
	PROP_ENUM_ITEM("WB_POINT", map_wb_point),
	PROP_BLOB_ITEM("WB_SPLITER", map_wb_spliter),
	PROP_BLOB_ITEM("WB_STRIPE_POS", map_wb_stripe_position),
	PROP_BLOB_ITEM("WB_DITHER", map_wb_dither),
	PROP_VALUE_U64_ITEM("WB_ROTATION", map_wb_rotation),
	PROP_VALUE_U64_ITEM("WB_CROP", map_panel_crop),
	PROP_BLOB_ITEM("DSC", map_dsc),
	PROP_BLOB_ITEM("VDCM", map_vdc),
	PROP_BLOB_ITEM("SPLITER", map_spliter),
	PROP_VALUE_U64_ITEM("RAMLESS_FB", map_ramless_fb),
	PROP_BLOB_ITEM("PVRIC_CLEAR", map_pvric_clear),
	PROP_BLOB_ITEM("PVRIC_CONST", map_pvric_const),
	PROP_ENUM_ITEM("DEGAMMA_CONFIG", map_degamma_config),
	PROP_BLOB_ITEM("VRR_REFRESH", map_vrr_refresh),
	PROP_BLOB_ITEM("FREE_SYNC", map_free_sync),
	PROP_BLOB_ITEM("DP_SYNC", map_dp_sync),
	PROP_BLOB_ITEM("SPLICE_MODE", map_splice_mode),
	PROP_BLOB_ITEM("PANEL_CROP", map_panel_crop),
	PROP_BLOB_ITEM("LAYER_CROP", map_panel_crop),
	PROP_BLOB_ITEM("DATA_EXTEND", map_data_extend),
	PROP_BLOB_ITEM("DATA_TRUNC", map_data_trunc),
	PROP_BLOB_ITEM("CTM", map_ctm),
	PROP_BLOB_ITEM("SR2000", map_sr2000),
	PROP_BLOB_ITEM("MULTI_EXT_LAYER", map_multi_extend_layer),
	PROP_BLOB_ITEM("DEC_DECODER_FC", map_dec_fc),
	PROP_BLOB_ITEM("CTX_ID", map_ctx_id),
	PROP_BLOB_ITEM("LAYER_DEGAMMA", map_xstep_lut),
	PROP_BLOB_ITEM("WB_GAMMA", map_regamma_lut),
	PROP_BLOB_ITEM("HISTOGRAM_0", map_histogram0),
	PROP_BLOB_ITEM("HISTOGRAM_1", map_histogram1),
	PROP_BLOB_ITEM("HISTOGRAM_2", map_histogram2),
	PROP_BLOB_ITEM("HISTOGRAM_3", map_histogram3),
	PROP_BLOB_ITEM("HISTOGRAM_RGB", map_histogram_rgb),
	PROP_BLOB_ITEM("CCM_LNR_EX", map_be_lnr_ccm_ex),
	PROP_ENUM_ITEM("COLOR_CALIBRATION_PATH_MODE", map_color_calibration_path_mode),
	PROP_BLOB_ITEM("LBOX", map_lbox),
	PROP_BLOB_ITEM("DBI", map_dbi),
	PROP_BLOB_ITEM("DBI_CORRECTION_COEF", map_dbi_correction_coef),
};

void property_blob_free(struct device *dev, const uint32_t prop_count,
			struct property_arg *prop_args)
{
	uint32_t i, j;
	for (i = 0; i < prop_count; ++i) {
		for (j = 0; j < ARRAY_SIZE(prop_table); j++) {
			const struct prop_map *prop = &prop_table[j];
			if (strncmp(prop_args[i].name, prop->name, DRM_PROP_NAME_LEN + 1) == 0) {
				drmModeDestroyPropertyBlob(dev->fd, (uint32_t)prop_args[i].value);
			}
		}
	}
}

int property_blob_load(struct device *dev, const char *path, const uint32_t prop_count,
		       struct property_arg *prop_args)
{
	uint32_t i, j;
	cJSON *property = NULL;
	cJSON *root = dtest_load_json(path);
	if (NULL == root) {
		printf("Load json file fail\n");
		return -1;
	}
	property = cJSON_GetObjectItem(root, "property");

	for (i = 0; i < prop_count; i++) {
		char *prop_name = prop_args[i].name;
		for (j = 0; j < ARRAY_SIZE(prop_table); j++) {
			/* Create a blob if needed. */
			const struct prop_map *prob = &prop_table[j];
			if (strncmp(prop_name, prob->name, DRM_PROP_NAME_LEN + 1) == 0) {
				cJSON *obj = cJSON_GetObjectItem(property, prop_name);
				if (obj) {
					printf("Load %s param from %s\n", prob->name, path);
					prop_args[i].value = prob->map(obj, dev);
				}
			}
		}
		set_property(dev, &prop_args[i]);
	}

	cJSON_Delete(root);
	return 0;
}

dtest_status dtest_prop_map(device_t *dev, property_t *prob)
{
	uint32_t i, prob_value;
	dtest_status status;
	for (i = 0; i < ARRAY_SIZE(prop_table); i++) {
		const struct prop_map *prob_tab = &prop_table[i];
		prob_value = 0;
		if (strncmp(prob->name, prob_tab->name, DRM_PROP_NAME_LEN + 1) == 0) {
			prob_value = prob_tab->map(prob->json_obj, dev);
			DTESTLOGI("Create %s prop, prob value = %u", prob->name, prob_value);
			prob->value = prob_value;
		}
	}
	status = set_property(dev, prob);
	if (DTEST_FAILURE == status) {
		DTESTLOGE("set_property fail");
	}

	return status;
}

dtest_status dtest_blob_prop_disable(device_t *dev, property_t *prob)
{
	uint32_t i, prob_value;
	dtest_status status;
	for (i = 0; i < ARRAY_SIZE(prop_table); i++) {
		const struct prop_map *prob_tab = &prop_table[i];
		prob_value = 0;
		if (strncmp(prob->name, prob_tab->name, DRM_PROP_NAME_LEN + 1) == 0) {
			if (prob_tab->type == PROP_TYPE_BLOB) {
				drmModeDestroyPropertyBlob(dev->fd, (uint32_t)prob->value);

				/* reset this property's blob */
				prob->value = 0;
				DTESTLOGI("Disable %s prop, blob_id value = %u", prob->name,
					  prob_value);
			}
		}
	}
	status = set_property(dev, prob);
	if (DTEST_FAILURE == status) {
		DTESTLOGE("set_property fail");
	}

	return status;
}
