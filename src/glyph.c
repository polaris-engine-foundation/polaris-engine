/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 8; c-basic-offset: 8; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * Glyph rendering and text layout subsystem
 */

#include "polarisengine.h"

/*
 * FreeType2 headers
 */
#include <ft2build.h>

#include FT_FREETYPE_H
#ifdef POLARIS_ENGINE_TARGET_WASM
#include <ftstroke.h>
#else
#include <freetype/ftstroke.h>
#endif

/*
 * The scale constant
 */
#define SCALE	(64)

/*
 * FreeType2 objects
 */
static FT_Library library;
static FT_Face face[FONT_COUNT];
static FT_Byte *font_file_content[FONT_COUNT];
static FT_Long font_file_size[FONT_COUNT];

/*
 * Emoticon images
 */
static struct image *emoticon_image[EMOTICON_COUNT];

/*
 * Forward declarations
 */
static bool read_font_file_content(
	const char *file_name,
	FT_Byte **content,
	FT_Long *size);
static bool draw_glyph_without_outline(
	struct image *img,
	int font_type,
	int font_size,
	int base_font_size,
	int x,
	int y,
	pixel_t color,
	uint32_t codepoint,
	int *ret_w,
	int *ret_h,
	bool is_dim);
static void draw_glyph_func(
	unsigned char * RESTRICT font,
	int font_width,
	int font_height,
	int margin_left,
	int margin_top,
	pixel_t * RESTRICT image,
	int image_width,
	int image_height,
	int image_x,
	int image_y,
	pixel_t color);
static void draw_glyph_dim_func(
	unsigned char * RESTRICT font,
	int font_width,
	int font_height,
	int margin_left,
	int margin_top,
	pixel_t * RESTRICT image,
	int image_width,
	int image_height,
	int image_x,
	int image_y,
	pixel_t color);
static bool isgraph_extended(const char **mbs, uint32_t *wc);
static int translate_font_type(int font_ype);
static bool apply_font_size(int font_type, int size);
static bool draw_emoticon(struct draw_msg_context *context, const char *name, int *w, int *h);

/*
 * フォントレンダラの初期化処理を行う
 */
bool init_glyph(void)
{
	const char *fname[FONT_COUNT];
	FT_Error err;
	int i;

#ifdef POLARIS_ENGINE_DLL
	/* DLLが再利用されたときのために初期化する */
	cleanup_glyph();
#endif

	/* FreeType2ライブラリを初期化する */
	err = FT_Init_FreeType(&library);
	if (err != 0) {
		log_api_error("FT_Init_FreeType");
		return false;
	}

	/* コンフィグを読み込む */
	fname[FONT_GLOBAL] = conf_font_global_file;
	fname[FONT_MAIN] = conf_font_main_file;
	fname[FONT_ALT1] = conf_font_alt1_file;
	fname[FONT_ALT2] = conf_font_alt2_file;

	/* フォントを読み込む */
	for (i = 0; i < FONT_COUNT; i++) {
		if (fname[i] == NULL)
			continue;

		/* フォントファイルの内容を読み込む */
		if (!read_font_file_content(fname[i],
					    &font_file_content[i],
					    &font_file_size[i]))
			return false;

		/* フォントファイルを読み込む */
		err = FT_New_Memory_Face(library,
					 font_file_content[i],
					 font_file_size[i],
					 0,
					 &face[i]);
		if (err != 0) {
			log_font_file_error(conf_font_global_file);
			return false;
		}
	}

	/* フォントのプリロードを行う */
	for (i = 0; i < FONT_COUNT; i++) 
		get_glyph_width(i, conf_font_size, 'A');

	/* エモーティコンのロードを行う */
	for (i = 0; i < EMOTICON_COUNT; i++) {
		if (conf_emoticon_name[i] != NULL && conf_emoticon_file[i] != NULL) {
			emoticon_image[i] = create_image_from_file(CG_DIR, conf_emoticon_file[i]);
			if (emoticon_image[i] == NULL)
				return false;
		}
	}

	return true;
}

/*
 * フォントレンダラの終了処理を行う
 */
void cleanup_glyph(void)
{
	int i;

	for (i = 0; i < FONT_COUNT; i++) {
		if (face[i] != NULL) {
			FT_Done_Face(face[i]);
			face[i] = NULL;
		}
		if (font_file_content[i] != NULL) {
			free(font_file_content[i]);
			font_file_content[i] = NULL;
		}
	}

	if (library != NULL) {
		FT_Done_FreeType(library);
		library = NULL;
	}

	for (i = 0; i < EMOTICON_COUNT; i++) {
		if (emoticon_image[i] != NULL) {
			destroy_image(emoticon_image[i]);
			emoticon_image[i] = NULL;
		}
	}
}

/*
 * グローバルフォントを変更する
 */
bool update_global_font(void)
{
	FT_Error err;

	assert(conf_font_global_file != NULL);

	/* Return if before init. */
	if (face[FONT_GLOBAL] == NULL)
		return true;

	/* Cleanup the current global font. */
	assert(face[FONT_GLOBAL] != NULL);
	FT_Done_Face(face[FONT_GLOBAL]);
	face[FONT_GLOBAL] = NULL;
	assert(font_file_content[FONT_GLOBAL] != NULL);
	free(font_file_content[FONT_GLOBAL]);
	font_file_content[FONT_GLOBAL] = NULL;

	/* フォントファイルの内容を読み込む */
	if (!read_font_file_content(conf_font_global_file,
				    &font_file_content[FONT_GLOBAL],
				    &font_file_size[FONT_GLOBAL]))
		return false;

	/* フォントファイルを読み込む */
	err = FT_New_Memory_Face(library,
				 font_file_content[FONT_GLOBAL],
				 font_file_size[FONT_GLOBAL],
				 0,
				 &face[FONT_GLOBAL]);
	if (err != 0) {
		log_font_file_error(conf_font_global_file);
		return false;
	}

	return true;
}

/* フォントファイルの内容を読み込む */
static bool read_font_file_content(const char *file_name,
				   FT_Byte **content,
				   FT_Long *size)
{
	struct rfile *rf;

	/* フォントファイルを開く */
	rf = open_rfile(FONT_DIR, file_name, false);
	if (rf == NULL)
		return false;

	/* フォントファイルのサイズを取得する */
	*size = (FT_Long)get_rfile_size(rf);
	if (*size == 0) {
		log_font_file_error(file_name);
		close_rfile(rf);
		return false;
	}

	/* メモリを確保する */
	*content = malloc((size_t)*size);
	if (*content == NULL) {
		log_memory();
		close_rfile(rf);
		return false;
	}

	/* ファイルの内容を読み込む */
	if (read_rfile(rf, *content, (size_t)*size) != (size_t)*size) {
		log_font_file_error(file_name);
		close_rfile(rf);
		return false;
	}
	close_rfile(rf);

	return true;
}

/*
 * utf-8文字列の先頭文字をutf-32文字に変換する
 * XXX: サロゲートペア、合字は処理しない
 */
int utf8_to_utf32(const char *mbs, uint32_t *wc)
{
	size_t mbslen, octets, i;
	uint32_t ret;

	assert(mbs != NULL);

	/* 変換元がNULLか長さが0の場合 */
	mbslen = strlen(mbs);
	if(mbslen == 0)
		return 0;

	/* 1バイト目をチェックしてオクテット数を求める */
	if (mbs[0] == '\0')
		octets = 0;
	else if ((mbs[0] & 0x80) == 0)
		octets = 1;
	else if ((mbs[0] & 0xe0) == 0xc0)
		octets = 2;
	else if ((mbs[0] & 0xf0) == 0xe0)
		octets = 3;
	else if ((mbs[0] & 0xf8) == 0xf0)
		octets = 4;
	else
		return -1;	/* 解釈できない */

	/* sの長さをチェックする */
	if (mbslen < octets)
		return -1;	/* mbsの長さが足りない */

	/* 2-4バイト目をチェックする */
	for (i = 1; i < octets; i++) {
		if((mbs[i] & 0xc0) != 0x80)
			return -1;	/* 解釈できないバイトである */
	}

	/* 各バイトを合成してUTF-32文字を求める */
	switch (octets) {
	case 0:
		ret = 0;
		break;
	case 1:
		ret = (uint32_t)mbs[0];
		break;
	case 2:
		ret = (uint32_t)(((mbs[0] & 0x1f) << 6) |
				 (mbs[1] & 0x3f));
		break;
	case 3:
		ret = (uint32_t)(((mbs[0] & 0x0f) << 12) |
				 ((mbs[1] & 0x3f) << 6) |
				 (mbs[2] & 0x3f));
		break;
	case 4:
		ret = (uint32_t)(((mbs[0] & 0x07) << 18) |
				 ((mbs[1] & 0x3f) << 12) |
				 ((mbs[2] & 0x3f) << 6) |
				 (mbs[3] & 0x3f));
		break;
	default:
		/* never come here */
		assert(0);
		return -1;
	}

	/* 結果を格納する */
	if(wc != NULL)
		*wc = ret;

	/* 消費したオクテット数を返す */
	return (int)octets;
}

