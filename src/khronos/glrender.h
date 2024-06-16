/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * The HAL for OpenGL
 */

#ifndef POLARIS_ENGINE_GLRENDER_H
#define POLARIS_ENGINE_GLRENDER_H

#if !defined(USE_QT) && !defined(POLARIS_ENGINE_TARGET_ANDROID)
#include "../polarisengine.h"
#else
#include "polarisengine.h"
#endif

/* OpenGLの初期化処理を行う */
bool init_opengl(void);

/* OpenGLの終了処理を行う */
void cleanup_opengl(void);

/* フレームのレンダリングを開始する */
void opengl_start_rendering(void);

/* フレームのレンダリングを終了する */
void opengl_end_rendering(void);

/* テクスチャを更新する */
void opengl_notify_image_update(struct image *img);

/* テクスチャを破棄する */
void opengl_notify_image_free(struct image *img);

/* 画面にイメージをレンダリングする */
void opengl_render_image_normal(int dst_left,
				int dst_top,
				int dst_width,
				int dst_height,
				struct image *src_image,
				int src_top,
				int src_left,
				int src_width,
				int src_height,
				int alpha);

/* 画面にイメージをレンダリングする */
void opengl_render_image_add(int dst_left,
			     int dst_top,
			     int dst_width,
			     int dst_height,
			     struct image *src_image,
			     int src_left,
			     int src_top,
			     int src_width,
			     int src_height,
			     int alpha);

/* 画面にイメージを暗くレンダリングする */
void opengl_render_image_dim(int dst_left,
			     int dst_top,
			     int dst_width,
			     int dst_hieght,
			     struct image *src_image,
			     int src_left,
			     int src_top,
			     int src_width,
			     int src_height,
			     int alpha);

/* 画面にイメージをルール付きでレンダリングする */
void opengl_render_image_rule(struct image *src_image,
			      struct image *rule_image,
			      int threshold);

/* 画面にイメージをルール付き(メルト)でレンダリングする */
void opengl_render_image_melt(struct image *src_image,
			      struct image *rule_image,
			      int progress);

void
opengl_render_image_3d_normal(
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

void
opengl_render_image_3d_add(
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

/* 全画面表示のときのスクリーンオフセットを指定する */
void opengl_set_screen(int x, int y, int w, int h);

#endif
