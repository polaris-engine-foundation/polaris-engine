/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * The game loop handler
 */

#ifndef XENGINE_MAIN_H
#define XENGINE_MAIN_H

#include "types.h"

/*
 * GUIファイル
 */

/* コンフィグのGUIファイル */
#define CONFIG_GUI_FILE			"system.txt"
#define COMPAT_CONFIG_GUI_FILE		"system.gui"

/* セーブGUIファイル */
#define SAVE_GUI_FILE			"save.txt"
#define COMPAT_SAVE_GUI_FILE		"save.gui"

/* ロードGUIファイル */
#define LOAD_GUI_FILE			"load.txt"
#define COMPAT_LOAD_GUI_FILE		"load.gui"

/* ヒストリのGUIファイル */
#define HISTORY_GUI_FILE		"history.txt"
#define COMPAT_HISTORY_GUI_FILE		"history.gui"

/* custom1のGUIファイル */
#define CUSTOM1_GUI_FILE		"custom1.txt"
#define COMPAT_CUSTOM1_GUI_FILE		"custom1.gui"

/* custom2のGUIファイル */
#define CUSTOM2_GUI_FILE		"custom2.txt"
#define COMPAT_CUSTOM2_GUI_FILE		"custom2.gui"

/*
 * 入力の状態
 */
extern bool is_left_button_pressed;
extern bool is_right_button_pressed;
extern bool is_left_clicked;
extern bool is_right_clicked;
extern bool is_return_pressed;
extern bool is_space_pressed;
extern bool is_escape_pressed;
extern bool is_up_pressed;
extern bool is_down_pressed;
extern bool is_left_arrow_pressed;
extern bool is_right_arrow_pressed;
extern bool is_page_up_pressed;
extern bool is_page_down_pressed;
extern bool is_control_pressed;
extern bool is_s_pressed;
extern bool is_l_pressed;
extern bool is_h_pressed;
extern int mouse_pos_x;
extern int mouse_pos_y;
extern bool is_mouse_dragging;
extern bool is_touch_canceled;
extern bool is_swiped;

void clear_input_state(void);

/*
 * ゲームループの中身
 */

void init_game_loop(void);
bool game_loop_iter(void);
void cleanup_game_loop(void);

/*
 * コマンドの実装
 */

bool message_command(bool *cont);
bool bg_command(void);
bool bgm_command(void);
bool ch_command(void);
bool click_command(void);
bool wait_command(void);
bool goto_command(bool *cont);
bool load_command(void);
bool vol_command(void);
bool set_command(void);
bool if_command(void);
bool select_command(void);
bool se_command(void);
bool switch_command(void);
bool gosub_command(void);
bool return_command(bool *cont);
bool cha_command(void);
bool shake_command(void);
bool setsave_command(void);
bool chs_command(void);
bool video_command(void);
bool skip_command(void);
bool chapter_command(void);
bool gui_command(void);
bool wms_command(void);
bool anime_command(void);
bool pencil_command(void);
bool setconfig_command(void);
bool layer_command(void);
bool ciel_command(bool *cont);
bool addmsg_command(void);

/*
 * 複数のイテレーションに渡るコマンドの実行中であるかの設定
 */

void start_command_repetition(void);
void stop_command_repetition(void);
bool is_in_command_repetition(void);

/*
 * メッセージの表示中状態の設定
 *  - メッセージ表示中にシステムGUIに移動した場合、「表示中」状態が保持される
 *  - メッセージコマンドから次のコマンドに進むときにクリアされる
 *  - ロードされてもクリアされる
 */

void set_message_active(void);
void clear_message_active(void);
bool is_message_active(void);

/*
 * オートモードの設定
 */

void start_auto_mode(void);
void stop_auto_mode(void);
bool is_auto_mode(void);

/*
 * スキップモードの設定
 */

void start_skip_mode(void);
void stop_skip_mode(void);
bool is_skip_mode(void);

/*
 * セーブ・ロード画面の許可の設定
 */

void set_save_load(bool enable);
bool is_save_load_enabled(void);

/*
 * 割り込み不可モードの設定
 */

void set_non_interruptible(bool mode);
bool is_non_interruptible(void);

/*
 * ペンの位置(@ichoose用)
 */

void set_pen_position(int x, int y);
int get_pen_position_x(void);
int get_pen_position_y(void);

/*
 * マクロ/WMS/アニメ引数
 */

bool set_call_argument(int index, const char *val);
const char *get_call_argument(int index);

/*
 * スクリプトのページモード
 */

bool is_page_mode(void);
bool append_buffered_message(const char *msg);
const char *get_buffered_message(void);
void clear_buffered_message(void);
void reset_page_line(void);
void inc_page_line(void);
bool is_page_top(void);

/*
 * Editing/Debugging Support
 */
#ifdef USE_EDITOR

/*
 * Stop execution and update debugger UI elements.
 */
void dbg_stop(void);

/*
 * Return whether a stop request is pending.
 *  - This is for:
 *    - Stopping execution on command moves and script loads in script.c
 *    - Specia message displaying behaviour in cmd_message.c
 */
bool dbg_is_stop_requested(void);

/*
 * Set the runtime error state.
 */
void dbg_raise_runtime_error(void);

/*
 * Reset the error count.
 */
void dbg_reset_parse_error_count(void);

/*
 * Increment the error count.
 */
void dbg_increment_parse_error_count(void);

/*
 * Get the error count.
 */
int dbg_get_parse_error_count(void);

#endif	/* USE_EDITOR */

#endif	/* XENGINE_MAIN_H */
