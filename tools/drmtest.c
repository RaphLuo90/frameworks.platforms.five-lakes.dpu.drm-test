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
#include <unistd.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#include <pthread.h>

#include "viv_dc_functions.h"
#include "dpu_algorithm.h"
#include "map_funcs/prop_map_funcs.h"
#include <time.h>

#define TIME_OUT 300

#define GOTO_LABEL(val, lbl) \
	do {                 \
		ret = val;   \
		goto lbl;    \
	} while (0)

#define CHECK_STATUS(val, stat, lbl)                                              \
	do {                                                                      \
		if (DTEST_SUCCESS != stat) {                                      \
			ret = val;                                                \
			if (DTEST_NOT_SUPPORT == stat)                            \
				DTESTLOGE("CHECK STATUS(%d): Unsupport", (stat)); \
			else                                                      \
				DTESTLOGE("CHECK STATUS(%d): Failure", (stat));   \
			goto lbl;                                                 \
		}                                                                 \
	} while (0)

#define CHECK_PTR(val, ptr, lbl)                     \
	do {                                         \
		if (NULL == ptr) {                   \
			ret = val;                   \
			DTESTLOGE("CHECK PTR FAIL"); \
			goto lbl;                    \
		}                                    \
	} while (0)

static enum util_fill_pattern primary_fill = UTIL_PATTERN_SMPTE;
static enum util_fill_pattern secondary_fill = UTIL_PATTERN_TILES;

struct arguments {
	const char *case_path;
	bool reset;
	bool clear;
	bool raw;
	bool check;
	bool clean;
	bool vdp_dirty;
};

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

static dtest_status __check_dump_folder(const char *dump_folder)
{
	if (strcmp(dump_folder, ".") == 0) {
		return DTEST_SUCCESS;
	}

	if (access(dump_folder, 0) == 0) {
		return DTEST_SUCCESS;
	}

	if (mkdir(dump_folder, 0775) == 0) {
		DTESTLOGI("Create the dump_folder %s", dump_folder);
		return DTEST_SUCCESS;
	}

	return DTEST_FAILURE;
}

static dtest_status __parse_case_name(const char *case_path, char *case_name)
{
	char *suffix = ".json";
	char buffer[512] = { 0 };
	char *ptr = strrchr(case_path, '/');
	if (!ptr) {
		DTESTLOGI("parse case name fail, %s", case_path);
		return DTEST_FAILURE;
	}
	if (strlen(ptr + 1) > 511) {
		DTESTLOGI("case name overflow, %s", case_path);
		return DTEST_FAILURE;
	}
	memcpy(buffer, ptr + 1, strlen(ptr + 1));
	buffer[strlen(ptr + 1)] = '\0';

	ptr = strstr(buffer, suffix);
	if (!ptr || strlen(ptr) != strlen(suffix) || strncmp(ptr, suffix, strlen(suffix))) {
		DTESTLOGI("Invalid case_path %s", case_path);
		return DTEST_FAILURE;
	}

	memcpy(case_name, buffer, strlen(buffer) - strlen(suffix));
	return DTEST_SUCCESS;
}

static void usage(char *name)
{
	fprintf(stderr, "usage: %s\n", name);

	fprintf(stderr, "\n Test options:\n\n");
	fprintf(stderr, "\t-case <case path> \t Load a case file.\n");
	fprintf(stderr, "\t-o <dump directory> \t The directory of dump files.\n");
	fprintf(stderr, "\t-F pattern1,pattern2\tspecify fill patterns\n");

	fprintf(stderr, "\n Generic options:\n\n");
	fprintf(stderr, "\t-D device\tuse the given device\n");
	fprintf(stderr, "\t-reset \treset the drm\n");

	fprintf(stderr, "\n\tDefault is to dump all info.\n");
	exit(0);
}

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t *stream_lock = NULL;
pthread_cond_t *stream_cond = NULL;
#if CONFIG_VERISILICON_MD5_CHECK
char ***stream_golden = NULL;
char ***stream_result = NULL;
#endif

struct dump_params {
	dtest_frame_t dtest_frames;
	dtest_status status;
	struct device *dev;
	char *dump_folder;
	char *case_name;
	struct arguments args;
};

struct dump_stream_params {
	struct device *dev;
	stream_t *stream;
	uint32_t run_count;
	char *dump_folder;
	char *case_name;
	struct arguments args;
};

