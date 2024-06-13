/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

#include "polarisengine.h"

/* CGディレクトリ */
#define CG_DIR_PREFIX "cg/"

/* コマンドの経過時刻を表すストップウォッチ */
static uint64_t sw;

/* コマンドの長さ(秒) */
static float span;

/*
 * 前方参照
 */
static bool init(void);
static void draw(void);
static bool cleanup(void);

/*
 * bgコマンド
 */
bool bg_command(void)
{
	if (!is_in_command_repetition())
		if (!init())
			return false;

	draw();

	if (!is_in_command_repetition())
		if (!cleanup())
			return false;

	return true;
}

/* 初期化処理を行う */
static bool init(void)
{
	static struct image *img, *rule_img;
	const char *fname, *method;
	int fade_method, ofs_x, ofs_y;

	/* パラメータを取得する */
	fname = get_string_param(BG_PARAM_FILE);
	span = get_float_param(BG_PARAM_SPAN);
	method = get_string_param(BG_PARAM_METHOD);
	ofs_x = get_int_param(BG_PARAM_X);
	ofs_y = get_int_param(BG_PARAM_Y);

 	/* 描画メソッドを識別する */
	fade_method = get_fade_method(method);
	if (fade_method == FADE_METHOD_INVALID) {
		log_script_fade_method(method);
		log_script_exec_footer();
		return false;
	}

	/* ルールが使用される場合 */
	if (fade_method == FADE_METHOD_RULE ||
	    fade_method == FADE_METHOD_MELT) {
		/* ルールファイルが指定されていない場合 */
		if (strcmp(&method[5], "") == 0) {
			log_script_rule();
			log_script_exec_footer();
			return false;
		}

		/* イメージを読み込む */
		rule_img = create_image_from_file(RULE_DIR, &method[5]);
		if (rule_img == NULL) {
			log_script_exec_footer();
			return false;
		}
	} else {
		rule_img = NULL;
	}

	/* 色指定の場合 */
	if (fname[0] == '#') {
		/* 色を指定してイメージを作成する */
		img = create_image_from_color_string(conf_window_width,
						     conf_window_height,
						     &fname[1]);
	} else {
		if (strncmp(fname, CG_DIR_PREFIX, strlen(CG_DIR_PREFIX)) == 0) {
			/* cgからイメージを読み込む */
			img = create_image_from_file(CG_DIR, &fname[strlen(CG_DIR_PREFIX)]);
		} else {
			/* bgからイメージを読み込む */
			img = create_image_from_file(BG_DIR, fname);
		}
	}
	if (img == NULL) {
		log_script_exec_footer();
		return false;
	}

	/* 発話中のキャラをなしにする */
	set_ch_talking(-1);

	/* メッセージボックスを瞬時に消す (msgbox.show.on.bg=2) */
	if (conf_msgbox_show_on_bg == 2) {
		show_namebox(false);
		show_msgbox(false);
	}

	/* 背景フェードを開始する */
	if (!start_fade_for_bg(fname, img, ofs_x, ofs_y, 255, fade_method,
			       rule_img)) {
		log_script_exec_footer();
		return false;
	}

	/* メッセージボックスをフェードで消す (msgbox.show.on.bg=0) */
	if (conf_msgbox_show_on_bg == 0) {
		show_namebox(false);
		show_msgbox(false);
	}

	/* 繰り返し動作を開始する */
	start_command_repetition();

	/* 時間計測を開始する */
	reset_lap_timer(&sw);

	return true;
}

/* 描画を行う */
static void draw(void)
{
	float lap;

	/* 経過時間を取得する */
	lap = (float)get_lap_timer_millisec(&sw) / 1000.0f;
	if (lap >= span)
		lap = span;

	/* 入力に反応する */
	if (is_auto_mode() &&
	    (is_control_pressed || is_return_pressed ||
	     is_left_clicked || is_down_pressed)) {
		/* 入力によりオートモードを終了する */
		stop_auto_mode();
		show_automode_banner(false);

		/* 繰り返し動作を停止する */
		stop_command_repetition();

		/* フェードを完了する */
		finish_fade();
	} else if (is_skip_mode() &&
		   (is_control_pressed || is_return_pressed ||
		    is_left_clicked || is_down_pressed)) {
		/* 入力によりスキップモードを終了する */
		stop_skip_mode();
		show_skipmode_banner(false);

		/* 繰り返し動作を停止する */
		stop_command_repetition();

		/* フェードを完了する */
		finish_fade();
	} else if ((lap >= span)
		   ||
		   is_skip_mode()
		   ||
		   (!is_non_interruptible() &&
		    (is_control_pressed || is_return_pressed ||
		     is_left_clicked || is_down_pressed))) {
		/*
		 * 経過時間が一定値を超えた場合と、
		 * スキップモードの場合と、
		 * 入力により省略された場合
		 */

		/* 繰り返し動作を停止する */
		stop_command_repetition();

		/* フェードを完了する */
		finish_fade();

		/* 入力の場合はスキップモードとオートモードを終了する */
		if (is_control_pressed || is_return_pressed ||
		    is_left_clicked || is_down_pressed) {
			if (is_skip_mode()) {
				stop_skip_mode();
				show_skipmode_banner(false);
			} else if (is_auto_mode()) {
				stop_auto_mode();
				show_automode_banner(false);
			}
		}
	} else {
		/* フェーディングを行う */
		set_fade_progress(lap / span);
	}

	/* ステージを描画する */
	if (is_in_command_repetition())
		render_fade();
	else
		render_stage();

	/* 折りたたみシステムメニューを描画する */
	if (conf_sysmenu_transition && !is_non_interruptible())
		render_collapsed_sysmenu(false);
}

/* 終了処理を行う */
static bool cleanup(void)
{
	/* 次のコマンドに移動する */
	if (!move_to_next_command())
		return false;

	return true;
}
