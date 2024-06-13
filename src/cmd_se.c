/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

#include "polarisengine.h"

/*
 * seコマンド
 */
bool se_command(void)
{
	struct wave *w;
	const char *fname;
	const char *option;
	int stream;
	bool loop, stop;

	/* パラメータを取得する */
	fname = get_string_param(SE_PARAM_FILE);
	option = get_string_param(SE_PARAM_OPTION);

	/* 停止の指示かどうかチェックする */
	if (strcmp(fname, "stop") == 0 || strcmp(fname, U8("停止")) == 0)
		stop = true;
	else
		stop = false;

	/*
	 * 1. voice指示の有無を確認する
	 *  - マスターボリュームのフィードバック再生を行う際、テキスト表示なし
	 *    でボイスを再生できるようにするための指示
	 * 2. ループ再生するかを確認する
	 */
	if (strcmp(option, "voice") == 0) {
		stream = VOICE_STREAM;
		loop = false;
	} else if (strcmp(option, "loop") == 0) {
		stream = SE_STREAM;
		loop = true;
	} else {
		stream = SE_STREAM;
		loop = false;
	}

	/* 停止の指示でない場合 */
	if (!stop) {
		/* PCMストリームをオープンする */
		w = create_wave_from_file(SE_DIR, fname, loop);
		if (w == NULL) {
			log_script_exec_footer();
			return false;
		}
	} else {
		w = NULL;
	}

	/* ループ再生のときはSEファイル名を登録する */
	if (!stop && loop) {
		if (!set_se_file_name(fname))
			return false;
	} else {
		set_se_file_name(NULL);
	}

	/* 再生を開始する */
	set_mixer_input(stream, w);

	/* 次のコマンドへ移動する */
	return move_to_next_command();
}
