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

#ifndef __PROP_MAP_FUNCS_H__
#define __PROP_MAP_FUNCS_H__

#include "json/cJSON.h"
#include "drmtest.h"

#ifdef MAP_RELOCATION
#define MAPAPI __attribute__((weak))
#else
#define MAPAPI
#endif

#define LUT_PATH "./lut/"

static const uint32_t NR_weak[VS_NR_COEF_NUM] = {
	0x0,   0x40,  0x40,  0x40,  0x10101010, 0x10101010, 0x10101010, 0x10101010, 0x3ff,
	0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff,	0x3ff,	    0x3ff,	0x3ff,	    0x3ff,
	0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff,	0x3ff,	    0x3ff
};
static const uint32_t NR_strong[VS_NR_COEF_NUM] = {
	0x11,  0x10,  0x10,  0x10,  0x090a0c0e, 0x01020305, 0x0c0d0e0f, 0x0506080a, 0x3ff,
	0x3ff, 0x397, 0x2ff, 0x29f, 0x22a,	0x1e3,	    0x1b1,	0x18c,	    0x158,
	0x134, 0x11a, 0xf4,  0xdb,  0xc8,	0xb9,	    0xad
};

static const uint32_t SSR3_strongUps[VS_SSR3_COEF_NUM] = {
	0,  255, 128, 12,  6,	9,     1,   48,	   16,	5,    64,  16,	 20,  12,  255, 96, 128,
	96, 1,	 128, 16,  40,	63,    63,  24,	   1,	128,  255, 72,	 16,  64,  4,	32, 40,
	63, 48,	 1,   32,  40,	16,    1,   1,	   8,	0,    4,   8,	 8,   8,   8,	0,  0,
	12, 10,	 6,   0,   12,	10,    6,   10,	   0,	3,    16,  0,	 1,   15,  8,	4,  2,
	15, 8,	 3,   10,  0,	3,     12,  1,	   20,	0,    3,   3,	 3,   5,   0,	4,  2,
	0,  28,	 64,  80,  128, 4,     8,   32,	   11,	2,    6,   0,	 2,   102, 153, 4,  3,
	25, 50,	 102, 153, 4,	3,     50,  25,	   1,	15,   32,  6,	 11,  16,  0,	0,  128,
	1,  102, 255, 16,  0,	25,    50,  8,	   0,	192,  255, 100,	 0,   0,   6,	8,  0,
	8,  48,	 32,  63,  0,	48,    8,   32,	   8,	26,   1,   0,	 40,  8,   16,	0,  64,
	48, 32,	 63,  4,   4,	4,     15,  20,	   24,	32,   4,   32,	 127, 44,  1,	6,  0,
	1,  8,	 8,   1,   1,	0,     0,   140,   90,	64,   64,  235,	 16,  1,   0,	1,  8,
	16, 1,	 0,   0,   1,	0,     0,   1,	   0,	0,    50,  6144, 255, 0,   255, 0,  0,
	0,  255, 0,   0,   0,	0,     1,   1,	   100, 3840, 255, 0,	 255, 0,   7,	0,  255,
	0,  7,	 0,   0,   1,	1,     150, 10752, 255, 0,    255, 0,	 0,   0,   255, 0,  7,
	3,  0,	 1,   1,   150, 10752, 255, 0,	   255, 0,    0,   0,	 255, 8,   7,	3,  0,
	1,  1,	 3,   4,   8,	0,     8,   8192,  0,	3,    6,   8,	 0
};
static const uint32_t SSR3_TV1[VS_SSR3_COEF_NUM] = {
	0,   255, 255, 12,  6,	 9,  1,	 25,  24,  5,	64,   16, 20,  12,  255, 96,  128, 96,
	1,   64,  32,  40,  63,	 63, 24, 1,   17,  128, 36,   16, 64,  4,   32,	 40,  63,  48,
	1,   0,	  40,  16,  1,	 1,  15, 0,   4,   8,	8,    8,  8,   10,  0,	 11,  9,   0,
	0,   13,  13,  0,   4,	 0,  1,	 1,   8,   1,	0,    8,  0,   0,   0,	 8,   0,   0,
	8,   3,	  12,  3,   20,	 0,  3,	 3,   3,   1,	0,    32, 0,   0,   0,	 24,  140, 148,
	2,   4,	  16,  0,   0,	 15, 0,	 1,   128, 153, 4,    3,  25,  50,  102, 153, 4,   3,
	0,   25,  1,   15,  32,	 6,  80, 64,  0,   0,	128,  1,  102, 255, 16,	 0,   25,  50,
	16,  0,	  128, 160, 47,	 0,  0,	 6,   8,   0,	12,   63, 55,  31,  0,	 46,  8,   32,
	12,  26,  13,  13,  75,	 15, 47, 47,  0,   40,	40,   63, 7,   7,   3,	 0,   8,   16,
	32,  2,	  4,   8,   0,	 0,  0,	 0,   1,   1,	3,    1,  1,   0,   0,	 140, 90,  64,
	64,  235, 16,  1,   0,	 1,  6,	 16,  0,   0,	0,    1,  0,   0,   1,	 0,   0,   0,
	24,  255, 0,   255, 0,	 0,  0,	 255, 0,   0,	0,    0,  1,   1,   100, 15,  128, 0,
	255, 0,	  7,   0,   255, 0,  7,	 0,   0,   1,	1,    0,  0,   95,  0,	 255, 0,   7,
	7,   255, 0,   7,   3,	 0,  1,	 1,   100, 15,	255,  0,  255, 0,   7,	 0,   255, 8,
	7,   3,	  0,   1,   1,	 5,  5,	 5,   0,   8,	8192, 0,  3,   8,   7,	 0
};
static const uint32_t SSR3_TV2[VS_SSR3_COEF_NUM] = {
	0,   255, 255, 12,  6,	 9,  1,	 0,   24,  5,	64,   16, 20,  12,  255, 96,  128, 96,
	1,   64,  32,  40,  63,	 63, 24, 1,   53,  128, 36,   16, 64,  4,   32,	 40,  63,  48,
	1,   0,	  40,  16,  1,	 1,  15, 0,   4,   8,	8,    8,  8,   10,  0,	 16,  13,  2,
	0,   16,  13,  0,   8,	 0,  1,	 1,   8,   1,	4,    8,  0,   0,   0,	 8,   0,   0,
	8,   3,	  12,  3,   20,	 0,  3,	 3,   3,   1,	0,    44, 0,   0,   0,	 24,  140, 148,
	2,   4,	  16,  0,   0,	 15, 0,	 1,   128, 153, 4,    3,  25,  50,  102, 153, 4,   3,
	0,   25,  1,   15,  32,	 6,  80, 64,  0,   0,	128,  1,  102, 255, 16,	 0,   25,  50,
	16,  0,	  128, 160, 47,	 0,  0,	 6,   8,   0,	12,   63, 55,  31,  0,	 46,  8,   32,
	12,  26,  10,  10,  75,	 15, 47, 47,  0,   40,	40,   63, 7,   7,   3,	 0,   8,   16,
	32,  2,	  4,   8,   0,	 0,  0,	 0,   1,   6,	8,    1,  1,   0,   0,	 140, 90,  64,
	64,  235, 16,  1,   0,	 1,  9,	 16,  0,   0,	0,    1,  0,   0,   1,	 0,   0,   0,
	24,  255, 0,   255, 0,	 0,  0,	 255, 0,   0,	0,    0,  1,   1,   100, 15,  128, 0,
	255, 0,	  7,   0,   255, 0,  7,	 0,   0,   1,	1,    0,  0,   95,  0,	 255, 0,   7,
	7,   255, 0,   7,   3,	 0,  1,	 1,   100, 15,	255,  0,  255, 0,   7,	 0,   255, 8,
	7,   3,	  0,   1,   1,	 5,  5,	 5,   1,   8,	8192, 0,  3,   8,   7,	 0
};
static const uint32_t SSR3_TV3[VS_SSR3_COEF_NUM] = {
	0,   255, 255, 12,  6,	 9,  1,	 0,   16,  5,	64,  16,   20,	12,  255, 24,  32,  96,
	1,   48,  16,  63,  63,	 63, 24, 1,   89,  0,	72,  31,   64,	4,   32,  16,  32,  48,
	1,   0,	  40,  16,  1,	 1,  15, 0,   4,   8,	8,   8,	   8,	0,   0,	  20,  17,  6,
	1,   20,  15,  6,   16,	 0,  2,	 3,   0,   1,	6,   0,	   0,	0,   0,	  8,   0,   10,
	0,   3,	  10,  3,   20,	 0,  3,	 3,   3,   1,	0,   56,   0,	0,   0,	  16,  24,  32,
	2,   4,	  16,  0,   1,	 15, 1,	 1,   102, 153, 4,   3,	   25,	50,  102, 153, 4,   3,
	50,  25,  1,   15,  32,	 6,  80, 64,  0,   0,	128, 0,	   102, 255, 16,  0,   25,  50,
	8,   0,	  128, 160, 144, 3,  3,	 6,   8,   1,	12,  63,   55,	31,  0,	  46,  12,  32,
	24,  8,	  12,  12,  60,	 15, 47, 47,  0,   40,	40,  63,   7,	7,   3,	  0,   8,   16,
	32,  2,	  4,   8,   0,	 0,  0,	 0,   1,   4,	6,   1,	   1,	0,   0,	  140, 90,  64,
	64,  235, 16,  1,   1,	 1,  12, 16,  1,   0,	0,   1,	   0,	0,   1,	  0,   0,   0,
	24,  255, 0,   255, 0,	 0,  0,	 255, 0,   0,	0,   0,	   1,	1,   100, 15,  128, 0,
	255, 0,	  7,   0,   255, 0,  7,	 0,   0,   1,	1,   0,	   0,	95,  0,	  255, 0,   7,
	7,   255, 0,   7,   7,	 0,  1,	 0,   100, 15,	255, 15,   255, 0,   7,	  0,   255, 8,
	7,   3,	  0,   1,   1,	 8,  8,	 8,   0,   8,	0,   2560, 5,	8,   7,	  0
};
static const uint32_t SSR3_DESK[VS_SSR3_COEF_NUM] = {
	0,   64,  64,  28,  6,	 9,  1,	 12,  8,   5,	64,   16, 16,  12,  0,	 0,   32,  96,
	1,   32,  32,  32,  63,	 63, 63, 0,   48,  255, 64,   31, 64,  2,   16,	 32,  32,  24,
	1,   32,  8,   8,   1,	 1,  0,	 0,   1,   15,	15,   0,  0,   10,  0,	 12,  8,   2,
	1,   12,  8,   2,   8,	 2,  3,	 1,   8,   1,	12,   8,  2,   1,   12,	 8,   2,   8,
	2,   3,	  12,  3,   20,	 0,  3,	 3,   3,   4,	0,    4,  1,   0,   32,	 64,  128, 255,
	4,   8,	  32,  64,  2,	 6,  0,	 1,   102, 153, 4,    3,  18,  18,  102, 153, 4,   3,
	18,  18,  1,   15,  32,	 6,  0,	 0,   0,   0,	128,  1,  102, 255, 16,	 0,   25,  50,
	8,   0,	  128, 128, 48,	 3,  0,	 6,   8,   0,	12,   48, 32,  63,  0,	 48,  10,  32,
	8,   12,  1,   1,   32,	 8,  32, 16,  96,  48,	32,   63, 4,   4,   4,	 12,  8,   16,
	32,  4,	  32,  16,  8,	 1,  6,	 0,   1,   3,	3,    1,  1,   0,   0,	 140, 90,  64,
	64,  235, 16,  1,   0,	 1,  2,	 16,  1,   0,	0,    1,  0,   0,   1,	 0,   0,   0,
	24,  255, 0,   255, 0,	 0,  0,	 255, 0,   0,	0,    0,  1,   1,   100, 15,  128, 0,
	255, 0,	  7,   0,   255, 0,  7,	 0,   0,   1,	1,    0,  0,   95,  0,	 255, 0,   7,
	7,   255, 0,   7,   3,	 0,  1,	 1,   100, 15,	255,  0,  255, 0,   7,	 0,   255, 8,
	7,   3,	  0,   1,   1,	 5,  5,	 5,   0,   8,	8192, 0,  3,   8,   7,	 0
};
static const uint32_t SSR3_GAME[VS_SSR3_COEF_NUM] = {
	0,   255, 255, 22,  6,	 9,  1,	 24,  16,  5,	64,   16, 20,  12,  255, 24,  32,  96,
	1,   16,  40,  40,  63,	 63, 24, 1,   78,  128, 72,   16, 64,  4,   32,	 40,  63,  48,
	1,   0,	  40,  16,  1,	 1,  15, 0,   4,   8,	8,    8,  8,   10,  0,	 11,  9,   0,
	0,   13,  13,  0,   4,	 0,  1,	 1,   8,   1,	0,    8,  0,   0,   0,	 8,   0,   0,
	8,   3,	  12,  3,   20,	 0,  3,	 3,   3,   1,	0,    4,  2,   0,   128, 28,  80,  128,
	4,   8,	  49,  11,  2,	 6,  0,	 1,   102, 153, 4,    3,  25,  50,  102, 153, 4,   3,
	50,  25,  1,   15,  32,	 6,  80, 64,  0,   0,	128,  1,  102, 255, 16,	 0,   25,  50,
	8,   0,	  192, 255, 70,	 0,  0,	 6,   8,   0,	8,    48, 32,  63,  0,	 48,  8,   32,
	8,   28,  1,   0,   40,	 8,  16, 0,   96,  48,	32,   63, 4,   4,   4,	 48,  15,  20,
	32,  4,	  32,  127, 44,	 1,  6,	 0,   1,   2,	3,    0,  1,   0,   1,	 140, 90,  64,
	64,  235, 16,  1,   1,	 1,  0,	 16,  1,   0,	0,    1,  0,   0,   1,	 0,   0,   0,
	24,  255, 0,   255, 0,	 0,  0,	 255, 0,   0,	0,    0,  1,   1,   100, 15,  128, 0,
	255, 0,	  7,   0,   255, 0,  7,	 0,   0,   1,	1,    0,  0,   95,  0,	 255, 0,   7,
	7,   255, 0,   7,   3,	 0,  1,	 1,   100, 15,	255,  0,  255, 0,   7,	 0,   255, 8,
	7,   3,	  0,   1,   1,	 5,  5,	 5,   0,   8,	8192, 0,  3,   8,   7,	 0
};
static const uint32_t SSR3_NATURE[VS_SSR3_COEF_NUM] = {
	0,   255, 255, 12,  6,	 9,  1,	 12,  16,  5,	64,  16,   20,	12,  255, 24,  32,  96,
	1,   48,  16,  63,  63,	 63, 24, 1,   56,  0,	72,  31,   64,	4,   32,  16,  32,  48,
	1,   0,	  40,  16,  1,	 1,  15, 0,   4,   8,	8,   8,	   8,	0,   0,	  20,  17,  6,
	1,   20,  15,  6,   16,	 0,  2,	 3,   0,   1,	6,   0,	   0,	0,   0,	  8,   0,   10,
	0,   3,	  10,  3,   20,	 0,  3,	 3,   3,   1,	0,   4,	   0,	0,   16,  24,  32,  64,
	2,   4,	  16,  0,   1,	 15, 1,	 1,   102, 153, 4,   3,	   25,	50,  102, 153, 4,   3,
	50,  25,  1,   15,  32,	 6,  80, 64,  0,   0,	128, 1,	   102, 255, 16,  0,   25,  50,
	8,   0,	  128, 80,  144, 3,  3,	 6,   8,   1,	12,  63,   55,	31,  0,	  48,  8,   32,
	8,   28,  1,   0,   40,	 8,  16, 0,   96,  40,	40,  63,   7,	7,   3,	  8,   16,  32,
	64,  2,	  4,   8,   0,	 0,  0,	 0,   1,   5,	6,   1,	   1,	0,   0,	  140, 90,  64,
	64,  235, 16,  1,   1,	 1,  12, 16,  1,   0,	0,   1,	   0,	0,   1,	  0,   0,   0,
	24,  255, 0,   255, 0,	 0,  0,	 255, 0,   0,	0,   0,	   1,	1,   100, 15,  128, 0,
	255, 0,	  7,   0,   255, 0,  7,	 0,   0,   1,	1,   0,	   0,	95,  0,	  255, 0,   7,
	7,   255, 0,   7,   7,	 0,  1,	 0,   100, 15,	255, 15,   255, 0,   7,	  0,   255, 8,
	7,   3,	  0,   1,   1,	 8,  8,	 8,   0,   8,	0,   2560, 5,	8,   7,	  0
};

