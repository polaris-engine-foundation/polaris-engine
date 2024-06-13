/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * "seen" flag management
 */

#ifndef XENGINE_SEEN_H
#define XENGINE_SEEN_H

#include "types.h"

/*
 * 初期化
 */

/* 既読フラグ管理を初期化する */
bool init_seen(void);

/* 既読フラグ管理の終了処理を行う */
void cleanup_seen(void);

/*
 * 既読フラグへのアクセス
 */

/* 現在のスクリプトに対応する既読フラグをロードする */
bool load_seen(void);

/* 現在のスクリプトに対応する既読フラグをセーブする */
bool save_seen(void);

/* 現在のコマンドが既読かを返す */
bool get_seen(void);

/* 現在のコマンドを既読にする */
void set_seen(void);

#endif