/*
 * utf-8文字列のワイド文字数を返す
 */
int count_utf8_chars(const char *mbs)
{
	int count;
	int mblen;

	count = 0;
	while (*mbs != '\0') {
		mblen = utf8_to_utf32(mbs, NULL);
		if (mblen == -1)
			return -1;
		count++;
		mbs += mblen;
	}
	return count;
}

/*
 * 文字を描画した際の幅を取得する
 */
int get_glyph_width(int font_type, int font_size, uint32_t codepoint)
{
	int w, h;

	w = h = 0;

	/* 幅を求める */
	draw_glyph(NULL,
		   font_type,
		   font_size,
		   font_size,
		   false,
		   0,
		   0,
		   0,
		   0,
		   0,
		   codepoint, &w, &h, false);

	return w;
}

/*
 * 文字を描画した際の高さを取得する
 */
int get_glyph_height(int font_type, int font_size, uint32_t codepoint)
{
	int w, h;

	w = h = 0;

	/* 幅を求める */
	draw_glyph(NULL,
		   font_type,
		   font_size,
		   font_size,
		   false,
		   0,
		   0,
		   0,
		   0,
		   0,
		   codepoint,
		   &w,
		   &h,
		   false);

	return h;
}

/*
 * 文字列を描画した際の幅を取得する
 */
int get_string_width(int font_type, int font_size, const char *mbs)
{
	uint32_t c;
	int mblen, w;

	/* 1文字ずつ描画する */
	w = 0;
	c = 0; /* warning avoidance on gcc 5.3.1 */
	while (*mbs != '\0') {
		/* エスケープシーケンスをスキップする */
		while (*mbs == '\\') {
			if (*(mbs + 1) == 'n') {
				mbs += 2;
				continue;
			}
			while (*mbs != '\0' && *mbs != '}')
				mbs++;
			mbs++;
		}

		/* 文字を取得する */
		mblen = utf8_to_utf32(mbs, &c);
		if (mblen == -1)
			return -1;

		/* 幅を取得する */
		w += get_glyph_width(font_type, font_size, c);

		/* 次の文字へ移動する */
		mbs += mblen;
	}
	return w;
}

/*
 * 文字列を描画した際の高さを取得する
 */
int get_string_height(int font_type, int font_size, const char *mbs)
{
	uint32_t c;
	int mblen, w;

	/* 1文字ずつ描画する */
	w = 0;
	c = 0; /* warning avoidance on gcc 5.3.1 */
	while (*mbs != '\0') {
		/* エスケープシーケンスをスキップする */
		while (*mbs == '\\') {
			if (*(mbs + 1) == 'n') {
				mbs += 2;
				continue;
			}
			while (*mbs != '\0' && *mbs != '}')
				mbs++;
			mbs++;
		}

		/* 文字を取得する */
		mblen = utf8_to_utf32(mbs, &c);
		if (mblen == -1)
			return -1;

		/* 高さを取得する */
		w += get_glyph_height(font_type, font_size, c);

		/* 次の文字へ移動する */
		mbs += mblen;
	}
	return w;
}