static void *dump_wb_buffer_thread(void *arg)
{
	int ret, i = 0, j = 0;
	FILE *stream_fp = NULL;
	FILE *stream_fp_arr[1] = { NULL };
	uint32_t plane_num = 0;
	uint32_t pos_frame = 0;
	drm_vs_bo_param *bo_param = NULL;
	char stream_filename[512] = { 0 };

	struct dump_params *params = (struct dump_params *)arg;
	dtest_frame_t dtest_frames = params->dtest_frames;
	dtest_status status = params->status;
	struct device *dev = params->dev;
	char *dump_folder = params->dump_folder;
	char *case_name = params->case_name;
	struct arguments args = params->args;
	uint32_t query_num = 0;

	if (dtest_frames.run_stream) {
		snprintf(stream_filename, sizeof(stream_filename), "%s/%s_%s.raw", dump_folder,
			 case_name, dtest_frames.frames[0].pipe.pipes[0].cons[0]);

		stream_fp = fopen(stream_filename, "wb");
		if (NULL == stream_fp) {
			DTESTLOGE("Open %s fail", stream_filename);
			goto exit;
		}
		stream_fp_arr[0] = stream_fp;
	}

	for (i = 0; i < dtest_frames.num; i++) {
		frame_t *frame = &dtest_frames.frames[i];
		dtest_pipe_t *pipes = &frame->pipe;
		pthread_mutex_lock(&lock);
		time_t start_wait_time = time(NULL);
		bool continue_flag = false;
		while (!frame->commit_done) {
			pthread_cond_wait(&cond, &lock);
			time_t end_wait_time = time(NULL);
			double time_diff = difftime(end_wait_time, start_wait_time);
			if (time_diff > TIME_OUT) {
				continue_flag = true;
				break;
			}
		}
		if (continue_flag) {
			pthread_mutex_unlock(&lock);
			continue;
		}
		pthread_mutex_unlock(&lock);

		dpu_get_time_of_day(pipes);

		/*
		* vcmd need query the frame done.
		*/
		if (dev->use_vcmd) {
			/*query the exe frame count*/
			struct drm_vs_ctx ctx_arg = { .type = VS_CTX_QUERY };
			uint32_t frm_exe_count = 0;
			ctx_arg.handle = 1; /*the default ctx id, multi-stream need refine*/
			query_num = 0;

			drmIoctl(dev->fd, DRM_IOCTL_VS_SET_CTX, &ctx_arg);
			frm_exe_count = ctx_arg.frm_exe_count;

			while (frm_exe_count < i + 1) {
				sleep(2);
				drmIoctl(dev->fd, DRM_IOCTL_VS_SET_CTX, &ctx_arg);
				frm_exe_count = ctx_arg.frm_exe_count;
				query_num++;
				if (query_num > 10) {
					DTESTLOGI("[%s] query wait frame done time out!", __func__);
					break;
				}
			}
		}
		if (dtest_frames.run_stream)
			pos_frame = ftell((FILE *)stream_fp);
		status = dpu_dump_output_fbs(dev, pipes, dump_folder, case_name, stream_fp_arr,
					     &pos_frame, args.raw, i, args.check, NULL, NULL);
		CHECK_STATUS(-1, status, final);

		if (dtest_frames.run_stream) {
			plane_num = pipes->pipes->num_planes;
			bo_param = pipes->pipes->bo_param;

			for (j = 0; j < plane_num; j++) {
				pos_frame += bo_param[j].width * bo_param[j].bpp / 8 *
					     bo_param[j].height;
			}

			fseek(stream_fp, pos_frame, SEEK_SET);
		}

		if (i > 0)
			dpu_clear_frame_FB(dev, &dtest_frames.frames[i - 1]);
	}
exit:
	if (stream_fp)
		fclose((FILE *)stream_fp);

final:
	pthread_exit(&ret);
}

