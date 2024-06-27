/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * HAL API definition for Polaris Engine's runtime (engine-*)
 *
 * [HAL]
 *  HAL (Hardware Abstraction Layer) is a thin layer to port Polaris Engine to
 *  various platforms. For a porting, only the functions listed in this
 *  header file need to be implemented.
 *
 * [Note]
 *  - We use the utf-8 encoding for all "const char *" and "char *" string pointers
 *  - Return values of the non-const type "char *" must be free()-ed by callers
 */

#ifndef POLARIS_ENGINE_HAL_H
#define POLARIS_ENGINE_HAL_H

#include "types.h"

/**************
 * Structures *
 **************/

/*
 * Image Object:
 *  - We use "struct image *" for images
 *  - The operations to "struct image *" are common to all ports and written in image.[ch]
 */
struct image;

/*
 * Sound Object:
 *  - We use "struct wave *" for sound streams
 *  - The operations to "struct wave *" are common to almost all ports and written in wave.[ch]
 *  - The only exception is Android NDK and ndkwave.c implements an alternative using MediaPlayer class
 *  - We will remove ndkwave.c and integrate to wave.c in the near future
 */
struct wave;

/***********
 * Logging *
 ***********/

/*
 * Puts a info log with printf formats.
 *  - An "info" level log will be put into log.txt file
 *  - Note that sound threads cannot use logging
 */
bool log_info(const char *s, ...);

/*
 * Puts a warn log with printf formats.
 *  - A "warn" level log will be put into log.txt file and a dialog will be shown
 *  - Note that sound threads cannot use logging
 */
bool log_warn(const char *s, ...);

/*
 * Puts a error log with printf formats.
 *  - An "error" level log will be put into log.txt file and a dialog will be shown
 *  - Note that sound threads cannot use logging
 */
bool log_error(const char *s, ...);

/*********************
 * Path Manipulation *
 *********************/

/*
 * Creates a save directory if it is does not exist.
 */
bool make_sav_dir(void);

/*
 * Creates an effective path string from a directory name and a file name.
 *  - This function absorbs OS-specific path handling
 *  - Resulting strings can be passed to open_rfile() and open_wfile()
 *  - Resulting strings must be free()-ed by callers
 */
char *make_valid_path(const char *dir, const char *fname);

/************************
 * Texture Manipulation *
 ************************/

/*
 * Notifies an image update.
 *  - This function tells a HAL that an image needs to be uploaded to GPU
 *  - A HAL can upload images to GPU at an appropriate time
 */
void notify_image_update(struct image *img);

/*
 * Notifies an image free.
 *  - This function tells a HAL that an image is no longer used
 *  - This function must be called from destroy_image() only
 */
void notify_image_free(struct image *img);

/*
 * Returns if RGBA values have to be reversed to BGRA.
 */
#if defined(POLARIS_ENGINE_TARGET_ANDROID) || defined(POLARIS_ENGINE_TARGET_WASM) || defined(POLARIS_ENGINE_TARGET_POSIX) || defined(USE_QT)
#define is_opengl_byte_order()	true
#else
#define is_opengl_byte_order()	false
#endif

/*************
 * Rendering *
 *************/

/*
 * Renders an image to the screen with the "normal" shader pipeline.
 *  - The "normal" shader pipeline renders pixels with alpha blending
 */
void
render_image_normal(
	int dst_left,			/* The X coordinate of the screen */
	int dst_top,			/* The Y coordinate of the screen */
	int dst_width,			/* The width of the destination rectangle */
	int dst_height,			/* The width of the destination rectangle */
	struct image *src_image,	/* [IN] The image to be rendered */
	int src_left,			/* The X coordinate of a source image */
	int src_top,			/* The Y coordinate of a source image */
	int src_width,			/* The width of the source rectangle */
	int src_height,			/* The height of the source rectangle */
	int alpha);			/* The alpha value (0 to 255) */

/*
 * Renders an image to the screen with the "normal" shader pipeline.
 *  - The "normal" shader pipeline renders pixels with alpha blending
 */
void
render_image_add(
	int dst_left,			/* The X coordinate of the screen */
	int dst_top,			/* The Y coordinate of the screen */
	int dst_width,			/* The width of the destination rectangle */
	int dst_height,			/* The width of the destination rectangle */
	struct image *src_image,	/* [IN] The image to be rendered */
	int src_left,			/* The X coordinate of a source image */
	int src_top,			/* The Y coordinate of a source image */
	int src_width,			/* The width of the source rectangle */
	int src_height,			/* The height of the source rectangle */
	int alpha);			/* The alpha value (0 to 255) */

/*
 * Renders an image to the screen with the "dim" shader pipeline.
 *  - The "dim" shader pipeline renders pixels at 50% values for each RGB component
 */