static const uint32_t SSR3_DEFAULT[VS_SSR3_COEF_NUM] = {
	0,   255, 255, 12,  6,	 9,  1,	 12,  16,  5,	64,  16,   20,	12,  255, 24,  32,  96,
	1,   48,  16,  63,  63,	 63, 24, 1,   40,  0,	72,  31,   64,	4,   32,  16,  32,  32,
	1,   0,	  40,  16,  0,	 0,  15, 0,   4,   8,	8,   8,	   8,	0,   0,	  10,  8,   0,
	0,   12,  10,  0,   4,	 0,  2,	 3,   0,   1,	6,   0,	   0,	0,   0,	  8,   0,   10,
	0,   3,	  10,  3,   20,	 0,  3,	 3,   3,   1,	0,   4,	   0,	0,   16,  24,  32,  64,
	2,   4,	  16,  0,   1,	 15, 1,	 1,   102, 153, 4,   3,	   25,	50,  102, 153, 4,   3,
	50,  25,  1,   15,  32,	 6,  80, 64,  0,   0,	128, 1,	   102, 255, 16,  0,   25,  50,
	8,   0,	  106, 80,  144, 3,  3,	 6,   8,   1,	12,  63,   55,	31,  0,	  48,  8,   32,
	8,   28,  1,   0,   40,	 8,  16, 0,   96,  40,	40,  63,   7,	7,   3,	  8,   16,  32,
	64,  2,	  4,   8,   0,	 0,  0,	 0,   1,   5,	5,   1,	   1,	0,   0,	  140, 90,  64,
	64,  235, 16,  1,   1,	 1,  3,	 16,  1,   0,	0,   1,	   0,	0,   1,	  0,   0,   0,
	24,  255, 0,   255, 0,	 0,  0,	 255, 0,   0,	0,   0,	   1,	1,   100, 15,  128, 0,
	255, 0,	  7,   0,   255, 0,  7,	 0,   0,   1,	1,   0,	   0,	95,  0,	  255, 0,   7,
	7,   255, 0,   7,   7,	 0,  1,	 0,   100, 15,	255, 15,   255, 0,   7,	  0,   255, 8,
	7,   3,	  0,   1,   1,	 8,  8,	 8,   0,   8,	0,   2560, 5,	8,   7,	  0
};

