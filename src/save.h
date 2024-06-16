/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * Save data management.
 */

#ifndef POLARIS_ENGINE_SAVE_H
#define POLARIS_ENGINE_SAVE_H

#include "types.h"

/*
 * グローバルセーブファイル
 */
#define GLOBAL_SAVE_FILE	"g000.sav"

/*
 * クイックセーブファイル
 */
#define QUICK_SAVE_FILE		"q000.sav"

/*
 * クイックセーブファイル(extra)
 */
#define QUICK_SAVE_EXTRA_FILE	"q001.sav"

/*
 * セーブデータ数
 */
#define SAVE_SLOTS		(100)

/* セーブデータに関する初期化処理を行う */
bool init_save(void);

/* セーブデータに関する終了処理を行う */
void cleanup_save(void);

/* ロードが終了した直後であるかを調べる */
bool check_load_flag(void);

/* セーブを実行する */
bool execute_save(int index);

/* ロードを実行する */
bool execute_load(int index);

/* グローバルデータを保存する */
void save_global_data(void);

/* クイックセーブデータがあるか */
bool have_quick_save_data(void);

/* クイックセーブを行う */
bool quick_save(bool extra);

/* クイックロードを行う */
bool quick_load(bool extra);

/* ローカルセーブデータの削除を行う */
void delete_local_save(int index);

/* グローバルセーブデータの削除を行う */
void delete_global_save(void);

/* セーブデータの日付を取得する */
time_t get_save_date(int index);

/* 最新のセーブデータの番号を取得する */
int get_latest_save_index(void);

/* セーブデータの章タイトルを取得する */
const char *get_save_chapter_name(int index);

/* セーブデータの最後のメッセージを取得する */
const char *get_save_last_message(int index);

/* セーブデータのサムネイルを取得する */
struct image *get_save_thumbnail(int index);

/* 章題を設定する */
bool set_chapter_name(const char *name);

/* 章題を取得する */
const char *get_chapter_name(void);

/* 最後のメッセージを設定する */
bool set_last_message(const char *msg, bool is_append);

/* テキストレイヤのテキストを設定する */
bool set_layer_text(int text_layer_index, const char *msg);

/* テキストスピードを設定する */
void set_text_speed(float val);

/* テキストスピードを取得する */
float get_text_speed(void);

/* オートスピードを設定する */
void set_auto_speed(float val);

/* オートスピードを取得する */
float get_auto_speed(void);

/* 最後の+en+コマンドの位置を記録する */
void set_last_en_command(void);

/* 最後の+en+コマンドの位置を消去する */
void clear_last_en_command(void);

/* ロード直後のメッセージボックスの内容を取得する */
char *get_pending_message(void);

#endif