void
render_image_dim(
	int dst_left,			/* The X coordinate of the screen */
	int dst_top,			/* The Y coordinate of the screen */
	int dst_width,			/* The width of the destination rectangle */
	int dst_height,			/* The width of the destination rectangle */
	struct image *src_image,	/* [IN] The image to be rendered */
	int src_left,			/* The X coordinate of a source image */
	int src_top,			/* The Y coordinate of a source image */
	int src_width,			/* The width of the source rectangle */
	int src_height,			/* The height of the source rectangle */
	int alpha);			/* The alpha value (0 to 255) */

/*
 * Renders an image to the screen with the "rule" shader pipeline.
 *  - The "rule" shader pipeline is a variation of "universal transition" with a threshold value
 *  - A rule image must have the same size as the screen
 */
void
render_image_rule(
	struct image *src_img,		/* [IN] The source image */
	struct image *rule_img,		/* [IN] The rule image */
	int threshold);			/* The threshold (0 to 255) */

/*
 * Renders an image to the screen with the "melt" shader pipeline.
 *  - The "melt" shader pipeline is a variation of "universal transition" with a progress value
 *  - A rule image must have the same size as the screen
 */
void render_image_melt(
	struct image *src_img,		/* [IN] The source image */
	struct image *rule_img,		/* [IN] The rule image */
	int progress);			/* The progress (0 to 255) */

/*
 * Renders an image to the screen as a triangle strip with the "normal" shader pipeline.
 */
void
render_image_3d_normal(
	float x1,
	float y1,
	float x2,
	float y2,
	float x3,
	float y3,
	float x4,
	float y4,
	struct image *src_image,
	int src_left,
	int src_top,
	int src_width,
	int src_height,
	int alpha);

/*
 * Renders an image to the screen as a triangle strip with the "add" shader pipeline.
 */
void
render_image_3d_add(
	float x1,
	float y1,
	float x2,
	float y2,
	float x3,
	float y3,
	float x4,
	float y4,
	struct image *src_image,
	int src_left,
	int src_top,
	int src_width,
	int src_height,
	int alpha);

/*************
 * Lap Timer *
 *************/

/*
 * Resets a lap timer and initializes it with a current time.
 */
void reset_lap_timer(uint64_t *origin);

/*
 * Gets a lap time in milliseconds.
 */
uint64_t get_lap_timer_millisec(uint64_t *origin);

/******************
 * Sound Playback *
 ******************/

/*
 * Note: we have the following sound streams:
 *  - BGM_STREAM:   background music
 *  - SE_STREAM:    sound effect
 *  - VOICE_STREAM: character voice
 *  - SYS_STREAM:   system sound
 */

/*
 * Starts playing a sound file on a track.
 */
bool
play_sound(int stream,		/* A sound stream index */
	   struct wave *w);	/* [IN] A sound object, ownership will be delegated to the callee */

/*
 * Stops playing a sound track.
 */
bool stop_sound(int stream);

/*
 * Sets sound volume.
 */
bool set_sound_volume(int stream, float vol);

/*
 * Returns whether a sound playback for a stream is already finished.
 *  - This function exists to detect voice playback finish
 */
bool is_sound_finished(int stream);

/******************
 * Video Playback *
 ******************/

/*
 * Starts playing a video file.
 */
bool play_video(const char *fname,	/* file name */
		bool is_skippable);	/* allow skip for a unseen video */

/*
 * Stops playing music stream.
 */
void stop_video(void);

/*
 * Returns whether a video playcack is running.
 */
bool is_video_playing(void);

/***********************
 * Window Manipulation *
 ***********************/

/*
 * Updates the window title by configs and the chapter name.
 */
void update_window_title(void);

/*
 * Returns whether the current HAL supports the "full screen mode".
 *  - The "full screen mode" includes docking of some game consoles
 *  - A HAL can implement the "full screen mode" but it is optional
 */
bool is_full_screen_supported(void);

/*
 * Returns whether the current HAL is in the "full screen mode".
 */
bool is_full_screen_mode(void);

/*
 * Enters the full screen mode.
 *  - A HAL can ignore this call
 */
void enter_full_screen_mode(void);

/*
 * Leaves the full screen mode.
 *  - A HAL can ignore this call
 */
void leave_full_screen_mode(void);

/**********
 * Locale *
 **********/

/*
 * Gets the system language.
 *  - Return value can be:
 *    - "ja": Japanese
 *    - "en": English
 *    - "zh": Simplified Chinese
 *    - "tw": Traditional Chinese
 *    - "fr": French
 *    - "it": Italian
 *    - "es": Spanish (Castellano)
 *    - "de": Deutsch
 *    - "el": Greek
 *    - "ru": Russian
 *    - "other": Other (must fallback to English)
 *  - To add a language, make sure to change the following:
 *    - "enum language_code" in conf.h
 *    - init_locale_code() in conf.c
 *    - set_locale_mapping() in conf.c
 *    - get_ui_message() in uimsg.c
 *  - Note that:
 *    - Currently we don't have a support for right-to-left writing systems
 *      - e.g. Hebrew, Arabic
 *      - It should be implemented in draw_msg_common() in glyph.c
 *      - It can be easily implemented but the author cannot read those characters and need helps
 *    - Glyphs that are composed from multiple Unicode codepoints are not supported
 *      - Currently we need pre-composed texts
 */
