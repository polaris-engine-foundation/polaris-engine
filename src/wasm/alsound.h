/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

#ifndef XENGINE_EMOPENAL_H
#define XENGINE_EMOPENAL_H

#include "../xengine.h"

/* OpenALの初期化処理を行う */
bool init_openal(void);

/* OpenALの終了処理を行う */
void cleanup_openal(void);

/* サウンドのバッファを埋める */
void fill_sound_buffer(void);

/* サウンドを一時停止する */
void pause_sound(void);

/* サウンドを再開する */
void resume_sound(void);

#endif
