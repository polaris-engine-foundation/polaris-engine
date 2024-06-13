/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

#include "polarisengine.h"

/* オートモードのとき待ち時間 */
#define AUTO_MODE_WAIT	(2000)

static uint64_t sw;

/*
 * clickコマンド
 */
bool click_command(void)
{
	/* 初期化処理を行う */
	if (!is_in_command_repetition()) {
		/* メッセージボックスを非表示にする */
		show_msgbox(false);
		show_namebox(false);

		/* スキップモードを終了する */
		if (is_skip_mode()) {
			stop_skip_mode();
			show_skipmode_banner(false);
		}

		/* 時間の計測を開始する */
		reset_lap_timer(&sw);

		/* 繰り返し動作を開始する */
		start_command_repetition();
	}

	/*
	 * 入力があった場合か、オートモード中に一定時間経過したら、
	 * 繰り返し動作を終了する
	 */
	if ((!is_auto_mode() &&
	     (is_control_pressed || is_return_pressed || is_down_pressed ||
	      is_left_clicked))
	    ||
	    (is_auto_mode() &&
	     (float)get_lap_timer_millisec(&sw) >= AUTO_MODE_WAIT)) {
		stop_command_repetition();
	} else {
#if defined(USE_EDITOR)
		if (dbg_is_stop_requested())
			stop_command_repetition();
#endif
	}

	/* ステージの描画を行う */
	render_stage();

	/* 後処理を行う */
	if (!is_in_command_repetition()) {
		/* 次のコマンドへ移動する */
		return move_to_next_command();
	}

	/* コマンドの実行を継続する */
	return true;
}