const char *get_system_locale(void);

/******************
 * Text-To-Speech *
 ******************/

/*
 * Speaks a text.
 *  - Specifying NULL stops speaking.
 */
void speak_text(const char *text);

/****************
 * Touch Screen *
 ****************/

/*
 * Enable/disable message skip by touch move.
 */
#if defined(POLARIS_ENGINE_TARGET_IOS) || defined(POLARIS_ENGINE_TARGET_ANDROID) || defined(POLARIS_ENGINE_TARGET_WASM)
void set_continuous_swipe_enabled(bool is_enabled);
#else
static inline void set_continuous_swipe_enabled(bool is_enabled) { UNUSED_PARAMETER(is_enabled); }
#endif

/*
 * For Foreign Languages (Swift and C#)
 */
#ifdef POLARIS_ENGINE_DLL

#ifndef NO_CDECL
#define POLARISENGINEAPI __cdecl
#else
#define POLARISENGINEAPI
#endif

extern void init_hal_func_table(
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
	bool POLARISENGINEAPI (*p_check_file_exist)(intptr_t file_name),
	intptr_t POLARISENGINEAPI (*p_get_file_contents)(intptr_t file_name, intptr_t len),
	void POLARISENGINEAPI (*p_open_save_file)(intptr_t file_name),
	void POLARISENGINEAPI (*p_write_save_file)(int b),
	void POLARISENGINEAPI (*p_close_save_file)(void));
extern void POLARISENGINEAPI (*wrap_log_info)(intptr_t s);
extern void POLARISENGINEAPI (*wrap_log_warn)(intptr_t s);
extern void POLARISENGINEAPI (*wrap_log_error)(intptr_t s);
extern void POLARISENGINEAPI (*wrap_make_sav_dir)(void);
extern void POLARISENGINEAPI (*wrap_make_valid_path)(intptr_t dir, intptr_t fname, intptr_t dst, int len);
extern void POLARISENGINEAPI (*wrap_notify_image_update)(int id, int width, int height, intptr_t pixels);
extern void POLARISENGINEAPI (*wrap_notify_image_free)(int id);
extern void POLARISENGINEAPI (*wrap_render_image_normal)(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
extern void POLARISENGINEAPI (*wrap_render_image_add)(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
extern void POLARISENGINEAPI (*wrap_render_image_dim)(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
extern void POLARISENGINEAPI (*wrap_render_image_rule)(int src_img, int rule_img, int threshold);
extern void POLARISENGINEAPI (*wrap_render_image_melt)(int src_img, int rule_img, int progress);
extern void POLARISENGINEAPI (*wrap_render_image_3d_normal)(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
extern void POLARISENGINEAPI (*wrap_render_image_3d_add)(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
extern void POLARISENGINEAPI (*wrap_reset_lap_timer)(intptr_t origin);
extern int64_t POLARISENGINEAPI (*wrap_get_lap_timer_millisec)(intptr_t origin);
extern void POLARISENGINEAPI (*wrap_play_sound)(int stream, intptr_t wave);
extern void POLARISENGINEAPI (*wrap_stop_sound)(int stream);
extern void POLARISENGINEAPI (*wrap_set_sound_volume)(int stream, float vol);
extern bool POLARISENGINEAPI (*wrap_is_sound_finished)(int stream);
extern bool POLARISENGINEAPI (*wrap_play_video)(intptr_t fname, bool is_skippable);
extern void POLARISENGINEAPI (*wrap_stop_video)(void);
extern bool POLARISENGINEAPI (*wrap_is_video_playing)(void);
extern void POLARISENGINEAPI (*wrap_update_window_title)(void);
extern bool POLARISENGINEAPI (*wrap_is_full_screen_supported)(void);
extern bool POLARISENGINEAPI (*wrap_is_full_screen_mode)(void);
extern void POLARISENGINEAPI (*wrap_enter_full_screen_mode)(void);
extern void POLARISENGINEAPI (*wrap_leave_full_screen_mode)(void);
extern void POLARISENGINEAPI (*wrap_get_system_locale)(intptr_t dst, int len);
extern void POLARISENGINEAPI (*wrap_speak_text)(intptr_t text);
extern void POLARISENGINEAPI (*wrap_set_continuous_swipe_enabled)(bool is_enabled);

#endif /* POLARIS_ENGINE_DLL */

#endif /* POLARIS_ENGINE_HAL_H */
