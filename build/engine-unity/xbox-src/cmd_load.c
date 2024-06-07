/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

#include "xengine.h"

/*
 * loadコマンド
 */
bool load_command(void)
{
	char *file, *label;

	/* パラメータからファイル名を取得する */
	file = strdup(get_string_param(LOAD_PARAM_FILE));
	if (file == NULL) {
		log_memory();
		return false;
	}

	/* パラメータからラベル名を取得する */
	label = strdup(get_string_param(LOAD_PARAM_LABEL));
	if (label == NULL) {
		log_memory();
		return false;
	}

	/* 既読フラグをセーブする */
	save_seen();

	/* スクリプトをロードする */
	if (!load_script(file)) {
		free(file);
		return false;
	}

	/* ラベルへジャンプする */
	if (strcmp(label, "") != 0) {
		if (!move_to_label(label)) {
			free(file);
			free(label);
			return false;
		}
	}

	free(file);
	free(label);

	return true;
}