/*
 * 文字の描画を行う
 */

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
		bool is_dim)
{
	FT_Stroker stroker;
	FT_UInt glyphIndex;
	FT_Glyph glyph;
	FT_BitmapGlyph bitmapGlyph;
	int descent;

	if (!use_outline) {
		return draw_glyph_without_outline(img,
						  font_type,
						  font_size,
						  base_font_size,
						  x,
						  y,
						  color,
						  codepoint,
						  ret_w,
						  ret_h,
						  is_dim);
	}
	font_type = translate_font_type(font_type);
	apply_font_size(font_type, font_size);

	/* アウトライン(内側)を描画する */
	FT_Stroker_New(library, &stroker);
	FT_Stroker_Set(stroker, outline_width * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	glyphIndex = FT_Get_Char_Index(face[font_type], codepoint);
	FT_Load_Glyph(face[font_type], glyphIndex, FT_LOAD_DEFAULT);
	FT_Get_Glyph(face[font_type]->glyph, &glyph);
	FT_Glyph_StrokeBorder(&glyph, stroker, true, true);
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
	bitmapGlyph = (FT_BitmapGlyph)glyph;
	if (img != NULL) {
		draw_glyph_func(bitmapGlyph->bitmap.buffer,
				(int)bitmapGlyph->bitmap.width,
				(int)bitmapGlyph->bitmap.rows,
				bitmapGlyph->left,
				font_size - bitmapGlyph->top,
				img->pixels,
				img->width,
				img->height,
				x,
				y - (font_size - base_font_size),
				outline_color);
	}
	FT_Done_Glyph(glyph);
	FT_Stroker_Done(stroker);

	/* アウトライン(外側)を描画する */
	FT_Stroker_New(library, &stroker);
	FT_Stroker_Set(stroker, outline_width * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	glyphIndex = FT_Get_Char_Index(face[font_type], codepoint);
	FT_Load_Glyph(face[font_type], glyphIndex, FT_LOAD_DEFAULT);
	FT_Get_Glyph(face[font_type]->glyph, &glyph);
	FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
	bitmapGlyph = (FT_BitmapGlyph)glyph;
	if (img != NULL) {
		draw_glyph_func(bitmapGlyph->bitmap.buffer,
				(int)bitmapGlyph->bitmap.width,
				(int)bitmapGlyph->bitmap.rows,
				bitmapGlyph->left,
				font_size - bitmapGlyph->top,
				img->pixels,
				img->width,
				img->height,
				x,
				y - (font_size - base_font_size),
				outline_color);
	}
	descent = (int)(face[font_type]->glyph->metrics.height / SCALE) -
		  (int)(face[font_type]->glyph->metrics.horiBearingY / SCALE);
	*ret_w = (int)face[font_type]->glyph->advance.x / SCALE;
	*ret_h = font_size + descent + 2;
	FT_Done_Glyph(glyph);
	FT_Stroker_Done(stroker);
	if (img == NULL)
		return true;

	/* 中身を描画する */
	glyphIndex = FT_Get_Char_Index(face[font_type], codepoint);
	FT_Load_Glyph(face[font_type], glyphIndex, FT_LOAD_DEFAULT);
	FT_Get_Glyph(face[font_type]->glyph, &glyph);
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
	bitmapGlyph = (FT_BitmapGlyph)glyph;
	draw_glyph_func(bitmapGlyph->bitmap.buffer,
			(int)bitmapGlyph->bitmap.width,
			(int)bitmapGlyph->bitmap.rows,
			bitmapGlyph->left,
			font_size - bitmapGlyph->top,
			img->pixels,
			img->width,
			img->height,
			x,
			y - (font_size - base_font_size),
			color);
	FT_Done_Glyph(glyph);

	notify_image_update(img);

	/* 成功 */
	return true;
}

static bool draw_glyph_without_outline(struct image *img,
				       int font_type,
				       int font_size,
				       int base_font_size,
				       int x,
				       int y,
				       pixel_t color,
				       uint32_t codepoint,
				       int *ret_w,
				       int *ret_h,
				       bool is_dim)
{
	FT_Error err;
	int descent;

	font_type = translate_font_type(font_type);
	apply_font_size(font_type, font_size);

	/* 文字をグレースケールビットマップとして取得する */
	err = FT_Load_Char(face[font_type], codepoint, FT_LOAD_RENDER);
	if (err != 0) {
		log_api_error("FT_Load_Char");
		return false;
	}

	/* 文字のビットマップを対象イメージに描画する */
	if (img != NULL) {
		if (!is_dim) {
			draw_glyph_func(face[font_type]->glyph->bitmap.buffer,
					(int)face[font_type]->glyph->bitmap.width,
					(int)face[font_type]->glyph->bitmap.rows,
					face[font_type]->glyph->bitmap_left,
					font_size - face[font_type]->glyph->bitmap_top,
					img->pixels,
					img->width,
					img->height,
					x,
					y - (font_size - base_font_size),
					color);
		} else {
			draw_glyph_dim_func(face[font_type]->glyph->bitmap.buffer,
					    (int)face[font_type]->glyph->bitmap.width,
					    (int)face[font_type]->glyph->bitmap.rows,
					    face[font_type]->glyph->bitmap_left,
					    font_size - face[font_type]->glyph->bitmap_top,
					    img->pixels,
					    img->width,
					    img->height,
					    x,
					    y - (font_size - base_font_size),
					    color);
		}
	}

	/* descentを求める */
	descent = (int)(face[font_type]->glyph->metrics.height / SCALE) -
		  (int)(face[font_type]->glyph->metrics.horiBearingY / SCALE);

	/* 描画した幅と高さを求める */
	*ret_w = (int)face[font_type]->glyph->advance.x / SCALE;
	*ret_h = font_size + descent;

	if (img != NULL)
		notify_image_update(img);

	return true;
}

/* サポートされているアルファベットか調べる */
static bool isgraph_extended(const char **mbs, uint32_t *wc)
{
	int len;

	/* 英語のアルファベットと記号の場合 */
	if (isgraph((unsigned char)**mbs)) {
		*wc = (unsigned char)**mbs;
		(*mbs)++;
		return true;
	}

	/* Unicode文字を取得する */
	len = utf8_to_utf32(*mbs, wc);
	if (len < 1)
		return false;
	*mbs += len;

	/* アクセント付きラテンアルファベットの場合 */
	if (*wc >= 0x00c0 && *wc <= 0x017f)
		return true;

	/* ギリシャ語の場合 */
	if (*wc >= 0x0370 && *wc <= 0x3ff)
		return true;

	/* ロシア語の場合 */
	if (*wc >= 0x410 && *wc <= 0x44f)
		return true;

	/* 他の言語 */
	return false;
}

/* フォントサイズを指定する */
static bool apply_font_size(int font_type, int size)
{
	FT_Error err;

	font_type = translate_font_type(font_type);

	/* 文字サイズをセットする */
	err = FT_Set_Pixel_Sizes(face[font_type], 0, (FT_UInt)size);
	if (err != 0) {
		log_api_error("FT_Set_Pixel_Sizes");
		return false;
	}
	return true;
}

/* フォントを選択する */
static int translate_font_type(int font_type)
{
	assert(font_type == FONT_GLOBAL || font_type == FONT_MAIN ||
	       font_type == FONT_ALT1 || font_type == FONT_ALT2);

	if (font_type == FONT_GLOBAL)
		return FONT_GLOBAL;
	if (font_type == FONT_MAIN) {
		if (conf_font_main_file == NULL)
			return FONT_GLOBAL;
		else
			return FONT_MAIN;
	}
	if (font_type == FONT_ALT1) {
		if (conf_font_alt1_file == NULL)
			return FONT_GLOBAL;
		else
			return FONT_ALT1;
	}
	if (font_type == FONT_ALT2) {
		if (conf_font_alt2_file == NULL)
			return FONT_GLOBAL;
		else
			return FONT_ALT2;
	}
	assert(0);
	return FONT_GLOBAL;
}

/*
 * フォントをイメージに描画する
 */
static void draw_glyph_func(unsigned char *font,
			    int font_width,
			    int font_height,
			    int margin_left,
			    int margin_top,
			    pixel_t * RESTRICT image,
			    int image_width,
			    int image_height,
			    int image_x,
			    int image_y,
			    pixel_t color)
{
	unsigned char *src_ptr, src_pix;
	pixel_t *dst_ptr, dst_pix, dst_a2;
	float color_r, color_g, color_b;
	float src_a, src_r, src_g, src_b;
	float dst_a, dst_r, dst_g, dst_b;
	int image_real_x, image_real_y;
	int font_real_x, font_real_y;
	int font_real_width, font_real_height;
	int px, py;

	/* 完全に描画しない場合のクリッピングを行う */
	if (image_x + margin_left + font_width < 0)
		return;
	if (image_x + margin_left >= image_width)
		return;
	if (image_y + margin_top + font_height < 0)
		return;
	if (image_y + margin_top > image_height)
		return;

	/* 部分的に描画しない場合のクリッピングを行う */
	image_real_x = image_x + margin_left;
	image_real_y = image_y + margin_top;
	font_real_x = 0;
	font_real_y = 0;
	font_real_width = font_width;
	font_real_height = font_height;
	if (image_real_x < 0) {
		font_real_x -= image_real_x;
		font_real_width += image_real_x;
		image_real_x = 0;
	}
	if (image_real_x + font_real_width >= image_width) {
		font_real_width -= (image_real_x + font_real_width) -
				   image_width;
	}
	if (image_real_y < 0) {
		font_real_y -= image_real_y;
		font_real_height += image_real_y;
		image_real_y = 0;
	}
	if (image_real_y + font_real_height >= image_height) {
		font_real_height -= (image_real_y + font_real_height) -
				    image_height;
	}

	/* 描画する */
	color_r = (float)get_pixel_r(color);
	color_g = (float)get_pixel_g(color);
	color_b = (float)get_pixel_b(color);
	dst_ptr = image + image_real_y * image_width + image_real_x;
	src_ptr = font + font_real_y * font_width + font_real_x;
	for (py = font_real_y; py < font_real_y + font_real_height; py++) {
		for (px = font_real_x; px < font_real_x + font_real_width; px++) {
			/* 文字のピクセルの値を取得する */
			src_pix = *src_ptr++;

			/* 色にピクセルの値(アルファ値)を乗算する */
			src_a = (float)src_pix / 255.0f;
			dst_a = 1.0f - src_a;
			src_r = src_a * color_r;
			src_g = src_a * color_g;
			src_b = src_a * color_b;

			/* 転送先画像のピクセルの値を取得する */
			dst_pix	= *dst_ptr;
			dst_r  = dst_a * (float)get_pixel_r(dst_pix);
			dst_g  = dst_a * (float)get_pixel_g(dst_pix);
			dst_b  = dst_a * (float)get_pixel_b(dst_pix);
			dst_a2 = src_pix + get_pixel_a(dst_pix);
			if (dst_a2 > 255)
				dst_a2 = 255;

			/* 合成して転送先に格納する */
			*dst_ptr++ = make_pixel((uint32_t)dst_a2,
						(uint32_t)(src_r + dst_r),
						(uint32_t)(src_g + dst_g),
						(uint32_t)(src_b + dst_b));
		}
		dst_ptr += image_width - font_real_width;
		src_ptr += font_width - font_real_width;
	}
}

/*
 * フォントをイメージに描画する(dimmingでの上書き用)
 */
static void draw_glyph_dim_func(unsigned char *font,
				int font_width,
				int font_height,
				int margin_left,
				int margin_top,
				pixel_t * RESTRICT image,
				int image_width,
				int image_height,
				int image_x,
				int image_y,
				pixel_t color)
{
	unsigned char *src_ptr;
	pixel_t *dst_ptr;
	int image_real_x, image_real_y;
	int font_real_x, font_real_y;
	int font_real_width, font_real_height;
	int px, py;

	/* 完全に描画しない場合のクリッピングを行う */
	if (image_x + margin_left + font_width < 0)
		return;
	if (image_x + margin_left >= image_width)
		return;
	if (image_y + margin_top + font_height < 0)
		return;
	if (image_y + margin_top > image_height)
		return;

	/* 部分的に描画しない場合のクリッピングを行う */
	image_real_x = image_x + margin_left;
	image_real_y = image_y + margin_top;
	font_real_x = 0;
	font_real_y = 0;
	font_real_width = font_width;
	font_real_height = font_height;
	if (image_real_x < 0) {
		font_real_x -= image_real_x;
		font_real_width += image_real_x;
		image_real_x = 0;
	}
	if (image_real_x + font_real_width >= image_width) {
		font_real_width -= (image_real_x + font_real_width) -
				   image_width;
	}
	if (image_real_y < 0) {
		font_real_y -= image_real_y;
		font_real_height += image_real_y;
		image_real_y = 0;
	}
	if (image_real_y + font_real_height >= image_height) {
		font_real_height -= (image_real_y + font_real_height) -
				    image_height;
	}

	color = make_pixel(255,
			   get_pixel_r(color),
			   get_pixel_g(color),
			   get_pixel_b(color));

	/* 描画する */
	dst_ptr = image + image_real_y * image_width + image_real_x;
	src_ptr = font + font_real_y * font_width + font_real_x;
	for (py = font_real_y; py < font_real_y + font_real_height; py++) {
		for (px = font_real_x; px < font_real_x + font_real_width; px++) {
			/* フォントのピクセル値が0なら書き込まず、1以上ならブレンドなしで上書きする */
			if (*src_ptr++ == 0)
				dst_ptr++;
			else
				*dst_ptr++ = color;
		}
		dst_ptr += image_width - font_real_width;
		src_ptr += font_width - font_real_width;
	}
}

/*
 * Text layout and drawing
 */

/* Forward declarations. */
static void process_escape_sequence(struct draw_msg_context *context);
static bool process_escape_sequence_centering(struct draw_msg_context *context);
static bool process_escape_sequence_rightify(struct draw_msg_context *context);
static bool process_escape_sequence_leftify(struct draw_msg_context *context);
static void process_escape_sequence_lf(struct draw_msg_context *context);
static bool process_escape_sequence_font(struct draw_msg_context *context);
static bool process_escape_sequence_outline(struct draw_msg_context *context);
static bool process_escape_sequence_color(struct draw_msg_context *context);
static bool process_escape_sequence_size(struct draw_msg_context *context);
static bool process_escape_sequence_wait(struct draw_msg_context *context);
static bool process_escape_sequence_pen(struct draw_msg_context *context);
static bool process_escape_sequence_ruby(struct draw_msg_context *context);
static bool process_escape_sequence_emoticon(struct draw_msg_context *context);
static bool process_escape_sequence_background(struct draw_msg_context *context);
static bool process_escape_sequence_line_margin(struct draw_msg_context *context);
static bool process_escape_sequence_left_top_margins(struct draw_msg_context *context);
static bool search_for_end_of_escape_sequence(const char **msg);
static bool do_word_wrapping(struct draw_msg_context *context);
static int get_en_word_width(struct draw_msg_context *context);
static uint32_t convert_tategaki_char(uint32_t wc);
static bool is_tategaki_punctuation(uint32_t wc);
static bool process_lf(struct draw_msg_context *context, uint32_t c, int glyph_width, int glyph_height, uint32_t c_next, int next_glyph_width, int next_glyph_height);
static bool is_gyomatsu_kinsoku(uint32_t c);
static bool is_gyoto_kinsoku(uint32_t c);
static bool is_small_kana(uint32_t wc);

/*
 * Initialize a message drawing context.
 *  - Too many parameters, but for now, I think it's useful to detect bugs
 *    as compile errors when we add a change on the design.
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
	bool use_tategaki)
{
	context->stage_layer = stage_layer;
	context->msg = msg;
	context->font = font;
	context->font_size = font_size;
	context->base_font_size = base_font_size;
	context->ruby_size = ruby_size;
	context->use_outline = use_outline;
	context->outline_width = outline_width;
	context->pen_x = pen_x;
	context->pen_y = pen_y;
	context->area_width = area_width;
	context->area_height = area_height;
	context->left_margin = left_margin;
	context->right_margin = right_margin;
	context->top_margin = top_margin;
	context->bottom_margin = bottom_margin;
	context->line_margin = line_margin;
	context->char_margin = char_margin;
	context->color = color;
	context->outline_color = outline_color;
	context->is_dimming = is_dimming;
	context->ignore_linefeed = ignore_linefeed;
	context->ignore_font = ignore_font;
	context->ignore_outline = ignore_outline;
	context->ignore_color = ignore_color;
	context->ignore_size = ignore_size;
	context->ignore_position = ignore_position;
	context->ignore_ruby = ignore_ruby;
	context->ignore_wait = ignore_wait;
	context->fill_bg = fill_bg;
	context->inline_wait_hook = inline_wait_hook;
	context->use_tategaki = use_tategaki;
	context->bg_color = make_pixel((uint32_t)conf_msgbox_fill_color_a,
				       (uint32_t)conf_msgbox_fill_color_r,
				       (uint32_t)conf_msgbox_fill_color_g,
				       (uint32_t)conf_msgbox_fill_color_b);

	/* Get a layer image. */
	if (stage_layer != -1)
		context->layer_image = get_layer_image(stage_layer);

	/* The first character is treated as after-space. */
	context->runtime_is_after_space = true;

	/* Set line-top.. */
	context->runtime_is_line_top = true;

	/* "no-beginning-of-line" rule. */
	context->runtime_is_gyoto_kinsoku = false;
	context->runtime_is_gyoto_kinsoku_second = false;

	/* Set zeros. */
	context->runtime_is_inline_wait = false;
	context->runtime_ruby_x = 0;
	context->runtime_ruby_y = 0;

	/* Is quoted? */
	if (conf_serif_quote_indent && is_quoted_serif(msg) && !use_tategaki)
		context->left_margin += get_glyph_width(font, font_size, U32_C('　'));
}

/*
 * Set an alternative target image.
 */
void set_alternative_target_image(struct draw_msg_context *context,
				  struct image *img)
{
	context->layer_image = img;
}

/*
 * エスケープシーケンスを除いた描画文字数を取得する
 *  - Unicodeの合成はサポートしていない
 *  - 基底文字+結合文字はそれぞれ1文字としてカウントする
 */
int count_chars_common(struct draw_msg_context *context, int *width)
{
	const char *msg;
	uint32_t wc;
	int count, mblen;

	if (width != NULL)
		*width = 0;

	count = 0;
	msg = context->msg;
	while (*msg) {
		/* 先頭のエスケープシーケンスを読み飛ばす */
		while (*msg == '\\') {
			/* HINT: エスケープシーケンスの追加時、ここの修正を忘れずに */
			switch (*(msg + 1)) {
			/*
			 * 1文字だけのエスケープシーケンス
			 */
			case 'c':	/* センタリング */
			case 'r':	/* 右寄せ */
			case 'l':	/* 左寄せ */
				msg += 2;
				break;
			case 'n':	/* 改行 */
				msg += 2;
				if (width != NULL)
					return count;
				break;
			/*
			 * ブロックつきのエスケープシーケンス
			 */
			case 'f':	/* フォント指定 */
			case 'o':	/* アウトライン指定 */
			case '#':	/* 色指定 */
			case '@':	/* サイズ指定 */
			case 'w':	/* インラインウェイト */
			case 'p':	/* ペン移動 */
			case 'e':	/* エモーティコン */
			case '^':	/* ルビ */
			case 'L':	/* 行間 */
			case 'M':	/* マージン */
			case 'k':	/* 背景色 */
				if (!search_for_end_of_escape_sequence(&msg))
					return count;
				break;
			default:
				/*
				 * 不正なエスケープシーケンス
				 *  - 読み飛ばさない
				 */
				return count;
			}
		}
		if (*msg == '\0')
			break;

		/* 次の1文字を取得する */
		mblen = utf8_to_utf32(msg, &wc);
		if (mblen == -1)
			break;

		if (width != NULL)
			*width += get_glyph_width(context->font, context->font_size, wc);

		msg += mblen;
		count++;
	}

	return count;
}

/* エスケープシーケンスの終了位置までポインタをインクリメントする */
static bool search_for_end_of_escape_sequence(const char **msg)
{
	const char *s;
	int len;

	s = *msg;
	len = 0;
	while (*s != '\0' && *s != '}') {
		s++;
		len++;
	}
	if (*s == '\0')
		return false;

	*msg += len + 1;
	return true;
}

/*
 * Draw characters in a message up to (max_chars) characters.
 */
int
draw_msg_common(
	struct draw_msg_context *context,	/* a drawing context. */
	int char_count)				/* characters to draw. */
{
	uint32_t wc = 0;
	uint32_t wc_next = 0;
	int i, mblen;
	int glyph_width, glyph_height, next_glyph_width, next_glyph_height, ofs_x, ofs_y;
	int ret_width = 0, ret_height = 0;

	context->font = translate_font_type(context->font);
	apply_font_size(context->font, context->font_size);

	if (char_count == -1)
		char_count = count_chars_common(context, NULL);

	/* 1文字ずつ描画する */
	for (i = 0; i < char_count; i++) {
		if (*context->msg == '\0')
			break;

		/* 先頭のエスケープシーケンスをすべて処理する */
		process_escape_sequence(context);
		if (context->runtime_is_inline_wait) {
			context->runtime_is_inline_wait = false;
			return i;
		}

		/* ワードラッピングを処理する */
		if (!do_word_wrapping(context))
			return i;

		/* 描画する文字を取得する */
		mblen = utf8_to_utf32(context->msg, &wc);
		if (mblen == -1) {
			/* Invalid utf-8 sequence. */
			return -1;
		}

		/* 行末禁則処理のために、1文字先読みする */
		if (utf8_to_utf32(context->msg + mblen, &wc_next) == -1)
			wc_next = 0;

		/* 縦書きの句読点変換を行う */
		if (context->use_tategaki) {
			wc = convert_tategaki_char(wc);
			wc_next = convert_tategaki_char(wc_next);
		}

		/* 文字の幅と高さを取得する */
		glyph_width = get_glyph_width(context->font, context->font_size, wc);
		glyph_height = get_glyph_height(context->font, context->font_size, wc);
		next_glyph_width = get_glyph_width(context->font, context->font_size, wc_next);
		next_glyph_height = get_glyph_height(context->font, context->font_size, wc_next);

		/* 右側の幅が足りなければ改行する */
		if (!process_lf(context, wc, glyph_width, glyph_height, wc_next, next_glyph_width, next_glyph_height))
			return i;

		/* 小さいひらがな/カタカタのオフセットを計算する */
		if (context->use_tategaki && is_small_kana(wc)) {
			/* FIXME: 何らかの調整を加える */
			ofs_x = context->font_size / 10;
			ofs_y = -context->font_size / 6;
		} else {
			ofs_x = 0;
			ofs_y = 0;
		}

		/* 背景を塗り潰す */
		if (context->fill_bg) {
			clear_image_color_rect(context->layer_image,
					       context->pen_x,
					       context->pen_y,
					       context->font_size,
					       context->line_margin,
					       context->bg_color);
		}

		/* 描画する */
		draw_glyph(context->layer_image,
			   context->font,
			   context->font_size,
			   context->base_font_size,
			   context->use_outline,
			   context->outline_width,
			   context->pen_x + ofs_x,
			   context->pen_y + ofs_y,
			   context->color,
			   context->outline_color,
			   wc,
			   &ret_width,
			   &ret_height,
			   context->is_dimming);

		/* ルビ用のペン位置を更新する */
		if (!context->use_tategaki) {
			context->runtime_ruby_x = context->pen_x;
			context->runtime_ruby_y = context->pen_y -
				context->ruby_size;
		} else {
			context->runtime_ruby_x = context->pen_x + ret_width;
			context->runtime_ruby_y = context->pen_y;
		}

		/* 次の文字へ移動する */
		context->msg += mblen;
		if (!context->use_tategaki) {
			context->pen_x += glyph_width + context->char_margin;
		} else {
			if (is_tategaki_punctuation(wc))
				context->pen_y += context->font_size;
			else
				context->pen_y += glyph_height;
			context->pen_y += context->char_margin;
		}
	}

	/* 末尾のエスケープシーケンスを処理する */
	process_escape_sequence(context);
	if (context->runtime_is_inline_wait)
		context->runtime_is_inline_wait = false;

	/* 描画した文字数を返す */
	return i;
}

/* ワードラッピングを処理する */
static bool do_word_wrapping(struct draw_msg_context *context)
{
	if (context->use_tategaki)
		return true;

	if (context->runtime_is_after_space) {
		if (context->pen_x + get_en_word_width(context) >=
		    context->area_width - context->right_margin) {
			if (context->ignore_linefeed)
				return false;

			context->pen_y += context->line_margin;
			context->pen_x = context->left_margin;
		}
	}

	context->runtime_is_after_space = *context->msg == ' ';

	return true;
}

/* msgが英単語の先頭であれば、その単語の描画幅、それ以外の場合0を返す */
static int get_en_word_width(struct draw_msg_context *context)
{
	const char *m;
	uint32_t wc;
	int width;

	m = context->msg;
	width = 0;
	while (isgraph_extended(&m, &wc))
		width += get_glyph_width(context->font, context->font_size, wc);

	return width;
}

/* 右側の幅が足りなければ改行する */
static bool process_lf(struct draw_msg_context *context, uint32_t c, int glyph_width, int glyph_height, uint32_t c_next, int next_glyph_width, int next_glyph_height)
{
	bool line_top, gyoto_second;

	line_top = context->runtime_is_line_top;
	gyoto_second = context->runtime_is_gyoto_kinsoku_second;
	context->runtime_is_line_top = false;
	context->runtime_is_gyoto_kinsoku_second = false;

	/* 前の文字で行頭禁則を検出した場合 */
	if (context->runtime_is_gyoto_kinsoku) {
		/* 行頭禁則を解除する */
		context->runtime_is_gyoto_kinsoku = false;
		context->runtime_is_gyoto_kinsoku_second = true;

		/* LFを無視する場合は、描画を終了する */
		if (context->ignore_linefeed && context->line_margin == 0)
			return false;

		/* 改行しない */
		return true;
	}

	if (!context->use_tategaki) {
		int limit = context->area_width - context->right_margin;

		/* 右側の幅が足りない場合 */
		if (context->pen_x + glyph_width + context->char_margin >= limit) {
			/* cが行頭禁則禁則文字の場合は改行しない */
			if (is_gyoto_kinsoku(c) && !line_top && !gyoto_second)
				return true;

			/* LFを無視する場合は、描画を終了する */
			if (context->ignore_linefeed && context->line_margin == 0)
				return false;

			/* 改行する */
			context->pen_y += context->line_margin;
			context->pen_x = context->left_margin;
			context->runtime_is_line_top = true;
			if (context->pen_y + context->line_margin >= context->area_height)
				return false; /* 描画終了 */
		} else {
			/* 右幅が足りる場合 */
			if (line_top) {
				/* 行頭なら改行しない */
			} else if (is_gyomatsu_kinsoku(c)) {
				/* cが行末禁止文字の場合は改行しない */
			} else if (is_gyoto_kinsoku(c_next) &&
				   context->pen_x + glyph_width + context->char_margin + next_glyph_width + context->char_margin >= limit) {
				/* c_nextが行頭禁則文字で右端からはみ出るなら改行しない */
				/* 次の文字で改行しないようにフラグを立てる */
				context->runtime_is_gyoto_kinsoku = true;
			}
		}
	} else {
		int limit = context->area_height - context->bottom_margin;

		/* 下側の幅が足りない場合 */
		if (context->pen_y + glyph_height + context->char_margin >= limit) {
			/* cが行頭禁則禁則文字の場合は改行しない */
			if (is_gyoto_kinsoku(c) && !line_top && !gyoto_second)
				return true;

			/* LFを無視する場合は、描画を終了する */
			if (context->ignore_linefeed && context->line_margin == 0)
				return false;

			/* 改行する */
			context->pen_x -= context->line_margin;
			context->pen_y = context->top_margin;
			context->runtime_is_line_top = true;
			if (context->pen_x + context->line_margin < 0)
				return false; /* 描画終了 */
		} else {
			/* 下幅が足りる場合 */
			if (line_top) {
				/* 行頭なら改行しない */
			} else if (is_gyomatsu_kinsoku(c)) {
				/* cが行末禁止文字の場合は改行しない */
			} else if (is_gyoto_kinsoku(c_next) &&
				   context->pen_y + glyph_height + context->char_margin + next_glyph_height + context->char_margin >= limit) {
				/* c_nextが行頭禁則文字で下端からはみ出るなら改行しない */
				/* 次の文字で改行しないようにフラグを立てる */
				context->runtime_is_gyoto_kinsoku = true;
			}
		}
	}

	/* 改行しない */
	return true;
}

/* Check if "no-end-of-line" ruled character. */
static bool is_gyomatsu_kinsoku(uint32_t c)
{
	switch (c) {
	case '(':
	case '[':
	case '{':
	case U32_C('（'):
	case U32_C('︵'):
	case U32_C('｛'):
	case U32_C('︷'):
	case U32_C('「'):
	case U32_C('﹁'):
	case U32_C('『'):
	case U32_C('﹃'):
	case U32_C('【'):
	case U32_C('︻'):
	case U32_C('［'):
	case U32_C('﹇'):
	case U32_C('〔'):
	case U32_C('︹'):
	case U32_C('〘'):
	case U32_C('〖'):
	case U32_C('《'):
	case U32_C('︽'):
	case U32_C('〈'): // U+3008
	case U32_C('〈'): // U+2329
	case U32_C('｟'):
	case U32_C('«'):
	case U32_C('〝'):
	case U32_C('‘'):
	case U32_C('“'):
		return true;
	default:
		break;
	}
	return false;
}

/* Check if "no-beginning-of-line" ruled character. */
static bool is_gyoto_kinsoku(uint32_t c)
{
	switch (c) {
	case ' ':
	case ',':
	case '.':
	case '!':
	case '?':
	case ':':
	case ';':
	case ')':
	case ']':
	case '}':
	case '/':
	case U32_C('？'):
	case U32_C('、'):
	case U32_C('︑'):
	case U32_C('，'):
	case U32_C('︐'):
	case U32_C('。'):
	case U32_C('︒'):
	case U32_C('〕'):
	case U32_C('〉'):
	case U32_C('》'):
	case U32_C('」'):
	case U32_C('』'):
	case U32_C('】'):
	case U32_C('〙'):
	case U32_C('〗'):
	case U32_C('︘'):
	case U32_C('〟'):
	case U32_C('’'):
	case U32_C('”'):
	case U32_C('｠'):
	case U32_C('»'):
	case U32_C('ゝ'):
	case U32_C('ゞ'):
	case U32_C('‐'):
	case U32_C('–'):
	case U32_C('ー'):
	case U32_C('丨'):
	case U32_C('︙'):
	case U32_C('︰'):
	case U32_C('ァ'):
	case U32_C('ィ'):
	case U32_C('ゥ'):
	case U32_C('ェ'):
	case U32_C('ォ'):
	case U32_C('ッ'):
	case U32_C('ャ'):
	case U32_C('ュ'):
	case U32_C('ョ'):
	case U32_C('ヮ'):
	case U32_C('ヵ'):
	case U32_C('ヶ'):
	case U32_C('ぁ'):
	case U32_C('ぃ'):
	case U32_C('ぅ'):
	case U32_C('ぇ'):
	case U32_C('ぉ'):
	case U32_C('っ'):
	case U32_C('ゃ'):
	case U32_C('ゅ'):
	case U32_C('ょ'):
	case U32_C('ゎ'):
	case U32_C('ゕ'):
	case U32_C('ゖ'):
	case U32_C('ㇰ'):
	case U32_C('ㇱ'):
	case U32_C('ㇲ'):
	case U32_C('ㇳ'):
	case U32_C('ㇴ'):
	case U32_C('ㇵ'):
	case U32_C('ㇶ'):
	case U32_C('ㇷ'):
	case U32_C('ㇸ'):
	case U32_C('ㇹ'):
	case U32_C('゚'):
	case U32_C('ㇺ'):
	case U32_C('ㇻ'):
	case U32_C('ㇼ'):
	case U32_C('ㇽ'):
	case U32_C('ㇾ'):
	case U32_C('ㇿ'):
	case U32_C('々'):
	case U32_C('〻'):
	case U32_C('゠'):
	case U32_C('〜'):
	case U32_C('～'):
	case U32_C('‼'):
	case U32_C('⁇'):
	case U32_C('⁈'):
	case U32_C('⁉'):
	case U32_C('・'):
		return true;
	default:
		break;
	}
	return false;
}

/* 縦書きの句読点変換を行う */
static uint32_t convert_tategaki_char(uint32_t wc)
{
	switch (wc) {
	case U32_C('、'): return U32_C('︑');
	case U32_C('，'): return U32_C('︐');
	case U32_C('。'): return U32_C('︒');
	case U32_C('（'): return U32_C('︵');
	case U32_C('）'): return U32_C('︶');
	case U32_C('｛'): return U32_C('︷');
	case U32_C('｝'): return U32_C('︸');
	case U32_C('「'): return U32_C('﹁');
	case U32_C('」'): return U32_C('﹂');
	case U32_C('『'): return U32_C('﹃');
	case U32_C('』'): return U32_C('﹄');
	case U32_C('【'): return U32_C('︻');
	case U32_C('】'): return U32_C('︼');
	case U32_C('［'): return U32_C('﹇');
	case U32_C('］'): return U32_C('﹈');
	case U32_C('〔'): return U32_C('︹');
	case U32_C('〕'): return U32_C('︺');
	case U32_C('…'): return U32_C('︙');
	case U32_C('‥'): return U32_C('︰');
	case U32_C('ー'): return U32_C('丨');
	case U32_C('─'): return U32_C('丨');
	case U32_C('〈'): return U32_C('︿'); // U+3008 -> U+FE3F
	case U32_C('〈'): return U32_C('︿'); // U+2329 -> U+FE3F
	case U32_C('〉'): return U32_C('⟩');  // U+3009 -> U+FE40
	case U32_C('〉'): return U32_C('⟩');  // U+232A -> U+FE40
	case U32_C('《'): return U32_C('︽');
	case U32_C('》'): return U32_C('︾');
	case U32_C('〖'): return U32_C('︗');
	case U32_C('〗'): return U32_C('︘');
	default:
		break;
	}
	return wc;
}

/* 縦書きの句読点かどうか調べる */
static bool is_tategaki_punctuation(uint32_t wc)
{
	switch (wc) {
	case U32_C('︑'): return true;
	case U32_C('︐'): return true;
	case U32_C('︒'): return true;
	case U32_C('︵'): return true;
	case U32_C('︶'): return true;
	case U32_C('︷'): return true;
	case U32_C('︸'): return true;
	case U32_C('﹁'): return true;
	case U32_C('﹂'): return true;
	case U32_C('﹃'): return true;
	case U32_C('﹄'): return true;
	case U32_C('︻'): return true;
	case U32_C('︼'): return true;
	case U32_C('﹇'): return true;
	case U32_C('﹈'): return true;
	case U32_C('︹'): return true;
	case U32_C('︺'): return true;
	case U32_C('︙'): return true;
	case U32_C('︰'): return true;
	case U32_C('丨'): return true;
	default:
		break;
	}
	return false;
}

/* 小さい仮名文字であるか調べる */
static bool is_small_kana(uint32_t wc)
{
	switch (wc) {
	case U32_C('ぁ'): return true;
	case U32_C('ぃ'): return true;
	case U32_C('ぅ'): return true;
	case U32_C('ぇ'): return true;
	case U32_C('ぉ'): return true;
	case U32_C('っ'): return true;
	case U32_C('ゃ'): return true;
	case U32_C('ゅ'): return true;
	case U32_C('ょ'): return true;
	case U32_C('ゎ'): return true;
	case U32_C('ゕ'): return true;
	case U32_C('ゖ'): return true;
	case U32_C('ァ'): return true;
	case U32_C('ィ'): return true;
	case U32_C('ゥ'): return true;
	case U32_C('ェ'): return true;
	case U32_C('ォ'): return true;
	case U32_C('ッ'): return true;
	case U32_C('ャ'): return true;
	case U32_C('ュ'): return true;
	case U32_C('ョ'): return true;
	case U32_C('ヮ'): return true;
	case U32_C('ヵ'): return true;
	case U32_C('ヶ'): return true;
	default: break;
	}
	return false;
}

/*
 * エスケープ文字かチェックする
 */
bool is_escape_sequence_char(char c)
{
	/* HINT: エスケープシーケンスの追加時、ここの修正を忘れずに */
	switch (c) {
	case 'c':	/* センタリング */
	case 'r':	/* 右寄せ */
	case 'l':	/* 左寄せ */
	case 'n':	/* 改行 */
	case 'f':	/* フォント指定 */
	case 'o':	/* アウトライン指定 */
	case '#':	/* 色指定 */
	case '@':	/* サイズ指定 */
	case 'w':	/* インラインウェイト */
	case 'p':	/* ペン移動 */
	case 'e':	/* エモーティコン */
	case '^':	/* ルビ */
	case 'L':	/* 行間 */
	case 'M':	/* マージン */
	case 'k':	/* 背景色 */
		return true;
	default:
		break;
	}

	return false;
}

/* 先頭のエスケープシーケンスを処理する */
static void process_escape_sequence(struct draw_msg_context *context)
{
	/* エスケープシーケンスが続く限り処理する */
	while (*context->msg == '\\') {
		/* HINT: エスケープシーケンスの追加時、ここの修正を忘れずに */
		switch (*(context->msg + 1)) {
		/*
		 * 1文字のエスケープシーケンス
		 */
		case 'c':
			/* センタリング */
			if (!process_escape_sequence_centering(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case 'r':
			/* 右寄せ */
			if (!process_escape_sequence_rightify(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case 'l':
			/* 左寄せ */
			if (!process_escape_sequence_leftify(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case 'n':
			/* 改行 */
			process_escape_sequence_lf(context);
			break;
		/*
		 * ブロックつきのエスケープシーケンス
		 */
		case 'f':
			/* フォント指定 */
			if (!process_escape_sequence_font(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case 'o':
			/* アウトライン指定 */
			if (!process_escape_sequence_outline(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case '#':
			/* 色指定 */
			if (!process_escape_sequence_color(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case '@':
			/* サイズ指定 */
			if (!process_escape_sequence_size(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case 'w':
			/* インラインウェイト */
			if (!process_escape_sequence_wait(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case 'p':
			/* ペン移動 */
			if (!process_escape_sequence_pen(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case 'e':
			/* エモーティコン */
			if (!process_escape_sequence_emoticon(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case '^':
			/* ルビ */
			if (!process_escape_sequence_ruby(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case 'k':
			/* 背景塗り潰し */
			if (!process_escape_sequence_background(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case 'L':
			/* 行間 */
			if (!process_escape_sequence_line_margin(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case 'M':
			/* 左右上下のマージン */
			if (!process_escape_sequence_left_top_margins(context))
				return; /* 不正: 読み飛ばさない */
			break;
		default:
			/* 不正なエスケープシーケンスなので読み飛ばさない */
			return;
		}
	}
}

/* 改行("\\n")を処理する */
static void process_escape_sequence_lf(struct draw_msg_context *context)
{
	if (context->ignore_linefeed) {
		context->msg += 2;
		return;
	}

	if (!context->use_tategaki) {
		context->pen_y += context->line_margin;
		context->pen_x = context->left_margin;
	} else {
		context->pen_x -= context->line_margin;
		context->pen_y = context->top_margin;
	}
	context->msg += 2;
}

/* フォント指定("\\f{X}")を処理する */
static bool process_escape_sequence_font(struct draw_msg_context *context)
{
	char font_type;
	const char *p;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == 'f');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* 長さが足りない場合 */
	if (strlen(p + 3) < 6)
		return false;

	/* '}'をチェックする */
	if (*(p + 4) != '}')
		return false;

	if (!context->ignore_font) {
		/* フォントタイプを読む */
		font_type = *(p + 3);
		switch (font_type) {
		case 'g':
			context->font = FONT_GLOBAL;
			break;
		case 'm':
			context->font = translate_font_type(FONT_MAIN);
			break;
		case 'a':
			context->font = translate_font_type(FONT_ALT1);
			break;
		case 'b':
			context->font = translate_font_type(FONT_ALT2);
			break;
		default:
			break;
		}
	}

	/* "\\f{" + "X" + "}" */
	context->msg += 3 + 1 + 1;
	return true;
}

/* アウトライン指定("\\o{X}")を処理する */
static bool process_escape_sequence_outline(struct draw_msg_context *context)
{
	char outline_type;
	const char *p;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == 'o');

	/* '{'をチェックする */
	if (*(p + 2) != '{') {
		log_memory();
		return false;
	}

	/* 長さが足りない場合 */
	if (*(p + 3) == '\0') {
		log_memory();
		return false;
	}

	/* '}'をチェックする */
	if (*(p + 4) != '}') {
		log_memory();
		return false;
	}

	if (!context->ignore_outline) {
		/* アウトラインタイプを読む */
		outline_type = *(p + 3);
		switch (outline_type) {
		case '+':
			context->use_outline = true;
			break;
		case '-':
			context->use_outline = false;
			break;
		default:
			break;
		}
	}

	/* "\\o{" + "X" + "}" */
	context->msg += 3 + 1 + 1;
	return true;
}

/* 色指定("\\#{RRGGBB}")を処理する */
static bool process_escape_sequence_color(struct draw_msg_context *context)
{
	char color_code[7];
	const char *p;
	uint32_t r, g, b;
	int rgb;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == '#');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* \#{DEF} */
	if (*(p + 3) == 'D' &&
	    *(p + 4) == 'E' &&
	    *(p + 5) == 'F' &&
	    *(p + 6) == '}') {
		context->color = make_pixel(0xff,
					    (pixel_t)conf_font_color_r,
					    (pixel_t)conf_font_color_g,
					    (pixel_t)conf_font_color_b);
		context->msg += 3 + 3 + 1;
		return true;
	}

	/* 長さが足りない場合 */
	if (strlen(p + 3) < 6)
		return false;

	/* '}'をチェックする */
	if (*(p + 9) != '}')
		return false;

	if (!context->ignore_color) {
		/* カラーコードを読む */
		memcpy(color_code, p + 3, 6);
		color_code[6] = '\0';
		rgb = 0;
		sscanf(color_code, "%x", &rgb);
		r = (rgb >> 16) & 0xff;
		g = (rgb >> 8) & 0xff;
		b = rgb & 0xff;
		context->color = make_pixel(0xff, r, g, b);
	}

	/* "\\#{" + "RRGGBB" + "}" */
	context->msg += 3 + 6 + 1;
	return true;
}

/* サイズ指定("\\@{xxx}")を処理する */
static bool process_escape_sequence_size(struct draw_msg_context *context)
{
	char size_spec[8];
	const char *p;
	int i, size;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == '@');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* サイズ文字列を読む */
	for (i = 0; i < (int)sizeof(size_spec) - 1; i++) {
		if (*(p + 3 + i) == '\0')
			return false;
		if (*(p + 3 + i) == '}')
			break;
		size_spec[i] = *(p + 3 + i);
	}
	size_spec[i] = '\0';

	if (!context->ignore_size) {
		/* サイズ文字列を整数に変換する */
		size = 0;
		if (size_spec[0] == '!') {
			sscanf(&size_spec[1], "%d", &size);

			/* ベースサイズを変更する */
			context->base_font_size = size;
		} else {
			sscanf(&size_spec[0], "%d", &size);
		}

		/* フォントサイズを変更する */
		context->font_size = size;
	}

	/* "\\@{" + "xxx" + "}" */
	context->msg += 3 + i + 1;
	return true;
}

/* インラインウェイト("\\w{f.f}")を処理する */
static bool process_escape_sequence_wait(struct draw_msg_context *context)
{
	char time_spec[16];
	const char *p;
	float wait_time;
	int i;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == 'w');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* 時間文字列を読む */
	for (i = 0; i < (int)sizeof(time_spec) - 1; i++) {
		if (*(p + 3 + i) == '\0')
			return false;
		if (*(p + 3 + i) == '}')
			break;
		time_spec[i] = *(p + 3 + i);
	}
	time_spec[i] = '\0';

	if (!context->ignore_wait) {
		/* 時間文字列を浮動小数点数に変換する */
		sscanf(time_spec, "%f", &wait_time);

		/* ウェイトを処理する */
		context->runtime_is_inline_wait = true;
		context->inline_wait_hook(wait_time);
	}

	/* "\\w{" + "f.f" + "}" */
	context->msg += 3 + i + 1;
	return true;
}

/* ペン移動("\\p{x,y}")を処理する */
static bool process_escape_sequence_pen(struct draw_msg_context *context)
{
	char pos_spec[32];
	const char *p;
	int i, pen_x, pen_y;
	bool separator_found;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == 'p');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* 座標文字列を読む */
	separator_found = false;
	for (i = 0; i < (int)sizeof(pos_spec) - 1; i++) {
		if (*(p + 3 + i) == '\0')
			return false;
		if (*(p + 3 + i) == '}')
			break;
		if (*(p + 3 + i) == ',')
			separator_found = true;
		pos_spec[i] = *(p + 3 + i);
	}
	pos_spec[i] = '\0';
	if (!separator_found)
		return false;

	if (!context->ignore_position) {
		/* 座標文字列を浮動小数点数に変換する */
		sscanf(pos_spec, "%d,%d", &pen_x, &pen_y);

		/* 描画位置を更新する */
		context->pen_x = pen_x;
		context->pen_y = pen_y;
	}

	/* "\\w{" + "x,y" + "}" */
	context->msg += 3 + i + 1;
	return true;
}

/* ルビ("\\^{ルビ}")を処理する */
static bool process_escape_sequence_ruby(struct draw_msg_context *context)
{
	char ruby[64];
	const char *p;
	uint32_t wc = 0;
	int i, mblen, ret_w, ret_h;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == '^');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* ルビを読む */
	for (i = 0; i < (int)sizeof(ruby) - 1; i++) {
		if (*(p + 3 + i) == '\0')
			return false;
		if (*(p + 3 + i) == '}')
			break;
		ruby[i] = *(p + 3 + i);
	}
	ruby[i] = '\0';

	/* \^{ + ruby[] + } */
	context->msg += 3 + i + 1;

	if (context->ignore_ruby)
		return true;

	/* 描画する */
	p = ruby;
	while (*p) {
		mblen = utf8_to_utf32(p, &wc);
		if (mblen == -1)
			return false;

		draw_glyph(context->layer_image,
			   context->font,
			   context->ruby_size,
			   context->ruby_size,
			   context->use_outline,
			   context->outline_width,
			   context->runtime_ruby_x,
			   context->runtime_ruby_y,
			   context->color,
			   context->outline_color,
			   wc,
			   &ret_w,
			   &ret_h,
			   context->is_dimming);

		if (!context->use_tategaki)
			context->runtime_ruby_x += ret_w;
		else
			context->runtime_ruby_y += ret_h;

		p += mblen;
	}

	return true;
}

/* エモーティコン("\\e{絵文字名}")を処理する */
static bool process_escape_sequence_emoticon(struct draw_msg_context *context)
{
	char name[256];
	const char *p;
	int i, ret_w, ret_h;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == 'e');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* 絵文字名を読む */
	for (i = 0; i < (int)sizeof(name) - 1; i++) {
		if (*(p + 3 + i) == '\0')
			return false;
		if (*(p + 3 + i) == '}')
			break;
		name[i] = *(p + 3 + i);
	}
	name[i] = '\0';

	/* \^{ + name[] + } */
	context->msg += 3 + i + 1;

	/* 描画する */
	ret_w = 0;
	ret_h = 0;
	if (!draw_emoticon(context, name, &ret_w, &ret_h))
		return false;

	if (!context->use_tategaki)
		context->pen_x += ret_w;
	else
		context->pen_y += ret_h;

	return true;
}

/* 行間("\L{n}")を処理する */
static bool process_escape_sequence_line_margin(struct draw_msg_context *context)
{
	char digits[32];
	const char *p;
	int i;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == 'L');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* サイズ文字列を読む */
	for (i = 0; i < (int)sizeof(digits) - 1; i++) {
		if (*(p + 3 + i) == '\0')
			return false;
		if (*(p + 3 + i) == '}')
			break;
		digits[i] = *(p + 3 + i);
	}
	digits[i] = '\0';

	/* 行間を変更する */
	context->line_margin = 0;
	sscanf(digits, "%d", &context->line_margin);

	/* "\\L{" + "xxx" + "}" */
	context->msg += 3 + i + 1;
	return true;
}

/* 左と上のマージン("\M{x,y}")を処理する */
static bool process_escape_sequence_left_top_margins(struct draw_msg_context *context)
{
	char margin_spec[32];
	const char *p;
	int i, left, top;
	bool separator_found;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == 'M');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* マージン文字列を読む */
	separator_found = false;
	for (i = 0; i < (int)sizeof(margin_spec) - 1; i++) {
		if (*(p + 3 + i) == '\0')
			return false;
		if (*(p + 3 + i) == '}')
			break;
		if (*(p + 3 + i) == ',')
			separator_found = true;
		margin_spec[i] = *(p + 3 + i);
	}
	margin_spec[i] = '\0';
	if (!separator_found)
		return false;

	if (!context->ignore_position) {
		/* 座標文字列を浮動小数点数に変換する */
		sscanf(margin_spec, "%d,%d", &left, &top);

		/* マージンと描画位置を更新する */
		if (!context->use_tategaki) {
			context->left_margin = left;
			context->top_margin = top;
			context->pen_x = left;
			context->pen_y = top;
		} else {
			context->right_margin = left;
			context->top_margin = top;
			context->pen_x = left;
			context->pen_y = top;
		}
	}

	/* "\\M{" + "x,y" + "}" */
	context->msg += 3 + i + 1;
	return true;
}

/* センタリング("\c")を処理する */
static bool process_escape_sequence_centering(struct draw_msg_context *context)
{
	int width;

	count_chars_common(context, &width);

	if (!context->use_tategaki)
		context->pen_x = (context->area_width - width) / 2;
	else
		context->pen_y = (context->area_height - width) / 2;

	context->msg += 2;
	return true;
}

/* 右寄せ("\r")を処理する */
static bool process_escape_sequence_rightify(struct draw_msg_context *context)
{
	int width;

	count_chars_common(context, &width);

	if (!context->use_tategaki)
		context->pen_x = context->area_width - context->right_margin - width - 1;
	else
		context->pen_y = context->area_height - context->bottom_margin - width;

	context->msg += 2;
	return true;
}

/* 左寄せ("\l")を処理する */
static bool process_escape_sequence_leftify(struct draw_msg_context *context)
{
	int width;

	count_chars_common(context, &width);

	if (!context->use_tategaki)
		context->pen_x = context->left_margin;
	else
		context->pen_y = context->top_margin;

	context->msg += 2;
	return true;
}

/* 背景塗り潰し("\\k{RRGGBB}")を処理する */
static bool process_escape_sequence_background(struct draw_msg_context *context)
{
	char color_code[7];
	const char *p;
	uint32_t r, g, b;
	int rgb;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == 'k');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* \k{OFF} */
	if (*(p + 3) == 'D' &&
	    *(p + 4) == 'E' &&
	    *(p + 5) == 'F' &&
	    *(p + 6) == '}') {
		context->fill_bg = false;
		context->msg += 3 + 3 + 1;
		return true;
	}

	/* \k{DEF} */
	if (*(p + 3) == 'D' &&
	    *(p + 4) == 'E' &&
	    *(p + 5) == 'F' &&
	    *(p + 6) == '}') {
		context->fill_bg = conf_msgbox_fill;
		context->bg_color = make_pixel((uint32_t)conf_msgbox_fill_color_a,
					       (uint32_t)conf_msgbox_fill_color_r,
					       (uint32_t)conf_msgbox_fill_color_g,
					       (uint32_t)conf_msgbox_fill_color_b);
		context->msg += 3 + 3 + 1;
		return true;
	}

	/* 長さが足りない場合 */
	if (strlen(p + 3) < 6)
		return false;

	/* '}'をチェックする */
	if (*(p + 9) != '}')
		return false;

	if (!context->ignore_color) {
		/* カラーコードを読む */
		memcpy(color_code, p + 3, 6);
		color_code[6] = '\0';
		rgb = 0;
		sscanf(color_code, "%x", &rgb);
		r = (rgb >> 16) & 0xff;
		g = (rgb >> 8) & 0xff;
		b = rgb & 0xff;
		context->bg_color = make_pixel(0xff, r, g, b);
		context->fill_bg = true;
	}

	/* "\\#{" + "RRGGBB" + "}" */
	context->msg += 3 + 6 + 1;
	return true;
}

/*
 * Get a pen position.
 */
void get_pen_position_common(struct draw_msg_context *context, int *pen_x,
			     int *pen_y)
{
	*pen_x = context->pen_x;
	*pen_y = context->pen_y;
}

/*
 * Set ignore inline wait.
 */
void set_ignore_inline_wait(struct draw_msg_context *context)
{
	context->ignore_wait = true;
}

/*
 * Emoticon
 */

/* Draw an emoticon. */
static bool
draw_emoticon(struct draw_msg_context *context,
	      const char *name,
	      int *w,
	      int *h)
{
	int i;

	for (i = 0; i < EMOTICON_COUNT; i++) {
		if (conf_emoticon_name[i] == NULL || conf_emoticon_file[i] == NULL)
			continue;
		if (strcmp(name, conf_emoticon_name[i]) == 0)
			break;
	}
	if (i == EMOTICON_COUNT)
		return false;

	if (emoticon_image[i] == NULL)
		return false;

	*w = emoticon_image[i]->width;
	*h = emoticon_image[i]->height;

	context->runtime_is_line_top = false;
	if (!context->use_tategaki) {
		int limit = context->area_width - context->right_margin;

		/* 右側の幅が足りない場合 */
		if (context->pen_x + *w + context->char_margin >= limit) {
			/* LFを無視する場合 */
			if (context->ignore_linefeed && context->line_margin == 0)
				return false; /* 描画終了 */

			/* 改行する */
			context->pen_y += context->line_margin;
			context->pen_x = context->left_margin;
			context->runtime_is_line_top = true;
			if (context->pen_y + context->line_margin >= context->area_height)
				return false; /* 描画終了 */
		}
	} else {
		int limit = context->area_height - context->bottom_margin;

		/* 下側の幅が足りない場合 */
		if (context->pen_y + *h + context->char_margin >= limit) {
			/* LFを無視する場合 */
			if (context->ignore_linefeed && context->line_margin == 0)
				return false; /* 描画終了 */

			/* 改行する */
			context->pen_x -= context->line_margin;
			context->pen_y = context->top_margin;
			context->runtime_is_line_top = true;
			if (context->pen_x + context->line_margin < 0)
				return false; /* 描画終了 */
		}
	}

	draw_image_emoji(context->layer_image,
			 context->pen_x,
			 context->pen_y,
			 emoticon_image[i],
			 *w,
			 *h,
			 0,
			 0,
			 255);

	if (!context->use_tategaki)
		*w += context->char_margin;
	else
		*h += context->char_margin;

	return true;
}
