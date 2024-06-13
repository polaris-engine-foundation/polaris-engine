/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

#include "polarisengine.h"

/*
 * ボリュームコマンド
 */
bool vol_command(void)
{
	const char *stream;
	float vol, span;

	stream = get_string_param(VOL_PARAM_STREAM);
	vol = get_float_param(VOL_PARAM_VOL);
	span = get_float_param(VOL_PARAM_SPAN);

	if (vol < 0 || vol > 1.0f) {
		log_script_vol_value(vol);
		log_script_exec_footer();
		return false;
	}

	if (strcmp(stream, "bgm") == 0 ||
	    strcmp(stream, "b") == 0 ||
	    strcmp(stream, "音楽") == 0) {
		/* BGMストリーム */
		set_mixer_volume(BGM_STREAM, vol, span);
	} else if (strcmp(stream, "voice") == 0 ||
		   strcmp(stream, "v") == 0 ||
		   strcmp(stream, "声") == 0) {
		/* VOICEストリーム */
		set_mixer_volume(VOICE_STREAM, vol, span);
	} else if (strcmp(stream, "se") == 0 ||
		   strcmp(stream, "s") == 0 ||
		   strcmp(stream, "効果音") == 0) {
		/* SEストリーム */
		set_mixer_volume(SE_STREAM, vol, span);
	} else if (strcmp(stream, "BGM") == 0 ||
		   strcmp(stream, "B") == 0) {
		/* BGMグローバルボリューム */
		set_mixer_global_volume(BGM_STREAM, vol);
	} else if (strcmp(stream, "VOICE") == 0 ||
		   strcmp(stream, "V") == 0) {
		/* VOICEグローバルボリューム */
		set_mixer_global_volume(VOICE_STREAM, vol);
	} else if (strcmp(stream, "SE") == 0 ||
		   strcmp(stream, "S") == 0) {
		/* SEグローバルボリューム */
		set_mixer_global_volume(SE_STREAM, vol);
	} else {
		/* Incorrect stream name */
		log_script_mixer_stream(stream);
		log_script_exec_footer();
		return false;
	}

	return move_to_next_command();
}
