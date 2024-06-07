/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * Event handlers that are called from HALs
 */
 
#include "xengine.h"

/* False assertion */
#define INVALID_KEYCODE	(0)

/*
 * プラットフォーム非依存な初期化処理を行う
 */
bool on_event_init(void)
{
	srand((unsigned int)time(NULL));

	/* 変数の初期化処理を行う */
	init_vars();

	/* 文字レンダリングエンジンの初期化処理を行う */
	if (!init_glyph())
		return false;

	/* ミキサの初期化処理を行う */
	init_mixer();

	/* セーブデータの初期化処理を行う */
	if (!init_save())
		return false;

	/* ステージの初期化処理を行う */
	if (!init_stage())
		return false;

	/* 走査変換バッファの初期化処理を行う */
	if (!init_scbuf())
		return false;

	/* 初期スクリプトをロードする */
	if (!init_script())
		return false;

	/* 既読フラグをロードする */
	if (!init_seen())
		return false;

	/* GUIを初期化する */
	if (!init_gui())
		return false;

	/* ゲームループの初期化処理を行う */
	init_game_loop();

	return true;
}

/*
 * プラットフォーム非依存な終了処理を行う
 */
void on_event_cleanup(void)
{
	/* ゲームループの終了処理を行う */
	cleanup_game_loop();

	/* GUIの終了処理を行う */
	cleanup_gui();

	/* 既読フラグ管理の終了処理を行う */
	cleanup_seen();

	/* スクリプトの終了処理を行う */
	cleanup_script();

	/* デーブデータ関連の終了処理を行う */
	cleanup_save();

	/* 走査変換バッファの終了処理を行う */
	cleanup_scbuf();

	/* ステージの終了処理を行う */
	cleanup_stage();

	/* ミキサの終了処理を行う */
	cleanup_mixer();

	/* 文字レンダリングエンジンの終了処理を行う */
	cleanup_glyph();

	/* 変数の終了処理を行う */
	cleanup_vars();
}

/*
 * 再描画時に呼び出される
 */
bool on_event_frame(void)
{
	/* ゲームループの中身を実行する */
	if (!game_loop_iter()) {
		/* アプリケーションを終了する */
		return false;
	}

	/* キラキラエフェクトを描画する */
	if (conf_kirakira_on)
		render_kirakira();

	/* アプリケーションを続行する */
	return true;
}

/*
 * キー押下時に呼び出される
 */
void on_event_key_press(int key)
{
	switch(key) {
	case KEY_CONTROL:
		is_control_pressed = true;
		break;
	case KEY_SPACE:
		is_space_pressed = true;
		break;
	case KEY_RETURN:
		is_return_pressed = true;
		break;
	case KEY_UP:
		is_up_pressed = true;
		break;
	case KEY_DOWN:
		is_down_pressed = true;
		break;
	case KEY_LEFT:
		is_left_arrow_pressed = true;
		break;
	case KEY_RIGHT:
		is_right_arrow_pressed = true;
		break;
	case KEY_ESCAPE:
		is_escape_pressed = true;
		break;
	case KEY_S:
		is_s_pressed = true;
		break;
	case KEY_L:
		is_l_pressed = true;
		break;
	case KEY_H:
		is_h_pressed = true;
		break;
	default:
		assert(INVALID_KEYCODE);
		break;
	}
}

/*
 * キー解放時に呼び出される
 */
void on_event_key_release(int key)
{
	switch(key) {
	case KEY_CONTROL:
		is_control_pressed = false;
		break;
	case KEY_SPACE:
		is_space_pressed = false;
		break;
	case KEY_RETURN:	/* fall-thru */
	case KEY_UP:		/* fall-thru */
	case KEY_DOWN:		/* fall-thru */
	case KEY_LEFT:		/* fall-thru */
	case KEY_RIGHT:		/* fall-thru */
	case KEY_ESCAPE:	/* fall-thru */
	case KEY_S:		/* fall-thru */
	case KEY_L:		/* fall-thru */
	case KEY_H:
		/* これらのキーはフレームごとに解放されたことにされる */
		break;
	case KEY_C:
		/* このキーには解放処理を行わない */
		break;
	default:
		assert(0);
		break;
	}
}

/*
 * マウス押下時に呼び出される
 */
void on_event_mouse_press(int button, int x, int y)
{
	if (x < 0 || x >= conf_window_width || y < 0 || y >= conf_window_height)
		return;

	mouse_pos_x = x;
	mouse_pos_y = y;

	if (button == MOUSE_LEFT) {
		is_left_button_pressed = true;
		is_mouse_dragging = true;
	} else {
		is_right_button_pressed = true;
	}

	if (conf_kirakira_on)
		start_kirakira(x, y);
}

/*
 * マウス解放時に呼び出される
 */
void on_event_mouse_release(int button, int x, int y)
{
	is_left_button_pressed = false;
	is_right_button_pressed = false;
	is_mouse_dragging = false;

	if (x < 0 || x >= conf_window_width || y < 0 || y >= conf_window_height)
		return;

	mouse_pos_x = x;
	mouse_pos_y = y;

	if (button == MOUSE_LEFT)
		is_left_clicked = true;
	else
		is_right_clicked = true;
}

/*
 * マウス移動時に呼び出される
 */
void on_event_mouse_move(int x, int y)
{
	if (x < 0 || x >= conf_window_width || y < 0 || y >= conf_window_height)
		return;

	mouse_pos_x = x;
	mouse_pos_y = y;
}

/*
 * タッチキャンセル時に呼び出される
 */
void on_event_touch_cancel(void)
{
	is_left_button_pressed = false;
	is_right_button_pressed = false;
	is_mouse_dragging = false;
	is_left_clicked = false;
	is_right_clicked = false;
	is_touch_canceled = true;
}

/*
 * 下スワイプ時に呼び出される
 */
void on_event_swipe_down(void)
{
	is_down_pressed = true;
	is_swiped = true;
}

/*
 * 上スワイプ時に呼び出される
 */
void on_event_swipe_up(void)
{
	is_up_pressed = true;
	is_swiped = true;
}
