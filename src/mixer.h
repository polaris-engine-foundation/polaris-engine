/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * Audio Mixer
 */

#ifndef POLARIS_ENGINE_MIXER_H
#define POLARIS_ENGINE_MIXER_H

#include "wave.h"

/*
 * ミキサのストリーム
 */
#define MIXER_STREAMS		(4)
#define BGM_STREAM		(0)
#define VOICE_STREAM		(1)
#define SE_STREAM		(2)
#define SYS_STREAM		(3)

/*
 * キャラ別ボリュームのスロット
 */
#define CH_VOL_SLOTS		(16)
#define CH_VOL_SLOT_DEFAULT	(0)

/* ミキサーモジュールの初期化処理を行う */
void init_mixer(void);

/* ミキサーモジュールの初期化処理を行う */
void cleanup_mixer(void);

/* BGMのファイル名を設定する */
bool set_bgm_file_name(const char *file);

/* BGMのファイル名を取得する */
const char *get_bgm_file_name(void);

/* SEのファイル名を設定する(ループする場合のみ) */
bool set_se_file_name(const char *file);

/* SEのファイル名を取得する(ループ再生中の場合のみ) */
const char *get_se_file_name(void);

/* サウンドを再生・停止する */
void set_mixer_input(int n, struct wave *w);

/* ボリュームを設定する */
void set_mixer_volume(int n, float vol, float span);

/* ボリュームを取得する */
float get_mixer_volume(int n);

/* マスターボリュームを設定する */
void set_master_volume(float vol);

/* マスターボリュームを取得する */
float get_master_volume(void);

/* グローバルボリュームを設定する */
void set_mixer_global_volume(int n, float vol);

/* グローバルボリュームを取得する */
float get_mixer_global_volume(int n);

/* キャラクタボリュームを設定する */
void set_character_volume(int n, float vol);

/* キャラクタボリュームを取得する */
float get_character_volume(int n);

/* キャラクタボリュームを適用する */
void apply_character_volume(int index);

/* サウンドを再生し終わったかを取得する */
bool is_mixer_sound_finished(int n);

/* サウンドのフェード処理を実行する */
void process_sound_fading(void);

#endif
