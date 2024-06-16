/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * HAL Wrapper for Foreign Languages
 */

#include "polarisengine.h"

/*
 * Function Pointers
 */

void POLARISENGINEAPI (*wrap_log_info)(intptr_t s);
void POLARISENGINEAPI (*wrap_log_warn)(intptr_t s);
void POLARISENGINEAPI (*wrap_log_error)(intptr_t s);
void POLARISENGINEAPI (*wrap_make_sav_dir)(void);
void POLARISENGINEAPI (*wrap_make_valid_path)(intptr_t dir, intptr_t fname, intptr_t dst, int len);
void POLARISENGINEAPI (*wrap_notify_image_update)(int id, int width, int height, intptr_t pixels);
void POLARISENGINEAPI (*wrap_notify_image_free)(int id);
void POLARISENGINEAPI (*wrap_render_image_normal)(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
void POLARISENGINEAPI (*wrap_render_image_add)(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
void POLARISENGINEAPI (*wrap_render_image_dim)(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
void POLARISENGINEAPI (*wrap_render_image_rule)(int src_img, int rule_img, int threshold);
void POLARISENGINEAPI (*wrap_render_image_melt)(int src_img, int rule_img, int progress);
void POLARISENGINEAPI (*wrap_render_image_3d_normal)(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
void POLARISENGINEAPI (*wrap_render_image_3d_add)(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
void POLARISENGINEAPI (*wrap_reset_lap_timer)(intptr_t origin);
int64_t POLARISENGINEAPI (*wrap_get_lap_timer_millisec)(intptr_t origin);
void POLARISENGINEAPI (*wrap_play_sound)(int stream, intptr_t wave);
void POLARISENGINEAPI (*wrap_stop_sound)(int stream);
void POLARISENGINEAPI (*wrap_set_sound_volume)(int stream, float vol);
bool POLARISENGINEAPI (*wrap_is_sound_finished)(int stream);
bool POLARISENGINEAPI (*wrap_play_video)(intptr_t fname, bool is_skippable);
void POLARISENGINEAPI (*wrap_stop_video)(void);
bool POLARISENGINEAPI (*wrap_is_video_playing)(void);
void POLARISENGINEAPI (*wrap_update_window_title)(void);
bool POLARISENGINEAPI (*wrap_is_full_screen_supported)(void);
bool POLARISENGINEAPI (*wrap_is_full_screen_mode)(void);
void POLARISENGINEAPI (*wrap_enter_full_screen_mode)(void);
void POLARISENGINEAPI (*wrap_leave_full_screen_mode)(void);
void POLARISENGINEAPI (*wrap_get_system_locale)(intptr_t dst, int len);
void POLARISENGINEAPI (*wrap_speak_text)(intptr_t text);
void POLARISENGINEAPI (*wrap_set_continuous_swipe_enabled)(bool is_enabled);
void POLARISENGINEAPI (*wrap_free_shared)(intptr_t p);
intptr_t POLARISENGINEAPI (*wrap_get_file_contents)(intptr_t file_name, intptr_t len);
void POLARISENGINEAPI (*wrap_open_save_file)(intptr_t file_name);
void POLARISENGINEAPI (*wrap_write_save_file)(int b);
void POLARISENGINEAPI (*wrap_close_save_file)(void);

/*
 * Initialize
 */

void init_hal_func_table
(
	void POLARISENGINEAPI (*p_log_info)(intptr_t s),
	void POLARISENGINEAPI (*p_log_warn)(intptr_t s),
	void POLARISENGINEAPI (*p_log_error)(intptr_t s),
	void POLARISENGINEAPI (*p_make_sav_dir)(void),
	void POLARISENGINEAPI (*p_make_valid_path)(intptr_t dir, intptr_t fname, intptr_t dst, int len),
	void POLARISENGINEAPI (*p_notify_image_update)(int id, int width, int height, intptr_t pixels),
	void POLARISENGINEAPI (*p_notify_image_free)(int id),
	void POLARISENGINEAPI (*p_render_image_normal)(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha),
	void POLARISENGINEAPI (*p_render_image_add)(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha),
	void POLARISENGINEAPI (*p_render_image_dim)(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha),
	void POLARISENGINEAPI (*p_render_image_rule)(int src_img, int rule_img, int threshold),
	void POLARISENGINEAPI (*p_render_image_melt)(int src_img, int rule_img, int progress),
	void POLARISENGINEAPI (*p_render_image_3d_normal)(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha),
	void POLARISENGINEAPI (*p_render_image_3d_add)(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha),
	void POLARISENGINEAPI (*p_reset_lap_timer)(intptr_t origin),
	int64_t POLARISENGINEAPI (*p_get_lap_timer_millisec)(intptr_t origin),
	void POLARISENGINEAPI (*p_play_sound)(int stream, intptr_t wave),
	void POLARISENGINEAPI (*p_stop_sound)(int stream),
	void POLARISENGINEAPI (*p_set_sound_volume)(int stream, float vol),
	bool POLARISENGINEAPI (*p_is_sound_finished)(int stream),
	bool POLARISENGINEAPI (*p_play_video)(intptr_t fname, bool is_skippable),
	void POLARISENGINEAPI (*p_stop_video)(void),
	bool POLARISENGINEAPI (*p_is_video_playing)(void),
	void POLARISENGINEAPI (*p_update_window_title)(void),
	bool POLARISENGINEAPI (*p_is_full_screen_supported)(void),
	bool POLARISENGINEAPI (*p_is_full_screen_mode)(void),
	void POLARISENGINEAPI (*p_enter_full_screen_mode)(void),
	void POLARISENGINEAPI (*p_leave_full_screen_mode)(void),
	void POLARISENGINEAPI (*p_get_system_locale)(intptr_t dst, int len),
	void POLARISENGINEAPI (*p_speak_text)(intptr_t text),
	void POLARISENGINEAPI (*p_set_continuous_swipe_enabled)(bool is_enabled),
	void POLARISENGINEAPI (*p_free_shared)(intptr_t p),
	intptr_t POLARISENGINEAPI (*p_get_file_contents)(intptr_t file_name, intptr_t len),
	void POLARISENGINEAPI (*p_open_save_file)(intptr_t file_name),
	void POLARISENGINEAPI (*p_write_save_file)(int b),
	void POLARISENGINEAPI (*p_close_save_file)(void)
)
{
	wrap_log_info = p_log_info;
	wrap_log_warn = p_log_warn;
	wrap_log_error = p_log_error;
	wrap_make_sav_dir = p_make_sav_dir;
	wrap_make_valid_path = p_make_valid_path;
	wrap_notify_image_update = p_notify_image_update;
	wrap_notify_image_free = p_notify_image_free;
	wrap_render_image_normal = p_render_image_normal;
	wrap_render_image_add = p_render_image_add;
	wrap_render_image_dim = p_render_image_dim;
	wrap_render_image_rule = p_render_image_rule;
	wrap_render_image_melt = p_render_image_melt;
	wrap_render_image_3d_normal = p_render_image_3d_normal;
	wrap_render_image_3d_add = p_render_image_3d_add;
	wrap_reset_lap_timer = p_reset_lap_timer;
	wrap_get_lap_timer_millisec = p_get_lap_timer_millisec;
	wrap_play_sound = p_play_sound;
	wrap_stop_sound = p_stop_sound;
	wrap_set_sound_volume = p_set_sound_volume;
	wrap_is_sound_finished = p_is_sound_finished;
	wrap_play_video = p_play_video;
	wrap_stop_video = p_stop_video;
	wrap_is_video_playing = p_is_video_playing;
	wrap_update_window_title = p_update_window_title;
	wrap_is_full_screen_supported = p_is_full_screen_supported;
	wrap_is_full_screen_mode = p_is_full_screen_mode;
	wrap_enter_full_screen_mode = p_enter_full_screen_mode;
	wrap_leave_full_screen_mode = p_leave_full_screen_mode;
	wrap_get_system_locale = p_get_system_locale;
	wrap_speak_text = p_speak_text;
	wrap_set_continuous_swipe_enabled = p_set_continuous_swipe_enabled;
	wrap_free_shared = p_free_shared;
	wrap_get_file_contents = p_get_file_contents;
	wrap_open_save_file = p_open_save_file;
	wrap_write_save_file = p_write_save_file;
	wrap_close_save_file = p_close_save_file;
}

/*
 * Wrappers
 */

bool log_info(const char *s, ...)
{
	char buf[4096];
	va_list ap;

	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	wrap_log_info((intptr_t)buf);

	return true;
}

bool log_warn(const char *s, ...)
{
	char buf[4096];
	va_list ap;

	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	wrap_log_info((intptr_t)buf);

	return true;
}

bool log_error(const char *s, ...)
{
	char buf[4096];
	va_list ap;

	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	wrap_log_info((intptr_t)buf);

	return true;
}

bool make_sav_dir(void)
{
	wrap_make_sav_dir();
	return true;
}

char *make_valid_path(const char *dir, const char *fname)
{
	char buf[4096];
	char *ret;

	memset(buf, 0, sizeof(buf));
	wrap_make_valid_path((intptr_t)dir, (intptr_t)fname, (intptr_t)buf, sizeof(buf));

	ret = strdup(buf);
	if (ret == NULL) {
		log_memory();
		return NULL;
	}
	return ret;
}

void notify_image_update(struct image *img)
{
	wrap_notify_image_update(img->id, img->width, img->height, (intptr_t)img->pixels);
}

void notify_image_free(struct image *img)
{
	wrap_notify_image_free(img->id);
}

void render_image_normal(int dst_left, int dst_top, int dst_width, int dst_height, struct image *src_img, int src_left, int src_top, int src_width, int src_height, int alpha)
{
	if (dst_width == -1)
		dst_width = src_img->width;
	if (dst_height == -1)
		dst_height = src_img->height;
	if (src_width == -1)
		src_width = src_img->width;
	if (src_height == -1)
		src_height = src_img->height;

	wrap_render_image_normal(dst_left, dst_top, dst_width, dst_height, src_img->id, src_left, src_top, src_width, src_height, alpha);
}

void render_image_add(int dst_left, int dst_top, int dst_width, int dst_height, struct image *src_img, int src_left, int src_top, int src_width, int src_height, int alpha)
{
	if (dst_width == -1)
		dst_width = src_img->width;
	if (dst_height == -1)
		dst_height = src_img->height;
	if (src_width == -1)
		src_width = src_img->width;
	if (src_height == -1)
		src_height = src_img->height;

	wrap_render_image_add(dst_left, dst_top, dst_width, dst_height, src_img->id, src_left, src_top, src_width, src_height, alpha);
}

void render_image_dim(int dst_left, int dst_top, int dst_width, int dst_height, struct image *src_img, int src_left, int src_top, int src_width, int src_height, int alpha)
{
	if (dst_width == -1)
		dst_width = src_img->width;
	if (dst_height == -1)
		dst_height = src_img->height;
	if (src_width == -1)
		src_width = src_img->width;
	if (src_height == -1)
		src_height = src_img->height;

	wrap_render_image_dim(dst_left, dst_top, dst_width, dst_height, src_img->id, src_left, src_top, src_width, src_height, alpha);
}

void render_image_rule(struct image *src_img, struct image *rule_img, int threshold)
{
	wrap_render_image_rule(src_img->id, rule_img->id, threshold);
}

void render_image_melt(struct image *src_img, struct image *rule_img, int progress)
{
	wrap_render_image_melt(src_img->id, rule_img->id, progress);
}

void render_image_3d_normal(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, struct image *src_img, int src_left, int src_top, int src_width, int src_height, int alpha)
{
	wrap_render_image_3d_normal(x1, y1, x2, y2, x3, y3, x4, y4, (intptr_t)src_img, src_left, src_top, src_width, src_height, alpha);
}

void render_image_3d_add(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, struct image *src_img, int src_left, int src_top, int src_width, int src_height, int alpha)
{
	wrap_render_image_3d_add(x1, y1, x2, y2, x3, y3, x4, y4, (intptr_t)src_img, src_left, src_top, src_width, src_height, alpha);
}

void reset_lap_timer(uint64_t *origin)
{
	wrap_reset_lap_timer((intptr_t)origin);
}

uint64_t get_lap_timer_millisec(uint64_t *origin)
{
	uint64_t ret;
	ret = wrap_get_lap_timer_millisec((intptr_t)origin);
	return ret;
}

bool play_sound(int stream, struct wave *w)
{
	wrap_play_sound(stream, (intptr_t)w);
	return true;
}

bool stop_sound(int stream)
{
	wrap_stop_sound(stream);
	return true;
}

bool set_sound_volume(int stream, float vol)
{
	wrap_set_sound_volume(stream, vol);
	return true;
}

bool is_sound_finished(int stream)
{
	bool ret;
	ret =  wrap_is_sound_finished(stream);
	return ret;
}

bool play_video(const char *fname, bool is_skippable)
{
	bool ret;
	ret =  wrap_play_video((intptr_t)fname, is_skippable);
	return true;
}

void stop_video(void)
{
	wrap_stop_video();
}

bool is_video_playing(void)
{
	bool ret;
	ret = wrap_is_video_playing();
	return ret;
}

void update_window_title(void)
{
	wrap_update_window_title();
}

bool is_full_screen_supported(void)
{
	bool ret;
	ret = wrap_is_full_screen_supported();
	return ret;
}

bool is_full_screen_mode(void)
{
	bool ret;
	ret = wrap_is_full_screen_mode();
	return ret;
}

void enter_full_screen_mode(void)
{
	wrap_enter_full_screen_mode();
}

void leave_full_screen_mode(void)
{
	wrap_leave_full_screen_mode();
}

const char *get_system_locale(void)
{
	static char buf[64];

	memset(buf, 0, sizeof(buf));
	wrap_get_system_locale((intptr_t)buf, sizeof(buf));

	return buf;
}

void speak_text(const char *text)
{
	wrap_speak_text((intptr_t)text);
}

#if defined(POLARIS_ENGINE_TARGET_IOS) || defined(POLARIS_ENGINE_TARGET_ANDROID) || defined(POLARIS_ENGINE_TARGET_WASM)
void set_continuous_swipe_enabled(bool is_enabled)
{
	wrap_set_continuous_swipe_enabled(is_enabled);
}
#endif

/*
 * File API
 */

struct rfile {
	char *data;
	uint64_t size;
	uint64_t cur;
};

struct wfile {
	int dummy;
};

bool init_file(void)
{
	return true;
}

void cleanup_file(void)
{
}

bool check_file_exist(const char *dir, const char *file)
{
	intptr_t p;
	char *path;
	int len;

	path = make_valid_path(dir, file);
	if (path == NULL)
		return false;

	p = wrap_get_file_contents((intptr_t)path, (intptr_t)&len);
	free(path);

	if (p != 0) {
		wrap_free_shared((intptr_t)p);
		return true;
	}

	return false;
}

struct rfile *open_rfile(const char *dir, const char *file, bool save_data)
{
	struct rfile *rf;
	intptr_t p;
	char *path;
	int len;

	rf = malloc(sizeof(struct rfile));
	if (rf == NULL) {
		log_memory();
		return NULL;
	}

	path = make_valid_path(dir, file);
	if (path == NULL)
		return NULL;

	p = wrap_get_file_contents((intptr_t)path, (intptr_t)&len);
	free(path);

	if (p == 0) {
		free(rf);
		return NULL;
	}

	rf->data = (char *)p;
	rf->size = len;
	rf->cur = 0;
	return rf;
}

size_t get_rfile_size(struct rfile *rf)
{
	return rf->size;
}

size_t read_rfile(struct rfile *rf, void *buf, size_t size)
{
	size_t len;

	if (size <= rf->size - rf->cur)
		len = size;
	else
		len = rf->size - rf->cur;

	memcpy(buf, rf->data + rf->cur, len);
	rf->cur += len;

	return len;
}

const char *gets_rfile(struct rfile *rf, char *buf, size_t size)
{
	char *ptr;
	size_t len, ret;
	char c;

	ptr = buf;
	for (len = 0; len < size - 1; len++) {
		ret = read_rfile(rf, &c, 1);
		if (ret != 1) {
			*ptr = '\0';
			return len == 0 ? NULL : buf;
		}
		if (c == '\n' || c == '\0') {
			*ptr = '\0';
			return buf;
		}
		if (c == '\r') {
			if (read_rfile(rf, &c, 1) != 1) {
				*ptr = '\0';
				return buf;
			}
			if (c == '\n') {
				*ptr = '\0';
				return buf;
			}
			rf->cur--;
			*ptr = '\0';
			return buf;
		}
		*ptr++ = c;
	}
	*ptr = '\0';
	if (len == 0)
		return NULL;
	return buf;
}

void rewind_rfile(struct rfile *rf)
{
	rf->cur = 0;
}

void close_rfile(struct rfile *rf)
{
	wrap_free_shared((intptr_t)rf->data);
	free(rf);
}

struct wfile *open_wfile(const char *dir, const char *file)
{
	struct wfile *wf;

	wf = malloc(sizeof(struct wfile));
	if (wf == NULL) {
		log_memory();
		return NULL;
	}

	wrap_open_save_file((intptr_t)file);

	return wf;
}

size_t write_wfile(struct wfile *wf, const void *buf, size_t size)
{
	const char *p;
	size_t i;

	p = buf;
	for (i = 0; i < size; i++)
		wrap_write_save_file((int)*p++);
	return size;
}

void close_wfile(struct wfile *wf)
{
	wrap_close_save_file();
}

void remove_file(const char *dir, const char *file)
{
	// TODO
}