static void *pressure_stream_dump_wb_buffer_thread(void *arg)
{
	int ret = 0, i = 0, j = 0, k = 0;
	struct dump_stream_params *params = (struct dump_stream_params *)arg;
	char *dump_folder = params->dump_folder;
	char *case_name = params->case_name;
	stream_t *stream = params->stream;
	uint32_t run_count = params->run_count;
	uint32_t pip_num = stream->dtest_frame->frames[0].pipe.num;
	char **stream_filename = (char **)calloc(pip_num, sizeof(char *));
	FILE **stream_fp = (FILE **)calloc(pip_num, sizeof(FILE *));
	uint32_t *pos_frame = (uint32_t *)calloc(pip_num, sizeof(uint32_t));
	dtest_status status;
	struct device *dev = params->dev;
	struct arguments args = params->args;
	uint32_t query_num = 0;

	DTESTLOGI("-------- run count:%u stream:%u : begin stream_wb_dump thread ----------\n",
		  run_count, stream->stream_id);
	for (i = 0; i < pip_num; i++) {
		stream_filename[i] = (char *)calloc(512, sizeof(char));
#if CONFIG_VERISILICON_STREAM_DEBUG
		snprintf(stream_filename[i], 512, "%s/rt%u_%s_stream%u_%s.raw", dump_folder,
			 run_count, case_name, stream->stream_id,
			 stream->dtest_frame->frames[0].pipe.pipes[i].cons[0]);
#else
		snprintf(stream_filename[i], 512, "%s/%s_stream_%u_%s.raw", dump_folder, case_name,
			 stream->stream_id, stream->dtest_frame->frames[0].pipe.pipes[i].cons[0]);
#endif
		stream_fp[i] = fopen(stream_filename[i], "wb");
		if (NULL == stream_fp[i]) {
			DTESTLOGE("Open %s fail", stream_filename[i]);
			goto exit;
		}
	}

	for (i = 0; i < stream->dtest_frame->num; i++) {
		frame_t *frame = &stream->dtest_frame->frames[i];
		dtest_pipe_t *pipes = &frame->pipe;
		pthread_mutex_lock(&stream_lock[stream->stream_id]);
		time_t start_wait_time = time(NULL);
		bool continue_flag = false;

		while (!frame->commit_done) {
			DTESTLOGI(
				"--------run count:%u stream:%u : stream_wb_dump thread wait frame %u commit done ----------\n",
				run_count, stream->stream_id, i);
			pthread_cond_wait(&stream_cond[stream->stream_id],
					  &stream_lock[stream->stream_id]);
			time_t end_wait_time = time(NULL);
			double time_diff = difftime(end_wait_time, start_wait_time);
			if (time_diff > TIME_OUT) {
				continue_flag = true;
				break;
			}
		}
		if (continue_flag) {
			pthread_mutex_unlock(&stream_lock[stream->stream_id]);
			DTESTLOGE(
				"--------run count:%u stream:%u : stream_wb_dump thread wait frame %u commit done timeout--------\n",
				run_count, stream->stream_id, i);
			continue;
		}
		pthread_mutex_unlock(&stream_lock[stream->stream_id]);
		DTESTLOGI(
			"--------run count:%u stream:%u : stream_wb_dump thread get frame %u commit done-----------\n",
			run_count, stream->stream_id, i);

		dpu_get_time_of_day(pipes);
		if (dev->use_vcmd) {
			/*query the exe frame count*/
			struct drm_vs_ctx ctx_arg = { .type = VS_CTX_QUERY };
			uint32_t frm_exe_count = 0;
			ctx_arg.handle = 1; /*the default ctx id, multi-stream need refine*/
			query_num = 0;

			drmIoctl(dev->fd, DRM_IOCTL_VS_SET_CTX, &ctx_arg);
			frm_exe_count = ctx_arg.frm_exe_count;

			while (frm_exe_count < i + 1) {
				sleep(2);
				drmIoctl(dev->fd, DRM_IOCTL_VS_SET_CTX, &ctx_arg);
				frm_exe_count = ctx_arg.frm_exe_count;
				query_num++;
				if (query_num > 10) {
					DTESTLOGI("[%s] query wait frame done time out!", __func__);
					break;
				}
			}
		}
		for (j = 0; j < pip_num; j++)
			pos_frame[j] = ftell((FILE *)stream_fp[j]);
		status = dpu_dump_output_fbs(dev, pipes, dump_folder, case_name, stream_fp,
					     pos_frame, args.raw, i, args.check, &run_count,
					     &stream->stream_id);
		CHECK_STATUS(-1, status, final);
		DTESTLOGI(
			"--------run count:%u stream:%u : stream_wb_dump thread dump frame %u done--------------\n",
			run_count, stream->stream_id, i);

		if (i > 0)
			dpu_clear_frame_FB(dev, &stream->dtest_frame->frames[i - 1]);
		frame->commit_done = false;
	}
	dpu_clear_frame_FB(dev, &stream->dtest_frame->frames[i - 1]);
#if CONFIG_VERISILICON_MD5_CHECK
	for (k = 0; k < pip_num; k++) {
		calcu_md5(stream_filename[k], stream_result[stream->stream_id][k]);
		DTESTLOGI("---------run_count_%d, stream%d_, wb%d, md5_result:%s---------\n",
			  run_count, stream->stream_id, k, stream_result[stream->stream_id][k]);
	}
#endif
exit:
	for (k = 0; k < pip_num; k++) {
		if (stream_fp && stream_fp[k]) {
			fclose(stream_fp[k]);
		}
		if (stream_filename && stream_filename[k]) {
			free(stream_filename[k]);
		}
	}
	if (stream_fp)
		free(stream_fp);
	if (stream_filename)
		free(stream_filename);
final:
	DTESTLOGI("--------run count:%u stream:%u: close stream_wb_dump thread---------\n",
		  run_count, stream->stream_id);

	if (pos_frame)
		free(pos_frame);

	pthread_exit(&ret);
}

