/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 8; c-basic-offset: 8; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * Glyph rendering and text layout subsystem
 */

#ifndef XENGINE_GLYPH_H
#define XENGINE_GLYPH_H

#include "image.h"

/* Unicodeコードポイント */
#define CHAR_TOUTEN		(0x3001)
#define CHAR_KUTEN		(0x3002)
#define CHAR_YENSIGN		(0x00a5)

/* フォントタイプ */
#define FONT_GLOBAL		(0)
#define FONT_MAIN		(1)
#define FONT_ALT1		(2)
#define FONT_ALT2		(3)
#define FONT_COUNT		(4)

/* フォントレンダラの初期化処理を行う */
bool init_glyph(void);

/* フォントレンダラの終了処理を行う */
void cleanup_glyph(void);

/* グローバルフォントを変更する */
bool update_global_font(void);

/* utf-8文字列の先頭文字をutf-32文字に変換する */
int utf8_to_utf32(const char *mbs, uint32_t *wc);

/* utf-8文字列の文字数を返す */
int count_utf8_chars(const char *mbs);

/* 文字を描画した際の幅を取得する */
int get_glyph_width(int font_type, int font_size, uint32_t codepoint);

/* 文字を描画した際の高さを取得する */
int get_glyph_height(int font_type, int font_size, uint32_t codepoint);

/* 文字列を描画した際の幅を取得する */
int get_string_width(int font_type, int font_size, const char *mbs);

/* 文字列を描画した際の高さを取得する */
int get_string_height(int font_type, int font_size, const char *mbs);

/* 文字の描画を行う */
bool draw_glyph(struct image *img,
		int font_type,
		int font_size,
		int base_font_size,
		bool use_outline,
		int outline_width,
		int x,
		int y,
		pixel_t color,
		pixel_t outline_color,
		uint32_t codepoint,
		int *ret_w,
		int *ret_h,
		bool is_dim);

/*
 * Message drawing
 */

/*
 * A context for character drawing to an image.
 */
struct draw_msg_context {
/* private: */
	/* Will be copied in the constructor. */
	int stage_layer;
	const char *msg;	/* Updated on a draw. */
	int font;
	int font_size;
	int base_font_size;
	int ruby_size;
	bool use_outline;
	int outline_width;
	int pen_x;		/* Updated on a draw. */
	int pen_y;		/* Updated on a draw. */
	int area_width;
	int area_height;
	int left_margin;
	int right_margin;
	int top_margin;
	int bottom_margin;
	int line_margin;
	int char_margin;
	pixel_t color;
	pixel_t outline_color;
	pixel_t bg_color;
	bool is_dimming;
	bool ignore_linefeed;
	bool ignore_font;
	bool ignore_outline;
	bool ignore_color;
	bool ignore_size;
	bool ignore_position;
	bool ignore_ruby;
	bool ignore_wait;
	bool fill_bg;
	void (*inline_wait_hook)(float);
	bool use_tategaki;

	/* Internal: updated on a draw. */
	struct image *layer_image;
	bool runtime_is_after_space;
	bool runtime_is_inline_wait;
	int runtime_ruby_x;
	int runtime_ruby_y;
	bool runtime_is_line_top;
	bool runtime_is_gyoto_kinsoku;
	bool runtime_is_gyoto_kinsoku_second;
	bool is_quoted;
};

/*
 * Initialize a message drawing context.
 *  - Too many parameters, but I think it's useful to detect bugs as compile
 *    errors when we add a change on the design for now.
 */
void construct_draw_msg_context(
	struct draw_msg_context *context,
	int stage_layer,
	const char *msg,
	int font,
	int font_size,
	int base_font_size,
	int ruby_size,
	bool use_outline,
	int outline_width,
	int pen_x,
	int pen_y,
	int area_width,
	int area_height,
	int left_margin,
	int right_margin,
	int top_margin,
	int bottom_margin,
	int line_margin,
	int char_margin,
	pixel_t color,
	pixel_t outline_color,
	bool is_dimming,
	bool ignore_linefeed,
	bool ignore_font,
	bool ignore_outline,
	bool ignore_color,
	bool ignore_size,
	bool ignore_position,
	bool ignore_ruby,
	bool ignore_wait,
	bool fill_bg,
	void (*inline_wait_hook)(float),
	bool use_tategaki);

/* Set an alternative target image. */
void set_alternative_target_image(struct draw_msg_context *context,
				  struct image *img);

/* Count remaining characters excluding escape sequences. */
int count_chars_common(struct draw_msg_context *context, int *width);

/* Draw characters in a message up to (max_chars) characters. */
int draw_msg_common(struct draw_msg_context *context, int max_chars);

/* Get a pen position. */
void get_pen_position_common(struct draw_msg_context *context, int *pen_x,
			     int *pen_y);

/* Set ignore inline wait. */
void set_ignore_inline_wait(struct draw_msg_context *context);

/* Check if c is an escape sequence character. */
bool is_escape_sequence_char(char c);

#endif