enum map_ccm_type {
	FE_LNR_CCM,
	BLD_NONLNR_CCM,
	BLD_LNR_CCM,
	BE_NONLNR_CCM,
	BE_LNR_CCM,
	BE_LNR_CCM_EX,
	LAYER_CCM,
};

uint64_t map_ext_layer_fb(cJSON *obj, struct device *dev);
uint64_t map_dma_config(cJSON *obj, struct device *dev);
uint64_t map_line_padding(cJSON *obj, struct device *dev);
uint64_t map_up_sample(cJSON *obj, struct device *dev);
uint64_t map_y2r_csc(cJSON *obj, struct device *dev);
uint64_t map_drm_ltm_gamma(cJSON *obj, struct device *dev);
uint64_t map_drm_ltm_degamma(cJSON *obj, struct device *dev);
uint64_t map_drm_ltm_luma(cJSON *obj, struct device *dev);
uint64_t map_drm_ltm_freq_decomp(cJSON *obj, struct device *dev);
uint64_t map_drm_ltm_luma_adj(cJSON *obj, struct device *dev);
uint64_t map_drm_ltm_grid(cJSON *obj, struct device *dev);
uint64_t map_drm_ltm_af_filter(cJSON *obj, struct device *dev);
uint64_t map_drm_ltm_af_slice(cJSON *obj, struct device *dev);
uint64_t map_drm_ltm_af_trans(cJSON *obj, struct device *dev);
uint64_t map_drm_ltm_tone_adj(cJSON *obj, struct device *dev);
uint64_t map_drm_ltm_color(cJSON *obj, struct device *dev);
uint64_t map_drm_ltm_dither(cJSON *obj, struct device *dev);
uint64_t map_drm_ltm_luma_ave_set(cJSON *obj, struct device *dev);
uint64_t map_drm_ltm_hist_cd_set(cJSON *obj, struct device *dev);
uint64_t map_drm_ltm_local_hist_set(cJSON *obj, struct device *dev);
uint64_t map_drm_ltm_down_scaler(cJSON *obj, struct device *dev);
uint64_t map_drm_ltm(cJSON *obj, struct device *dev);

