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

void CDECL (*wrap_log_info)(UNSAFEPTR(const char *) s);
void CDECL (*wrap_log_warn)(UNSAFEPTR(const char *) s);
void CDECL (*wrap_log_error)(UNSAFEPTR(const char *) s);
void CDECL (*wrap_make_sav_dir)(void);
void CDECL (*wrap_make_valid_path)(UNSAFEPTR(const char *) dir, UNSAFEPTR(const char *) fname, UNSAFEPTR(const char *) dst, int len);
void CDECL (*wrap_notify_image_update)(int id, int width, int height, UNSAFEPTR(const uint32_t *) pixels);
void CDECL (*wrap_notify_image_free)(int id);
void CDECL (*wrap_render_image_normal)(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
void CDECL (*wrap_render_image_add)(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
void CDECL (*wrap_render_image_dim)(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
void CDECL (*wrap_render_image_rule)(int src_img, int rule_img, int threshold);
void CDECL (*wrap_render_image_melt)(int src_img, int rule_img, int progress);
void CDECL (*wrap_render_image_3d_normal)(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
void CDECL (*wrap_render_image_3d_add)(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
void CDECL (*wrap_reset_lap_timer)(UNSAFEPTR(uint64_t *) origin);
uint64_t CDECL (*wrap_get_lap_timer_millisec)(UNSAFEPTR(uint64_t *) origin);
void CDECL (*wrap_play_sound)(int stream, UNSAFEPTR(void *) wave);
void CDECL (*wrap_stop_sound)(int stream);
void CDECL (*wrap_set_sound_volume)(int stream, float vol);
int CDECL (*wrap_is_sound_finished)(int stream);
bool CDECL (*wrap_play_video)(UNSAFEPTR(const char *) fname, bool is_skippable);
void CDECL (*wrap_stop_video)(void);
bool CDECL (*wrap_is_video_playing)(void);
void CDECL (*wrap_update_window_title)(void);
bool CDECL (*wrap_is_full_screen_supported)(void);
bool CDECL (*wrap_is_full_screen_mode)(void);
void CDECL (*wrap_enter_full_screen_mode)(void);
void CDECL (*wrap_leave_full_screen_mode)(void);
void CDECL (*wrap_get_system_locale)(UNSAFEPTR(char *) dst, int len);
void CDECL (*wrap_speak_text)(UNSAFEPTR(const char *) text);
void CDECL (*wrap_set_continuous_swipe_enabled)(bool is_enabled);
void CDECL (*wrap_free_shared)(UNSAFEPTR(void *) p);
bool CDECL (*wrap_check_file_exist)(UNSAFEPTR(const char *) file_name);
UNSAFEPTR(void *) CDECL (*wrap_get_file_contents)(UNSAFEPTR(const char *) file_name, UNSAFEPTR(int *) len);
void CDECL (*wrap_open_save_file)(UNSAFEPTR(const char *) file_name);
void CDECL (*wrap_write_save_file)(int b);
void CDECL (*wrap_close_save_file)(void);

#ifdef USE_EDITOR
bool CDECL (*wrap_is_continue_pushed)(void);
bool CDECL (*wrap_is_next_pushed)(void);
bool CDECL (*wrap_is_stop_pushed)(void);
bool CDECL (*wrap_is_script_opened)(void);
UNSAFEPTR(const char *) CDECL (*wrap_get_opened_script)(void);
bool CDECL (*wrap_is_exec_line_changed)(void);
bool CDECL (*wrap_get_changed_exec_line)(void);
void CDECL (*wrap_on_change_running_state)(int running, int request_stop);
void CDECL (*wrap_on_load_script)(void);
void CDECL (*wrap_on_change_position)(void);
void CDECL (*wrap_on_update_variable)(void);
#endif

/*
 * Initializer
 *  - On C#, please call init_hal_func_table() because wrap_* is not visible to C# code
 *  - On Swift, please assign to wrap_* directly because they are visible to Swift code
 */

#if defined(USE_CSHARP)
void init_hal_func_table
(
 void CDECL (*p_log_info)(UNSAFEPTR(const char *) s),
 void CDECL (*p_log_warn)(UNSAFEPTR(const char *) s),
 void CDECL (*p_log_error)(UNSAFEPTR(const char *) s),
 void CDECL (*p_make_sav_dir)(void),
 void CDECL (*p_make_valid_path)(UNSAFEPTR(const char *) dir, UNSAFEPTR(const char *) fname, UNSAFEPTR(const char *) dst, int len),
 void CDECL (*p_notify_image_update)(int id, int width, int height, UNSAFEPTR(const uint32_t *) pixels),
 void CDECL (*p_notify_image_free)(int id),
 void CDECL (*p_render_image_normal)(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha),
 void CDECL (*p_render_image_add)(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha),
 void CDECL (*p_render_image_dim)(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha),
 void CDECL (*p_render_image_rule)(int src_img, int rule_img, int threshold),
 void CDECL (*p_render_image_melt)(int src_img, int rule_img, int progress),
 void CDECL (*p_render_image_3d_normal)(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha),
 void CDECL (*p_render_image_3d_add)(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha),
 void CDECL (*p_reset_lap_timer)(UNSAFEPTR(uint64_t *) origin),
 uint64_t CDECL (*p_get_lap_timer_millisec)(UNSAFEPTR(uint64_t *) origin),
 void CDECL (*p_play_sound)(int stream, UNSAFEPTR(void *) wave),
 void CDECL (*p_stop_sound)(int stream),
 void CDECL (*p_set_sound_volume)(int stream, float vol),
 bool CDECL (*p_is_sound_finished)(int stream),
 bool CDECL (*p_play_video)(UNSAFEPTR(const char *) fname, bool is_skippable),
 void CDECL (*p_stop_video)(void),
 bool CDECL (*p_is_video_playing)(void),
 void CDECL (*p_update_window_title)(void),
 bool CDECL (*p_is_full_screen_supported)(void),
 bool CDECL (*p_is_full_screen_mode)(void),
 void CDECL (*p_enter_full_screen_mode)(void),
 void CDECL (*p_leave_full_screen_mode)(void),
 void CDECL (*p_get_system_locale)(UNSAFEPTR(char *) dst, int len),
 void CDECL (*p_speak_text)(UNSAFEPTR(const char *) text),
 void CDECL (*p_set_continuous_swipe_enabled)(bool is_enabled),
 void CDECL (*p_free_shared)(UNSAFEPTR(void *) p),
 bool CDECL (*p_check_file_exist)(UNSAFEPTR(const char *) file_name),
 UNSAFEPTR(void *) CDECL (*p_get_file_contents)(UNSAFEPTR(const char *) file_name, UNSAFEPTR len),
 void CDECL (*p_open_save_file)(UNSAFEPTR(const char *) file_name),
 void CDECL (*p_write_save_file)(int b),
 void CDECL (*p_close_save_file)(void)
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
	wrap_check_file_exist = p_check_file_exist;
	wrap_get_file_contents = p_get_file_contents;
	wrap_open_save_file = p_open_save_file;
	wrap_write_save_file = p_write_save_file;
	wrap_close_save_file = p_close_save_file;
#ifdef USE_EDITOR
	wrap_is_continue_pushed = p_is_continue_pushed;
	wrap_is_next_pushed = p_is_next_pushed;
	wrap_is_stop_pushed = p_is_stop_pushed;
	wrap_is_script_opened = p_is_script_opened;
	wrap_get_opened_script = p_get_opened_script;
	wrap_is_exec_line_changed = p_is_exec_line_changed;
	wrap_get_changed_exec_line = p_get_changed_exec_line;
	wrap_on_change_running_state = p_on_change_running_state;
	wrap_on_load_script = p_on_load_script;
	wrap_on_change_running_state = p_on_change_running_state;
	wrap_on_update_variable = p_on_update_variable;
#endif
}
#endif /* defined(USE_CSHARP) */

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

	wrap_log_info((UNSAFEPTR(const char *))buf);

	return true;
}

bool log_warn(const char *s, ...)
{
	char buf[4096];
	va_list ap;

	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	wrap_log_info((UNSAFEPTR(const char *))buf);

	return true;
}

bool log_error(const char *s, ...)
{
	char buf[4096];
	va_list ap;

	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	wrap_log_info((UNSAFEPTR(const char *))buf);

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
	wrap_make_valid_path((UNSAFEPTR(const char *))dir, (UNSAFEPTR(const char *))fname, (UNSAFEPTR(char *))buf, sizeof(buf));

	ret = strdup(buf);
	if (ret == NULL) {
		log_memory();
		return NULL;
	}
	return ret;
}

void notify_image_update(struct image *img)
{
	wrap_notify_image_update(img->id, img->width, img->height, (UNSAFEPTR(uint32_t *))img->pixels);
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
	wrap_render_image_3d_normal(x1, y1, x2, y2, x3, y3, x4, y4, src_img->id, src_left, src_top, src_width, src_height, alpha);
}

void render_image_3d_add(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, struct image *src_img, int src_left, int src_top, int src_width, int src_height, int alpha)
{
	wrap_render_image_3d_add(x1, y1, x2, y2, x3, y3, x4, y4, src_img->id, src_left, src_top, src_width, src_height, alpha);
}

void reset_lap_timer(uint64_t *origin)
{
	wrap_reset_lap_timer((UNSAFEPTR(uint64_t *))origin);
}

uint64_t get_lap_timer_millisec(uint64_t *origin)
{
	uint64_t ret;
	ret = wrap_get_lap_timer_millisec((UNSAFEPTR(uint64_t *))origin);
	return ret;
}

#if defined(USE_UNITY)
bool play_sound(int stream, struct wave *w)
{
	wrap_play_sound(stream, (UNSAFEPTR(void *))w);
	return true;
}
#endif

#if defined(USE_UNITY)
bool stop_sound(int stream)
{
	wrap_stop_sound(stream);
	return true;
}
#endif

#if defined(USE_UNITY)
bool set_sound_volume(int stream, float vol)
{
	wrap_set_sound_volume(stream, vol);
	return true;
}
#endif

#if defined(USE_UNITY)
bool is_sound_finished(int stream)
{
	bool ret;
	ret =  wrap_is_sound_finished(stream);
	return ret;
}
#endif

bool play_video(const char *fname, bool is_skippable)
{
	bool ret;
	ret =  wrap_play_video((UNSAFEPTR(const char *))fname, is_skippable);
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
	wrap_get_system_locale((UNSAFEPTR(char *))buf, sizeof(buf));

	return buf;
}

void speak_text(const char *text)
{
	wrap_speak_text((UNSAFEPTR(const char *))text);
}

void set_continuous_swipe_enabled(bool is_enabled)
{
	wrap_set_continuous_swipe_enabled(is_enabled);
}

#if defined(USE_UNITY)
struct rfile {
	char *data;
	uint64_t size;
	uint64_t cur;
};
#endif

#if defined(USE_UNITY)
struct wfile {
	int dummy;
};
#endif

#if defined(USE_UNITY)
bool init_file(void)
{
	return true;
}
#endif

#if defined(USE_UNITY)
void cleanup_file(void)
{
}
#endif

#if defined(USE_UNITY)
bool check_file_exist(const char *dir, const char *file)
{
	char *path;
	bool ret;

	path = make_valid_path(dir, file);
	if (path == NULL)
		return false;

	ret = wrap_check_file_exist((intptr_t)path);
	free(path);

	if (!ret)
		return false;

	return true;
}
#endif

#if defined(USE_UNITY)
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
#endif

#if defined(USE_UNITY)
size_t get_rfile_size(struct rfile *rf)
{
	return rf->size;
}
#endif

#if defined(USE_UNITY)
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
#endif

#if defined(USE_UNITY)
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
#endif

#if defined(USE_UNITY)
void rewind_rfile(struct rfile *rf)
{
	rf->cur = 0;
}
#endif

#if defined(USE_UNITY)
void close_rfile(struct rfile *rf)
{
	wrap_free_shared((intptr_t)rf->data);
	free(rf);
}
#endif

#if defined(USE_UNITY)
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
#endif

#if defined(USE_UNITY)
size_t write_wfile(struct wfile *wf, const void *buf, size_t size)
{
	const char *p;
	size_t i;

	p = buf;
	for (i = 0; i < size; i++)
		wrap_write_save_file((int)*p++);
	return size;
}
#endif

#if defined(USE_UNITY)
void close_wfile(struct wfile *wf)
{
	wrap_close_save_file();
}
#endif

#if defined(USE_UNITY)
void remove_file(const char *dir, const char *file)
{
	// TODO
}
#endif

#if defined(USE_EDITOR)
bool is_continue_pushed(void)
{
	return wrap_is_continue_pushed();
}
#endif

#if defined(USE_EDITOR)
bool is_next_pushed(void)
{
	return wrap_is_next_pushed();
}
#endif

#if defined(USE_EDITOR)
bool is_stop_pushed(void)
{
	return wrap_is_stop_pushed();
}
#endif

#if defined(USE_EDITOR)
bool is_script_opened(void)
{
	return wrap_is_script_opened();
}
#endif

#if defined(USE_EDITOR)
const char *get_opened_script(void)
{
	return wrap_get_opened_script();
}
#endif

#if defined(USE_EDITOR)
bool is_exec_line_changed(void)
{
	return wrap_is_exec_line_changed();
}
#endif

#if defined(USE_EDITOR)
int get_changed_exec_line(void)
{
	return wrap_get_changed_exec_line();
}
#endif

#if defined(USE_EDITOR)
void on_change_running_state(bool running, bool request_stop)
{
	return wrap_on_change_running_state(running, request_stop);
}
#endif

#if defined(USE_EDITOR)
void on_load_script(void)
{
	return wrap_on_load_script();
}
#endif

#if defined(USE_EDITOR)
void on_change_position(void)
{
	return wrap_on_change_position();
}
#endif

#if defined(USE_EDITOR)
void on_update_variable(void)
{
	return wrap_on_update_variable();
}
#endif
