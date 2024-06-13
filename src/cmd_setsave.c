/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

#include "polarisengine.h"

/*
 * セーブ許可コマンド
 */
bool setsave_command(void)
{
	const char *param;

	param = get_string_param(SETSAVE_PARAM_MODE);

	if (strcmp(param, "disable") == 0) {
		set_save_load(false);
	} else if (strcmp(param, "enable") == 0) {
		set_save_load(true);
	} else {
		log_script_enable_disable(param);
		log_script_exec_footer();
		return false;
	}

	return move_to_next_command();
}
