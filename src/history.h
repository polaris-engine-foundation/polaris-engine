/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * History management (to be moved to main.c)
 */

#ifndef XENGINE_HISTORY_H
#define XENGINE_HISTORY_H

#include "types.h"

/* ヒストリに関する初期化処理を行う */
bool init_history(void);

/* ヒストリに関する終了処理を行う */
void cleanup_history(void);

/* メッセージを登録する */
bool register_message(const char *name, const char *msg, const char *voice,
		      pixel_t body_color, pixel_t body_outline_color,
		      pixel_t name_color, pixel_t name_outline_color);

/* メッセージを末尾に追記する */
bool append_message(const char *msg);

/* ヒストリをクリアする(ロード時) */
void clear_history(void);

/* ヒストリの数を取得する */
int get_history_count(void);

/* ヒストリを取得する */
const char *get_history_message(int index);

/* ヒストリのボイスを取得する */
const char *get_history_voice(int index);

/* セリフがカッコで始まりカッコで終わるかチェックする */
bool is_quoted_serif(const char *msg);

/* セリフがカッコで始まるかチェックする */
bool is_quote_started(const char *msg);

/* セリフがカッコで終わるかチェックする */
bool is_quote_ended(const char *msg);

#endif
