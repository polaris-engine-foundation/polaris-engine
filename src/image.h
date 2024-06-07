/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * Image Manipulation
 */

#ifndef XENGINE_IMAGE_H
#define XENGINE_IMAGE_H

#include "types.h"

/* RGBAカラー形式のピクセル値 */
typedef uint32_t pixel_t;

/*
 * image構造体
 */
struct image {
	/* 水平方向のピクセル数 */
	int width;

	/* 垂直方向のピクセル数 */
	int height;

	/* ピクセル列 */
	SIMD_ALIGNED_MEMBER(pixel_t *pixels);

	/* (HAL internal) テクスチャへのポインタ */
	void *texture;

	/* (HAL internal) テクスチャのアップロードが必要か */
	bool need_upload;

	/* (HAL internal) */
	int id;

	/* (HAL internal) */
	int context;
};

/*
 * ピクセル値の操作
 */

#if defined(XENGINE_TARGET_WIN32) || defined(XENGINE_TARGET_MACOS) || defined(XENGINE_TARGET_IOS)
/* Direct3D, Metalの場合はRGBA形式 */
#define ORDER_RGBA
#else
/* OpenGLの場合はBGRA形式 */
#define ORDER_BGRA
#endif

/* ピクセル値を合成する */
static INLINE pixel_t make_pixel(uint32_t a, uint32_t r, uint32_t g, uint32_t b)
{
#ifdef ORDER_RGBA
	return (((pixel_t)a) << 24) | (((pixel_t)r) << 16) | (((pixel_t)g) << 8) | ((pixel_t)b);
#else
	return (((pixel_t)a) << 24) | (((pixel_t)b) << 16) | (((pixel_t)g) << 8) | ((pixel_t)r);
#endif
}

/* ピクセル値のアルファチャンネルを取得する */
static INLINE uint32_t get_pixel_a(pixel_t p)
{
	return (p >> 24) & 0xff;
}

/* ピクセル値の赤チャンネルを取得する */
static INLINE uint32_t get_pixel_r(pixel_t p)
{
#ifdef ORDER_RGBA
	return (p >> 16) & 0xff;
#else
	return p & 0xff;
#endif
}

/* ピクセル値の緑チャンネルを取得する */
static INLINE uint32_t get_pixel_g(pixel_t p)
{
#ifdef ORDER_RGBA
	return (p >> 8) & 0xff;
#else
	return (p >> 8) & 0xff;
#endif
}

/* ピクセル値の青チャンネルを取得する */
static INLINE uint32_t get_pixel_b(pixel_t p)
{
#ifdef ORDER_RGBA
	return p & 0xff;
#else
	return (p >> 16) & 0xff;
#endif
}

#undef ORDER_RGBA
#undef ORDER_BGRA

/* イメージを作成する */
struct image *create_image(int w, int h);

/* ファイル名を指定してイメージを作成する */
struct image *create_image_from_file(const char *dir, const char *file);

/* 文字列で色を指定してイメージを作成する */
struct image *create_image_from_color_string(int w, int h, const char *color);

/* Bitmap/Pixmapによるバッキングイメージを作成する */
struct image *create_image_with_pixels(int w, int h, pixel_t *pixels);

/* イメージを削除する */
void destroy_image(struct image *img);

/* イメージを黒色でクリアする */
void clear_image_black(struct image *img);

/* イメージを白色でクリアする */
void clear_image_white(struct image *img);

/* イメージを色でクリアする */
void clear_image_color(struct image *img, pixel_t color);
void clear_image_color_rect(struct image *img, int x, int y, int w, int h, pixel_t color);

/* イメージのアルファチャンネルを255でクリアする */
void fill_image_alpha(struct image *img);

/* イメージを描画する(コピー) */
void draw_image_copy(struct image *dst_image,
		     int dst_left,
		     int dst_top,
		     struct image *src_image,
		     int width,
		     int height,
		     int src_left,
		     int src_top);

/* イメージを描画する(アルファブレンド,アルファ値255固定) */
void draw_image_fast(struct image *dst_image,
		     int dst_left,
		     int dst_top,
		     struct image *src_image,
		     int width,
		     int height,
		     int src_left,
		     int src_top,
		     int alpha);

/* イメージを描画する(アルファブレンド,アルファ値が特殊) */
void draw_image_emoji(struct image *dst_image,
		      int dst_left,
		      int dst_top,
		      struct image *src_image,
		      int width,
		      int height,
		      int src_left,
		      int src_top,
		      int alpha);

/* イメージを描画する(加算) */
void draw_image_add(struct image *dst_image,
		    int dst_left,
		    int dst_top,
		    struct image *src_image,
		    int width,
		    int height,
		    int src_left,
		    int src_top,
		    int alpha);

/* イメージを描画する(50%暗くする) */
void draw_image_dim(struct image *dst_image,
		    int dst_left,
		    int dst_top,
		    struct image *src_image,
		    int width,
		    int height,
		    int src_left,
		    int src_top,
		    int alpha);

/* イメージをルール付き(1-bit)で描画する */
void draw_image_rule(struct image *dst_image,
		     struct image *src_image,
		     struct image *rule_image,
		     int threshold);

/* イメージをルール付き(8-bit)で描画する */
void draw_image_melt(struct image *dst_image,
		     struct image *src_image,
		     struct image *rule_image,
		     int threshold);

/* イメージをスケールして描画する */
void draw_image_scale(struct image *dst_image,
		      int virtual_dst_width,
		      int virtual_dst_height,
		      int virtual_dst_left,
		      int virtual_dst_top,
		      struct image *src_image);

/*
 * Helpers for rendering HALs.
 */

/* 転送元領域のサイズを元に矩形のクリッピングを行う */
bool clip_by_source(int src_cx, int src_cy, int *cx, int *cy, int *dst_x,
		    int *dst_y, int *src_x, int *src_y);

/* 転送先領域のサイズを元に矩形のクリッピングを行う */
bool clip_by_dest(int dst_cx, int dst_cy, int *cx, int *cy, int *dst_x,
		  int *dst_y, int *src_x, int *src_y);

#endif /* XENGINE_IMAGE_H */