uint64_t update_ltm_slope_bias(cJSON *obj, struct device *dev, uint32_t frame_id);

uint64_t map_drm_gtm(cJSON *obj, struct device *dev);
uint64_t map_drm_sharpness(cJSON *obj, struct device *dev);
uint64_t map_drm_sharpness_csc(cJSON *obj, struct device *dev);
uint64_t map_drm_sharpness_luma_gain(cJSON *obj, struct device *dev);
uint64_t map_drm_sharpness_lpf(cJSON *obj, struct device *dev);
uint64_t map_drm_sharpness_lpf_noise(cJSON *obj, struct device *dev);
uint64_t map_drm_sharpness_lpf_curve(cJSON *obj, struct device *dev);
uint64_t map_drm_sharpness_color_adaptive(cJSON *obj, struct device *dev);
uint64_t map_drm_sharpness_color_boost(cJSON *obj, struct device *dev);
uint64_t map_drm_sharpness_soft_clip(cJSON *obj, struct device *dev);
uint64_t map_drm_sharpness_dither(cJSON *obj, struct device *dev);
uint64_t map_drm_blur(cJSON *obj, struct device *dev);
uint64_t map_drm_rcd(cJSON *obj, struct device *dev);
uint64_t map_pvric_clear(cJSON *obj, struct device *dev);
uint64_t map_pvric_const(cJSON *obj, struct device *dev);
uint64_t map_dither(cJSON *obj, struct device *dev);
uint64_t map_panel_dither(cJSON *obj, struct device *dev);
uint64_t map_llv_dither(cJSON *obj, struct device *dev);
uint64_t map_blender_dither(cJSON *obj, struct device *dev);
uint64_t map_wb_point(cJSON *obj, struct device *dev);
uint64_t map_wb_dither(cJSON *obj, struct device *dev);
uint64_t map_wb_stripe_position(cJSON *obj, struct device *dev);
uint64_t map_wb_rotation(cJSON *obj, struct device *dev);
uint64_t map_brightness(cJSON *obj, struct device *dev);
uint64_t map_brightness_roi(cJSON *obj, struct device *dev);
uint64_t map_color_calibration_path_mode(cJSON *obj, struct device *dev);
uint64_t map_ds_config(cJSON *obj, struct device *dev);
uint64_t map_scale_config(cJSON *obj, struct device *dev);
uint64_t map_r2y_config(cJSON *obj, struct device *dev);
uint64_t map_fe_ccm(cJSON *obj, struct device *dev);
uint64_t map_bld_nonlnr_ccm(cJSON *obj, struct device *dev);
uint64_t map_bld_lnr_ccm(cJSON *obj, struct device *dev);
uint64_t map_be_nonlnr_ccm(cJSON *obj, struct device *dev);
uint64_t map_be_lnr_ccm(cJSON *obj, struct device *dev);
uint64_t map_be_lnr_ccm_ex(cJSON *obj, struct device *dev);
uint64_t map_layer_ccm(cJSON *obj, struct device *dev);
uint64_t map_prior_3dlut(cJSON *obj, struct device *dev);
uint64_t map_roi_3dlut(cJSON *obj, struct device *dev);
uint64_t map_xstep_lut(cJSON *obj, struct device *dev);
uint64_t map_tone_map(cJSON *obj, struct device *dev);
uint64_t map_drm_hdr(cJSON *obj, struct device *dev);
uint64_t map_gamma_lut(cJSON *obj, struct device *dev);
uint64_t map_regamma_lut(cJSON *obj, struct device *dev);
uint64_t map_degamma_config(cJSON *obj, struct device *dev);
uint64_t map_drm_degamma(cJSON *obj, struct device *dev);
uint64_t map_color(cJSON *obj, struct device *dev);
uint64_t map_colorkey(cJSON *obj, struct device *dev);
uint64_t map_side_by_side(cJSON *obj, struct device *dev);
uint64_t map_blend_mode(cJSON *obj, struct device *dev);
uint64_t map_hist(cJSON *obj, struct device *dev);
uint64_t map_histogram0(cJSON *obj, struct device *dev);
uint64_t map_histogram1(cJSON *obj, struct device *dev);
uint64_t map_histogram2(cJSON *obj, struct device *dev);
uint64_t map_histogram3(cJSON *obj, struct device *dev);
uint64_t map_histogram_rgb(cJSON *obj, struct device *dev);
uint64_t map_hist_roi(cJSON *obj, struct device *dev);
uint64_t map_hist_block(cJSON *obj, struct device *dev);
uint64_t map_hist_prot(cJSON *obj, struct device *dev);
uint64_t map_hist_info(cJSON *obj, struct device *dev);
uint64_t map_ops(cJSON *obj, struct device *dev);
uint64_t map_wb_spliter(cJSON *obj, struct device *dev);
uint64_t map_dsc(cJSON *obj, struct device *dev);
uint64_t map_vdc(cJSON *obj, struct device *dev);
uint64_t map_spliter(cJSON *obj, struct device *dev);
uint64_t map_ramless_fb(cJSON *obj, struct device *dev);
uint64_t map_bool(cJSON *obj, struct device *dev);
uint64_t map_vrr_refresh(cJSON *obj, struct device *dev);
uint64_t map_free_sync(cJSON *obj, struct device *dev);
uint64_t map_dp_sync(cJSON *obj, struct device *dev);
uint64_t map_lbox(cJSON *obj, struct device *dev);
uint64_t map_splice_mode(cJSON *obj, struct device *dev);
uint64_t map_panel_crop(cJSON *obj, struct device *dev);
uint64_t map_data_extend(cJSON *obj, struct device *dev);
uint64_t map_data_trunc(cJSON *obj, struct device *dev);
uint64_t map_ctm(cJSON *obj, struct device *dev);
uint64_t map_sr2000(cJSON *obj, struct device *dev);
uint64_t map_multi_extend_layer(cJSON *obj, struct device *dev);
uint64_t map_dec_fc(cJSON *obj, struct device *dev);
uint64_t map_ctx_id(cJSON *obj, struct device *dev);
uint64_t map_dbi(cJSON *obj, struct device *dev);
uint64_t map_dbi_correction_coef(cJSON *obj, struct device *dev);
#endif