struct update_ltm_params {
	struct device *dev;
	bool is_stream;
	dtest_frame_t dtest_frames;
	dtest_stream_t dtest_streams;
};

static void *update_ltm_thread(void *arg)
{
	uint32_t i;
	struct update_ltm_params *params = (struct update_ltm_params *)arg;
	cJSON *ltm_json_obj = NULL;
	struct device *dev = params->dev;
	dtest_frame_t dtest_frames = params->dtest_frames;
	dtest_stream_t dtest_streams = params->dtest_streams;
	stream_t *stream = dtest_streams.streams;
	bool is_stream = params->is_stream;
	dtest_property_t *props = NULL;
	int frame_nums = 0;
	frame_t *frame = NULL;
	if (is_stream)
		props = &stream->dtest_frame->frames[0].property;
	else
		props = &dtest_frames.frames[0].property;
	property_t *prop = props->properties;
	bool found = false;

	/*ltm json config only has one for each stream.*/
	for (i = 0; i < props->num; i++) {
		if (strncmp(prop->name, "LTM", DRM_PROP_NAME_LEN + 1) == 0) {
			ltm_json_obj = prop->json_obj;
			found = true;
			DTESTLOGI("[%s] get the LTM json obj", __func__);
			break;
		}
		prop++;
	}

	if (!found) {
		DTESTLOGI("[%s] can not get the LTM json obj", __func__);
		goto final;
	}
	if (is_stream)
		frame_nums = stream->dtest_frame->num;
	else
		frame_nums = dtest_frames.num;

	for (i = 0; i < frame_nums - 1; i++) {
		if (is_stream)
			frame = &stream->dtest_frame->frames[i];
		else
			frame = &dtest_frames.frames[i];
		pthread_mutex_lock(&lock);
		time_t start_wait_time = time(NULL);
		bool continue_flag = false;
		while (!frame->commit_done) {
			pthread_cond_wait(&cond, &lock);
			time_t end_wait_time = time(NULL);
			double time_diff = difftime(end_wait_time, start_wait_time);
			if (time_diff > TIME_OUT) {
				continue_flag = true;
				break;
			}
		}
		if (continue_flag) {
			pthread_mutex_unlock(&lock);
			continue;
		}
		pthread_mutex_unlock(&lock);
		update_ltm_slope_bias(ltm_json_obj, dev, i);
	}

final:
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	struct device dev;
	char *device = NULL;
	unsigned int i = 0, j = 0, k = 0;
	int ret;
	dtest_status status;
	int opt = 1;
	cJSON *root = NULL;
	uint64_t config_type[] = { DRM_CLIENT_CAP_ATOMIC, DRM_CLIENT_CAP_WRITEBACK_CONNECTORS };
	uint64_t config_value[] = { 1, 1 };
	struct device_config *dev_config = NULL;
	char dump_folder[256] = { "." };
	char case_name[128] = { 0 };
	pthread_t thread = 0;
	pthread_t *stream_threads = NULL;
	pthread_t thread_update_ltm = 0;
	uint32_t query_num = 0, recycle_times = 1;
	uint32_t run_count = 1;
	bool is_stream = false;
	struct dump_stream_params *dump_stream_params = NULL;
	char vdp_base_str[20] = { '\0' };
	uint32_t vdp_out_addr = 0;
#if CONFIG_VERISILICON_MD5_CHECK
	bool global_result = true;
#endif
#if CONFIG_VERISILICON_STREAM_DEBUG
	time_t start_time, end_time;
	long int round_used_time = 0, total_used_time = 0;
#endif
	struct arguments args = {
		.reset = false,
		.clear = false,
		.case_path = NULL,
		.raw = false,
		.check = false,
		.clean = false,
		.vdp_dirty = false,
	};

	dtest_frame_t dtest_frames = { 0 };
	dtest_stream_t dtest_streams = { 0 };
	memset(&dev, 0, sizeof dev);

	while (opt < argc) {
		const char *arg = argv[opt];
		if (strcmp("-D", arg) == 0 && ((opt + 1) < argc)) {
			device = argv[opt + 1];
			opt += 2;
		} else if (strcmp("-F", arg) == 0 && ((opt + 1) < argc)) {
			parse_fill_patterns(argv[opt + 1]);
			opt += 2;
		} else if (strcmp("-o", arg) == 0 && ((opt + 1) < argc)) {
			memset(dump_folder, 0, sizeof(dump_folder));
			memcpy(dump_folder, argv[opt + 1], strlen(argv[opt + 1]));
			opt += 2;
		} else if (strcmp("-case", arg) == 0 && ((opt + 1) < argc)) {
			args.case_path = argv[opt + 1];
			opt += 2;
		} else if (strcmp("-reset", arg) == 0) {
			args.reset = true;
			opt += 1;
		} else if (strcmp("-clear", arg) == 0) {
			args.clear = true;
			opt += 1;
		} else if (strcmp("-raw", arg) == 0) {
			args.raw = true;
			opt += 1;
		} else if (strcmp("-clean", arg) == 0) {
			args.clean = true;
			opt += 1;
		} else if (strcmp("-v", arg) == 0 && ((opt + 1) < argc)) {
			memset(vdp_base_str, 0, sizeof(vdp_base_str));
			memcpy(vdp_base_str, argv[opt + 1], strlen(argv[opt + 1]));
			sscanf(vdp_base_str + 2, "%x", &vdp_out_addr);
			args.vdp_dirty = true;
			opt += 2;
		}
#if CONFIG_VERISILICON_MD5_CHECK
		else if (strcmp("-c", arg) == 0) {
			args.check = true;
			opt += 1;
		}
#endif
		else if (strcmp("-h", arg) == 0) {
			usage(argv[0]);
			return 0;
		} else if (strcmp("-p", arg) == 0) {
			recycle_times = atoi(argv[opt + 1]);
			opt += 2;
		} else {
			fprintf(stderr, "Unknown argument %s\n", argv[opt]);
			return -1;
		}
	}

	if (!args.case_path) {
		DTESTLOGE("Required parameter '-case'");
		usage(argv[0]);
		return -1;
	}

	status = __parse_case_name(args.case_path, case_name);
	CHECK_STATUS(-1, status, final);

	status = __check_dump_folder(dump_folder);
	CHECK_STATUS(-1, status, final);

	root = dtest_load_json(args.case_path);
	CHECK_PTR(-1, root, final);

	status = dpu_load_frames(root, &dtest_frames, &dtest_streams, &is_stream);
	CHECK_STATUS(-1, status, final);

	status = dpu_algorithm_lib_init(&dev.dpu_algorithm);
	CHECK_STATUS(-1, status, final);

	dev.fd = dpu_open_device(device, "vs-drm");
	if (dev.fd < 0) {
		if (device)
			DTESTLOGE("Open device %s vs-drm fail.", device);
		GOTO_LABEL(-1, final);
	}

	dev_config = dpu_create_device_config(config_type, config_value, ARRAY_SIZE(config_type),
					      ARRAY_SIZE(config_value));
	CHECK_PTR(-1, dev_config, final);

	ret = dpu_load_device_config(dev.fd, dev_config, ARRAY_SIZE(config_type));
	if (ret) {
		GOTO_LABEL(-1, final);
	}

	dev.resources = dpu_get_device_resources(&dev);
	CHECK_PTR(-1, dev.resources, final);

#ifdef CONFIG_VERISILICON_ARM64_QEMU
	dev.is_qemu = true;
#else
	dev.is_qemu = false;
#endif

	/*vcmd need create buffer befor set_frame*/
	struct drm_vs_query_feature_cap feature_cap = {
		.type = VS_FEATURE_CAP_USE_VCMD,
		.cap = 0,
	};
	drmIoctl(dev.fd, DRM_IOCTL_VS_GET_FEATURE_CAP, &feature_cap);
	dev.use_vcmd = feature_cap.cap;

	if (dev.use_vcmd) {
		/*create wb and plane buffer*/
		DTESTLOGI("prepare all plane and wb buffers");
		if (is_stream) {
			for (i = 0; i < dtest_streams.num; i++) {
				for (j = 0; j < dtest_streams.streams[i].dtest_frame->num; j++) {
					frame_t *frame =
						&dtest_streams.streams[i].dtest_frame->frames[j];
					status = dpu_create_wb_plane_buffer(&dev, frame);
					CHECK_STATUS(-1, status, final);
				}
			}
		} else {
			for (i = 0; i < dtest_frames.num; i++) {
				frame_t *frame = &dtest_frames.frames[i];
				status = dpu_create_wb_plane_buffer(&dev, frame);
				CHECK_STATUS(-1, status, final);
			}
		}
	}

	struct dump_params dump_param = {
		.dtest_frames = dtest_frames,
		.status = status,
		.dev = &dev,
		.dump_folder = dump_folder,
		.case_name = case_name,
		.args = args,
	};

	if (is_stream) {
		stream_threads = (pthread_t *)calloc(dtest_streams.num, sizeof(pthread_t));

		dump_stream_params = (struct dump_stream_params *)calloc(
			dtest_streams.num, sizeof(struct dump_stream_params));

		stream_lock = (pthread_mutex_t *)calloc(dtest_streams.num, sizeof(pthread_mutex_t));
		stream_cond = (pthread_cond_t *)calloc(dtest_streams.num, sizeof(pthread_cond_t));
		for (i = 0; i < dtest_streams.num; i++) {
			pthread_mutex_init(&stream_lock[dtest_streams.streams[i].stream_id], NULL);
			pthread_cond_init(&stream_cond[dtest_streams.streams[i].stream_id], NULL);
		}
#if CONFIG_VERISILICON_MD5_CHECK
		stream_golden = (char ***)calloc(dtest_streams.num, sizeof(char **));
		CHECK_PTR(-1, stream_golden, final);
		stream_result = (char ***)calloc(dtest_streams.num, sizeof(char ***));
		CHECK_PTR(-1, stream_result, final);
		for (i = 0; i < dtest_streams.num; i++) {
			stream_golden[i] = (char **)calloc(
				dtest_streams.streams[i].dtest_frame->frames[0].pipe.num,
				sizeof(char *));
			CHECK_PTR(-1, stream_golden[i], final);

			stream_result[i] = (char **)calloc(
				dtest_streams.streams[i].dtest_frame->frames[0].pipe.num,
				sizeof(char *));
			CHECK_PTR(-1, stream_result[i], final);
		}

		for (i = 0; i < dtest_streams.num; i++) {
			for (j = 0; j < dtest_streams.streams[i].dtest_frame->frames[0].pipe.num;
			     j++) {
				stream_golden[i][j] = dtest_streams.streams[i]
							      .dtest_frame->frames[0]
							      .pipe.pipes[j]
							      .md5_golden;
				stream_result[i][j] = (char *)calloc(MD5_BUFFER_SIZE, sizeof(char));
			}
		}
#endif
	} else {
		ret = pthread_create(&thread, NULL, dump_wb_buffer_thread, (void *)&dump_param);
		if (ret) {
			DTESTLOGE("---------Failed to create dump_wb_buffer thread!---------\n");
			GOTO_LABEL(-1, final);
		}
	}

	/*vcmd need update ltm slope and bias real-time*/
	if (dev.use_vcmd && !dev.is_qemu) {
		struct update_ltm_params update_ltm_param = { 0 };
		if (is_stream) {
			update_ltm_param.dev = &dev;
			update_ltm_param.dtest_streams = dtest_streams;
			update_ltm_param.is_stream = true;
		} else {
			update_ltm_param.dev = &dev;
			update_ltm_param.dtest_frames = dtest_frames;
			update_ltm_param.is_stream = false;
		}
		/*struct update_ltm_params update_ltm_param = {
			.dev = &dev,
			.dtest_frames = dtest_frames,
		};*/

		ret = pthread_create(&thread_update_ltm, NULL, update_ltm_thread,
				     (void *)&update_ltm_param);
		if (ret) {
			DTESTLOGE("Failed to create update_ltm_param thread!\n");
		}
	}

	/* init setting for VCMD/DC exception notification */
	ret = dtest_init_excp_notif(&dev);
	if (ret)
		DTESTLOGE("init exception notification failed!");

	if (args.vdp_dirty) {
		status = initialize_virtual_display(&dev, vdp_out_addr);
		CHECK_STATUS(-1, status, exit);
		dev.use_vdp = true;
	}

	if (is_stream) {
		DTESTLOGI("------recycle_times:%u---------\n", recycle_times);
		for (; run_count <= recycle_times; run_count++) {
#if CONFIG_VERISILICON_STREAM_DEBUG
			start_time = time(NULL);
#endif
			DTESTLOGI("--------------begin recycle:%u--------------\n", run_count);
			DTESTLOGI("--------------dtest_streams.num:%u--------------\n",
				  dtest_streams.num);

			for (i = 0; i < dtest_streams.num; i++) {
				DTESTLOGI("-----stream%d.frame_nums:%u------\n", i,
					  dtest_streams.streams[i].dtest_frame->num);
				dump_stream_params[i].dev = &dev;
				dump_stream_params[i].case_name = case_name;
				dump_stream_params[i].dump_folder = dump_folder;
				dump_stream_params[i].run_count = run_count;
				dump_stream_params[i].stream = &dtest_streams.streams[i];
				dump_stream_params[i].args = args;
				ret = pthread_create(&stream_threads[i], NULL,
						     pressure_stream_dump_wb_buffer_thread,
						     (void *)&dump_stream_params[i]);
				if (ret) {
					DTESTLOGE(
						"---------Failed to create pressure_stream_dump_wb_buffer thread!---------\n");
					GOTO_LABEL(-1, final);
				}

				for (j = 0; j < dtest_streams.streams[i].dtest_frame->num; j++) {
					frame_t *frame =
						&dtest_streams.streams[i].dtest_frame->frames[j];
					dtest_pipe_t *pipes = &frame->pipe;

					if (dev.use_vcmd) {
						query_num = 0;
						feature_cap.type =
							VS_FEATURE_CAP_VCMD_CMDBUF_REMAINING;
						feature_cap.cap = 0;
						drmIoctl(dev.fd, DRM_IOCTL_VS_GET_FEATURE_CAP,
							 &feature_cap);
						while (!feature_cap.cap) {
							sleep(2);
							drmIoctl(dev.fd,
								 DRM_IOCTL_VS_GET_FEATURE_CAP,
								 &feature_cap);
							query_num++;
							if (query_num > 50) {
								DTESTLOGE(
									"[%s] wait cmdbuf free time out!",
									__func__);
								break;
							}
						}
						if (feature_cap.cap)
							status = dpu_set_frame(&dev, frame);
					} else
						status = dpu_set_frame(&dev, frame);
					if (status != DTEST_SUCCESS && i > 0 && j > 0)
						dpu_clear_frame_FB(
							&dev, &dtest_streams.streams[i]
								       .dtest_frame->frames[j - 1]);

					CHECK_STATUS(-1, status, exit);
					DTESTLOGI("--------set frame %u done--------\n", j);

					if (!dev.use_vcmd)
						dpu_wait_signal(&dev, pipes);
					pthread_mutex_lock(&stream_lock[i]);
					frame->commit_done = true;
					pthread_cond_signal(&stream_cond[i]);
					pthread_mutex_unlock(&stream_lock[i]);
				}
				DTESTLOGI("--------------wait dump stream %u--------------\n", i);
				pthread_join(stream_threads[i], NULL);
				DTESTLOGI("--------------dump stream %u done--------------\n", i);
			}
#if CONFIG_VERISILICON_MD5_CHECK
			for (k = 0; k < dtest_streams.num; k++) {
				for (int m = 0;
				     m < dtest_streams.streams[k].dtest_frame->frames[0].pipe.num;
				     m++) {
					if (strcmp(stream_golden[k][m], stream_result[k][m]) != 0) {
						global_result = false;
						DTESTLOGI(
							"---------%d round stream_%d wb%d check md5 failed---------\n",
							run_count, k, m);
					} else {
						DTESTLOGI(
							"---------%d round stream_%d wb%d check md5 pass---------\n",
							run_count, k, m);
					}
				}
			}
#endif
#if CONFIG_VERISILICON_STREAM_DEBUG
			end_time = time(NULL);
			round_used_time = (end_time - start_time);
			DTESTLOGE("---------round_%d used_time:%ld---------\n", run_count,
				  round_used_time);
			total_used_time += round_used_time;
#endif
		}
	} else {
		for (i = 0; i < dtest_frames.num; i++) {
			frame_t *frame = &dtest_frames.frames[i];
			dtest_pipe_t *pipes = &frame->pipe;

			if (dev.use_vcmd) {
				query_num = 0;
				feature_cap.type = VS_FEATURE_CAP_VCMD_CMDBUF_REMAINING;
				feature_cap.cap = 0;
				drmIoctl(dev.fd, DRM_IOCTL_VS_GET_FEATURE_CAP, &feature_cap);
				while (!feature_cap.cap) {
					sleep(2);
					drmIoctl(dev.fd, DRM_IOCTL_VS_GET_FEATURE_CAP,
						 &feature_cap);
					query_num++;
					if (query_num > 50) {
						DTESTLOGE("[%s] wait cmdbuf free time out!",
							  __func__);
						break;
					}
				}
				if (feature_cap.cap)
					status = dpu_set_frame(&dev, frame);
			} else
				status = dpu_set_frame(&dev, frame);

			if (status != DTEST_SUCCESS && i > 0)
				dpu_clear_frame_FB(&dev, &dtest_frames.frames[i - 1]);
			CHECK_STATUS(-1, status, exit);

			/* the unit of random is second */
			if (frame->random) {
				sleep(frame->random);
				printf("---------resetting---------\n");
				dpu_device_software_reset(dev.fd, VS_RESET);
				printf("---------reset done---------\n");
			}

			/* it will choose whether to wait vblank or wb frm done
			 * according to wb point.
			 * if use_vcmd, not wait.
			 */
			if (!dev.use_vcmd)
				dpu_wait_signal(&dev, pipes);

			dtest_get_hist_info(&dev, frame);
			dtest_get_ops_result(&dev, frame);
			dtest_get_histogram_result(&dev, frame);
			pthread_mutex_lock(&lock);
			frame->commit_done = true;
			pthread_cond_signal(&cond);
			pthread_mutex_unlock(&lock);
		}
	}
#if CONFIG_VERISILICON_STREAM_DEBUG
	DTESTLOGI("---------multi stream used total time:%ld---------\n", total_used_time);
#endif

exit:
	if (is_stream) {
		if (status != DTEST_SUCCESS)
			pthread_cancel(stream_threads[i]);
	} else {
		if (status != DTEST_SUCCESS) {
			pthread_cancel(thread);
			if (dev.use_vcmd && !dev.is_qemu)
				pthread_cancel(thread_update_ltm);
		}
		pthread_join(thread, NULL);
		if (dev.use_vcmd && !dev.is_qemu)
			pthread_join(thread_update_ltm, NULL);
	}
final:
#if CONFIG_VERISILICON_MD5_CHECK
	if (!global_result)
		DTESTLOGE("---------multi stream check:fail---------\n");
	if (is_stream && global_result)
		DTESTLOGE("---------multi stream check:pass---------\n");
#endif
	if (args.clear) {
		if (is_stream) {
			for (i = 0; i < dtest_streams.num; i++) {
				dpu_clear_all_properties(&dev,
							 dtest_streams.streams[i].dtest_frame);
			}
		} else {
			dpu_clear_all_properties(&dev, &dtest_frames);
		}
	}

	if (is_stream) {
		for (k = 0; k < dtest_streams.num; k++) {
			dpu_clear_all_pipes(&dev, dtest_streams.streams[k].dtest_frame);
		}
	} else {
		dpu_clear_all_pipes(&dev, &dtest_frames);
	}
	i += (status != DTEST_SUCCESS) ? 1 : 0;
	j += (status != DTEST_SUCCESS) ? 1 : 0;
	if (is_stream)
		dpu_clear_frame_FB(&dev, &dtest_streams.streams[i - 1].dtest_frame->frames[j - 1]);
	else
		dpu_clear_frame_FB(&dev, &dtest_frames.frames[i - 1]);

	if (args.reset) {
		int errcode = dpu_device_software_reset(dev.fd, VS_RESET);

		if (errcode)
			DTESTLOGE("Reset fail %d\n", errcode);
	}

	if (args.clean) {
		int errcode = dpu_device_software_clean(dev.fd, VS_CLE_SW_RESET);

		if (errcode)
			DTESTLOGE("SW Clean fail %d\n", errcode);
	}

	if (is_stream) {
		for (i = 0; i < dtest_streams.num; i++) {
			dpu_free_frames(dtest_streams.streams[i].dtest_frame);
			dtest_streams.streams[i].stream_id = 0;
			free(dtest_streams.streams[i].dtest_frame);
			dtest_streams.streams[i].dtest_frame = NULL;
		}
		dtest_streams.num = 0;
		free(dtest_streams.streams);
		dtest_streams.streams = NULL;
	} else
		dpu_free_frames(&dtest_frames);

	if (root)
		cJSON_Delete(root);

	if (dev_config)
		free(dev_config);

	dpu_free_device(&dev);

	return ret;
}
