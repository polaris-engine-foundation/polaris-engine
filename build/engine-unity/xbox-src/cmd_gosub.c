/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

#include "xengine.h"

/*
 * gosubコマンド
 */
bool gosub_command(void)
{
	const char *label;

	/* パラメータを取得する */
	label = get_string_param(GOSUB_PARAM_LABEL);
	set_call_argument(0, get_string_param(GOSUB_PARAM_ARG1));
	set_call_argument(1, get_string_param(GOSUB_PARAM_ARG2));
	set_call_argument(2, get_string_param(GOSUB_PARAM_ARG3));
	set_call_argument(3, get_string_param(GOSUB_PARAM_ARG4));
	set_call_argument(4, get_string_param(GOSUB_PARAM_ARG5));
	set_call_argument(5, get_string_param(GOSUB_PARAM_ARG6));
	set_call_argument(6, get_string_param(GOSUB_PARAM_ARG7));
	set_call_argument(7, get_string_param(GOSUB_PARAM_ARG8));
	set_call_argument(8, get_string_param(GOSUB_PARAM_ARG9));

	/* リターンポイントを記録する */
	push_return_point();

	/* ラベルの次のコマンドへ移動する */
	if (!move_to_label(label))
		return false;

	return true;
}
