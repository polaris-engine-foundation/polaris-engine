/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

#include "polarisengine.h"

/*
 * 章タイトル設定コマンド
 */
bool chapter_command(void)
{
	const char *param;

	param = get_string_param(CHAPTER_PARAM_NAME);

	/* 章題を設定する */
	if (!set_chapter_name(param)) {
		log_script_exec_footer();
		return false;
	}

	/* ウィンドウタイトルを更新する */
	if (!conf_window_title_chapter_disable)
		update_window_title();

	return move_to_next_command();
}
