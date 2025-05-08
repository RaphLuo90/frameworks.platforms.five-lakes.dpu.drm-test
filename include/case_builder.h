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

#ifndef __CASE_BUILDER_H__
#define __CASE_BUILDER_H__

#include "drmtest.h"

typedef struct _dtest_property {
	property_t *properties;
	uint32_t num;
} dtest_property_t;

typedef struct _dtest_pipe {
	pipe_t *pipes;
	uint32_t num;
	dtest_property_t __property;
} dtest_pipe_t;

typedef struct _dtest_plane {
	plane_t *planes;
	uint32_t num;
	dtest_property_t __property;
} dtest_plane_t;

typedef struct _dtest_cursor {
	cursor_t *cursors;
	uint32_t num;
	dtest_property_t __property; /* not use for now */
} dtest_cursor_t;

typedef struct _dtest_crtc {
	void *crtc; // it is a placeholder.
	uint32_t num;
	dtest_property_t __property;
} dtest_crtc_t;

typedef struct _frame_arg {
	uint32_t frame_id;
	dtest_pipe_t pipe;
	dtest_plane_t plane;
	dtest_cursor_t cursor;
	dtest_crtc_t crtc;
	dtest_property_t property;
	bool commit_done;
	uint32_t random;
} frame_t;

typedef struct _dtest_frame {
	frame_t *frames;
	uint32_t num;
	bool run_stream;
} dtest_frame_t;

typedef struct _stream_arg {
	uint32_t stream_id;
	dtest_frame_t *dtest_frame;
} stream_t;

typedef struct _dtest_stream {
	stream_t *streams;
	uint32_t num;
} dtest_stream_t;

dtest_status dtest_case_props_map(device_t *dev, dtest_property_t *props);

cJSON *dtest_case_setup(const char *path, dtest_frame_t *frames);

void dtest_case_delete(dtest_frame_t *dtest_frames);

dtest_status build_frame(cJSON *obj_frame, frame_t *frame);

#endif
