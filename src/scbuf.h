/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * Scnan conversion buffer
 * 走査変換バッファ
 */

#ifndef XENGINE_SCBUF_H
#define XENGINE_SCBUF_H

#include "types.h"

/*
 * 初期化
 */

/* 走査変換バッファの初期化処理をする */
bool init_scbuf(void);

/* 走査変換バッファの終了処理を行う */
void cleanup_scbuf(void);

/*
 * 走査変換
 */

/* 走査変換バッファをクリアする */
void clear_scbuf(void);

/* エッジをスキャンして最小値を設定する */
void scan_edge_min(int x1, int y1, int x2, int y2);

/* エッジをスキャンして最大値を設定する */
void scan_edge_max(int x1, int y1, int x2, int y2);

/* 指定した走査線の最小値と最大値を取得する */
void get_scan_line(int y, int *min, int *max);

#endif /* XENGINE_SCBUF_H */
