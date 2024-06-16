/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * 選択肢系コマンドの実装
 *  - @choose ... 通常の選択肢
 *  - @ichoose ... 全画面ノベル用のインライン選択肢
 *  - @mchoose ... 条件つき選択肢
 *  - @michoose ... 条件つきインライン選択肢
 */

/*
 * [Memo]
 *  - システムメニューの非表示時には、折りたたみシステムメニューが表示される
 *  - コンフィグのsysmenu.hidden=1のとき、
 *    - メッセージコマンドでは折りたたみシステムメニューが表示されない
 *    - 選択肢コマンドでは折りたたみシステムメニューが表示される
 *    - 理由は選択肢でのセーブ・ロードの手段がなくなるから
 *  - sysmenu.hidden=2なら、選択肢コマンドでも折りたたみシステムメニューを表示しない
 */

#include "polarisengine.h"

/* false assertion */
#define ASSERT_INVALID_BTN_INDEX (0)

/*
 * ボタンの最大数
 */
#define CHOOSE_COUNT		(10)

/*
 * 引数インデックスの計算
 *  - コマンドの種類によって変わる
 */

/* @chooseのラベルの引数インデックス */
#define CHOOSE_LABEL(n)			(CHOOSE_PARAM_LABEL1 + n * 2)

/* @choose/@ichooseのメッセージの引数インデックス */
#define CHOOSE_MESSAGE(n)		(CHOOSE_PARAM_LABEL1 + n * 2 + 1)

/* @mchoose/@michooseのラベルの引数インデックス */
#define MCHOOSE_LABEL(n)		(MCHOOSE_PARAM_LABEL1 + n * 3)

/* @mchoose/@michooseの変数の引数インデックス */
#define MCHOOSE_VAR(n)			(MCHOOSE_PARAM_LABEL1 + n * 3 + 1)

/* @mchoose/@michooseのメッセージの引数インデックス */
#define MCHOOSE_MESSAGE(n)		(MCHOOSE_PARAM_LABEL1 + n * 3 + 2)

/*
 * 選択肢の項目
 */

/* 選択肢のボタン */
static struct choose_button {
	const char *msg;
	const char *label;
	int x;
	int y;
	int w;
	int h;
	struct image *img_idle;
	struct image *img_hover;
} choose_button[CHOOSE_COUNT];

/*
 * 選択肢の状態
 */

/* ポイントされている項目のインデックス */
static int pointed_index;

/* キー操作によってポイントが変更されたか */
static bool is_selected_by_key;

/* キー操作によってポイントが変更されたときのマウス座標 */
static int save_mouse_pos_x, save_mouse_pos_y;

/* このコマンドを無視するか */
static bool ignore_as_no_options;

/* 最初のフレームでマウスオーバーのSEを回避するフラグ */
static bool is_first_frame;

/*
 * 描画の状態
 */

/* センタリングするか */
static bool is_centered;

/*
 * システムメニュー
 */

/* システムメニューを表示中か */
static bool is_sysmenu;

/* システムメニューの最初のフレームか */
static bool is_sysmenu_first_frame;

/* システムメニューが終了した直後か */
static bool is_sysmenu_finished;

/* システムメニューのどのボタンがポイントされているか */
static int sysmenu_pointed_index;

/* システムメニューのどのボタンがポイントされていたか */
static int old_sysmenu_pointed_index;

/* 折りたたみシステムメニューが前のフレームでポイントされていたか */
static bool is_collapsed_sysmenu_pointed_prev;

/*
 * システム遷移フラグ
 */

/* クイックセーブを行うか */
static bool will_quick_save;

/* クイックロードを行ったか */
static bool did_quick_load;

/* セーブモードに遷移するか */
static bool need_save_mode;

/* ロードモードに遷移するか */
static bool need_load_mode;

/* ヒストリモードに遷移するか */
static bool need_history_mode;

/* コフィグモードに遷移するか */
static bool need_config_mode;

/* カスタム1に遷移するか */
static bool need_custom1_mode;

/* カスタム2に遷移するか */
static bool need_custom2_mode;

/* カスタム1/2でgosubを使うときの飛び先 */
static const char *custom_gosub_target;

/* クイックロードに失敗したか */
static bool is_quick_load_failed;

/*
 * 時間制限
 */
static bool is_timed;
static bool is_bombed;
static uint64_t bomb_sw;

/*
 * 前方参照
 */

/* 主な処理 */
static void pre_process(void);
static bool blit_process(void);
static void render_process(void);
static bool post_process(void);

/* 初期化 */
static bool init(void);
static bool init_choose(void);
static bool init_ichoose(void);
static int init_mchoose(void);
static int init_michoose(void);
static void draw_text(struct image *target, const char *text, int w, int h, bool is_bg);

/* 入力処理 */
static void process_main_input(void);
static int get_pointed_index(void);
static void process_sysmenu_input(void);
static int get_pointed_sysmenu_item_extended(void);

/* 描画 */
static void render_frame(void);
static void render_sysmenu_extended(void);
static void render_collapsed_sysmenu_extended(void);

/* その他 */
static void play_se(const char *file);
static void run_anime(int unfocus_index, int focus_index);

/* クリーンアップ */
static bool cleanup(void);

/*
 * switchコマンド
 */
bool switch_command(void)
{
	/* 初期化処理を行う */
	if (!is_in_command_repetition())
		if (!init())
			return false;

	/* 選択肢がなかった場合 */
	if (ignore_as_no_options)
		return move_to_next_command();

	pre_process();
	if (is_quick_load_failed)
		return false;
	blit_process();
	render_process();
	if (!post_process())
		return false;

	/* 終了処理を行う */
	if (!is_in_command_repetition())
		if (!cleanup())
			return false;

	return true;
}

static void pre_process(void)
{
	if (is_timed) {
		if ((float)get_lap_timer_millisec(&bomb_sw) >= conf_switch_timed * 1000.0f) {
			is_bombed = true;
			return;
		}
	}

	/* システムメニューが表示されていない場合 */
	if (!is_sysmenu) {
		process_main_input();
		return;
	}

	/* システムメニューが表示されている場合 */
	process_sysmenu_input();
}

static bool blit_process(void)
{
	int i;

	if (is_bombed)
		return true;

	/*
	 * 必要な場合はステージのサムネイルを作成する
	 *  - クイックセーブされるとき
	 *  - システムGUIに遷移するとき
	 */
	if (will_quick_save
	    ||
	    (need_save_mode || need_load_mode || need_history_mode ||
	     need_config_mode)) {
		draw_stage_to_thumb();
		for (i = 0; i < CHOOSE_COUNT; i++) {
			if (choose_button[i].img_idle == NULL)
				continue;
			draw_switch_to_thumb(choose_button[i].img_idle,
					     choose_button[i].x,
					     choose_button[i].y);
		}
	}

	if (need_save_mode) {
		if (check_file_exist(GUI_DIR, SAVE_GUI_FILE)) {
			if (!prepare_gui_mode(SAVE_GUI_FILE, true))
				return false;
		} else {
			if (!prepare_gui_mode(COMPAT_SAVE_GUI_FILE, true))
				return false;
		}
		set_gui_options(true, false, false);
		start_gui_mode();
	} else if (need_load_mode) {
		if (check_file_exist(GUI_DIR, LOAD_GUI_FILE)) {
			if (!prepare_gui_mode(LOAD_GUI_FILE, true))
				return false;
		} else {
			if (!prepare_gui_mode(COMPAT_LOAD_GUI_FILE, true))
				return false;
		}
		set_gui_options(true, false, false);
		start_gui_mode();
	} else if (need_history_mode) {
		if (check_file_exist(GUI_DIR, HISTORY_GUI_FILE)) {
			if (!prepare_gui_mode(HISTORY_GUI_FILE, true))
				return false;
		} else {
			if (!prepare_gui_mode(COMPAT_HISTORY_GUI_FILE, true))
				return false;
		}
		set_gui_options(true, false, false);
		start_gui_mode();
	} else if (need_config_mode) {
		if (check_file_exist(GUI_DIR, CONFIG_GUI_FILE)) {
			if (!prepare_gui_mode(CONFIG_GUI_FILE, true))
				return false;
		} else {
			if (!prepare_gui_mode(COMPAT_CONFIG_GUI_FILE, true))
				return false;
		}
		set_gui_options(true, false, false);
		start_gui_mode();
	} else if (need_custom1_mode) {
		if (conf_sysmenu_custom1_gosub == NULL)  {
			if (check_file_exist(GUI_DIR, CUSTOM1_GUI_FILE)) {
				if (!prepare_gui_mode(CUSTOM1_GUI_FILE, false))
					return false;
			} else {
				if (!prepare_gui_mode(COMPAT_CUSTOM1_GUI_FILE, false))
					return false;
			}
			set_gui_options(true, false, false);
			start_gui_mode();
		} else {
			custom_gosub_target = conf_sysmenu_custom1_gosub;
		}
	} else if (need_custom2_mode) {
		if (conf_sysmenu_custom2_gosub == NULL)  {
			if (check_file_exist(GUI_DIR, CUSTOM2_GUI_FILE)) {
				if (!prepare_gui_mode(CUSTOM2_GUI_FILE, false))
					return false;
			} else {
				if (!prepare_gui_mode(COMPAT_CUSTOM2_GUI_FILE, false))
					return false;
			}
			set_gui_options(true, false, false);
			start_gui_mode();
		} else {
			custom_gosub_target = conf_sysmenu_custom2_gosub;
		}
	}

	return true;
}

static void render_process(void)
{
	/* クイックロードされた場合 */
	if (did_quick_load)
		return;

	/* GUIへ遷移する場合、メッセージコマンドのレンダリングではなくGUIのレンダリングを行う */
	if (is_sysmenu_finished &&
	    (need_save_mode || need_load_mode || need_history_mode ||
	     need_config_mode || need_custom1_mode || need_custom2_mode)) {
		run_gui_mode();
		is_sysmenu_finished = false;
		return;
	}

	/* レンダリングを行う */
	render_frame();

	/* システムメニューの表示完了直後のフラグをクリアする */
	is_sysmenu_finished = false;
}

static bool post_process(void)
{
	/* システムメニューで押されたボタンの処理を行う */
	if (will_quick_save) {
		quick_save(false);
		will_quick_save = false;
	}

	/*
	 * 必要な場合は繰り返し動作を停止する
	 *  - クイックロードされたとき
	 *  - 時間制限に達したとき
	 *  - システムGUIに遷移するとき
	 */
	if (did_quick_load
	    ||
	    is_bombed
	    ||
	    (need_save_mode || need_load_mode || need_history_mode || need_config_mode || need_custom1_mode || need_custom2_mode))
		stop_command_repetition();

	is_first_frame = false;

	return true;
}

/*
 * 初期化
 */

/* コマンドの初期化処理を行う */
bool init(void)
{
	int type;

	pointed_index = -1;

	ignore_as_no_options = false;

	is_centered = true;

	is_sysmenu = false;
	is_sysmenu_first_frame = false;
	is_sysmenu_finished = false;
	is_collapsed_sysmenu_pointed_prev = false;

	will_quick_save = false;
	did_quick_load = false;
	need_save_mode = false;
	need_load_mode = false;
	need_history_mode = false;
	need_config_mode = false;
	need_custom1_mode = false;
	need_custom2_mode = false;
	is_quick_load_failed = false;

	is_first_frame = true;

	is_timed = false;
	is_bombed = false;
	reset_lap_timer(&bomb_sw);

	/* Android NDKの場合、画像を破棄する */
#ifdef POLARIS_ENGINE_TARGET_ANDROID
	for (int i = 0; i < CHOOSE_COUNT; i++) {
		if (parent_button[i].img_idle != NULL)
			destroy_image(parent_button[i].img_idle);
		if (parent_button[i].img_hover != NULL)
			destroy_image(parent_button[i].img_hover);
		parent_button[i].img_idle = NULL;
		parent_button[i].img_hover = NULL;
		for (int j = 0; j < CHILD_COUNT; j++) {
			if (child_button[i][j].img_idle != NULL)
				destroy_image(parent_button[i].img_idle);
			if (child_button[i][j].img_hover != NULL)
				destroy_image(parent_button[i].img_hover);
		}
	}
#endif

	/* コマンドの種類ごとに初期化を行う */
	type = get_command_type();
	switch (type) {
	case COMMAND_CHOOSE:
		if (!init_choose())
			return false;
		break;
	case COMMAND_ICHOOSE:
		if (!init_ichoose())
			return false;
		break;
	case COMMAND_MCHOOSE:
		switch (init_mchoose()) {
		case -1:
			/* エラー */
			return false;
		case 0:
			/* 表示する項目がない */
			ignore_as_no_options = true;
			return true;
		default:
			/* 表示を行う */
			break;
		}
		break;
	case COMMAND_MICHOOSE:
		switch (init_michoose()) {
		case -1:
			/* エラー */
			return false;
		case 0:
			/* 表示する項目がない */
			ignore_as_no_options = true;
			return true;
		default:
			/* 表示を行う */
			break;
		}
		break;
	default:
		assert(0);
		break;
	}

	start_command_repetition();

	/* 名前ボックス、メッセージボックスを非表示にする */
	if (!conf_msgbox_show_on_choose) {
		show_namebox(false);
		if (type == COMMAND_ICHOOSE || type == COMMAND_MICHOOSE)
			show_msgbox(true);
		else
			show_msgbox(false);
	}
	show_click(false);

	/* オートモードを終了する */
	if (is_auto_mode()) {
		stop_auto_mode();
		show_automode_banner(false);
	}

	/* スキップモードを終了する */
	if (is_skip_mode()) {
		stop_skip_mode();
		show_skipmode_banner(false);
	}

	/* 時間制限設定を行う */
	if (conf_switch_timed > 0)
		is_timed = true;
	else
		is_timed = false;

	/* 連続スワイプによるスキップ動作を無効にする */
	set_continuous_swipe_enabled(false);

	return true;
}

/* @chooseコマンドの初期化を行う */
static bool init_choose(void)
{
	const char *label, *msg;
	int i;

	memset(choose_button, 0, sizeof(choose_button));

	/* 選択肢の情報を取得する */
	for (i = 0; i < CHOOSE_COUNT; i++) {
		/* ラベルを取得する */
		label = get_string_param(CHOOSE_LABEL(i));
		if (strcmp(label, "") == 0)
			break;

		/* メッセージを取得する */
		msg = get_string_param(CHOOSE_MESSAGE(i));
		if (strcmp(msg, "") == 0) {
			log_script_choose_no_message();
			log_script_exec_footer();
			return false;
		}

		/* ボタンの情報を保存する */
		choose_button[i].msg = msg;
		choose_button[i].label = label;

		/* 座標を計算する */
		get_switch_rect(i,
				&choose_button[i].x,
				&choose_button[i].y,
				&choose_button[i].w,
				&choose_button[i].h);

		/* idle画像を作成する */
		choose_button[i].img_idle = create_image(choose_button[i].w, choose_button[i].h);
		if (choose_button[i].img_idle == NULL)
			return false;
		draw_switch_bg_image(choose_button[i].img_idle, i);

		/* hover画像を作成する */
		choose_button[i].img_hover = create_image(choose_button[i].w, choose_button[i].h);
		if (choose_button[i].img_hover == NULL)
			return false;
		draw_switch_fg_image(choose_button[i].img_hover, i);

		/* テキストを描画する */
		draw_text(choose_button[i].img_idle, choose_button[i].msg, choose_button[i].w, choose_button[i].h, true);
		draw_text(choose_button[i].img_hover, choose_button[i].msg, choose_button[i].w, choose_button[i].h, false);
	}

	/* テキスト読み上げする */
	if (conf_tts_enable == 1) {
		speak_text(NULL);
		if (strcmp(get_system_locale(), "ja") == 0)
			speak_text("選択肢が表示されています。左右のキーを押してください。");
		else
			speak_text("Options are displayed. Press the left or right arrow key.");
	}

	return true;
}

/* @ichooseコマンドの引数情報を取得する */
static bool init_ichoose(void)
{
	const char *label, *msg;
	int i, pen_x, pen_y;

	memset(choose_button, 0, sizeof(choose_button));

	is_centered = false;
	if (conf_msgbox_tategaki) {
		pen_x = get_pen_position_x() - conf_msgbox_margin_line;
		pen_y = conf_msgbox_y + conf_msgbox_margin_top;
	} else {
		pen_x = conf_msgbox_x + conf_msgbox_margin_left;
		pen_y = get_pen_position_y() + conf_msgbox_margin_line;
	}

	/* 選択肢の情報を取得する */
	for (i = 0; i < CHOOSE_COUNT; i++) {
		/* ラベルを取得する */
		label = get_string_param(CHOOSE_LABEL(i));
		if (strcmp(label, "") == 0)
			break;

		/* メッセージを取得する */
		msg = get_string_param(CHOOSE_MESSAGE(i));
		if (strcmp(msg, "") == 0) {
			log_script_choose_no_message();
			log_script_exec_footer();
			return false;
		}

		/* ボタンの情報を保存する */
		choose_button[i].msg = msg;
		choose_button[i].label = label;

		/* 座標を計算する */
		get_switch_rect(0,
				&choose_button[i].x,
				&choose_button[i].y,
				&choose_button[i].w,
				&choose_button[i].h);
		choose_button[i].x = pen_x;
		choose_button[i].y = pen_y;
		if (conf_msgbox_tategaki)
			pen_x -= conf_msgbox_margin_line;
		else
			pen_y += conf_msgbox_margin_line;

		/* idle画像を作成する */
		choose_button[i].img_idle = create_image(choose_button[i].w, choose_button[i].h);
		if (choose_button[i].img_idle == NULL)
			return false;
		draw_switch_bg_image(choose_button[i].img_idle, i);

		/* hover画像を作成する */
		choose_button[i].img_hover = create_image(choose_button[i].w, choose_button[i].h);
		if (choose_button[i].img_hover == NULL)
			return false;
		draw_switch_fg_image(choose_button[i].img_hover, i);

		/* テキストを描画する */
		draw_text(choose_button[i].img_idle, choose_button[i].msg, choose_button[i].w, choose_button[i].h, true);
		draw_text(choose_button[i].img_hover, choose_button[i].msg, choose_button[i].w, choose_button[i].h, false);
	}

	return true;
}

/* @mchooseコマンドの初期化を行う */
static int init_mchoose(void)
{
	const char *var, *label, *msg;
	int i, pos, var_index, var_val;

	memset(choose_button, 0, sizeof(choose_button));

	/* 選択肢の情報を取得する */
	pos = 0;
	for (i = 0; i < CHOOSE_COUNT; i++) {
		/* 変数を取得する */
		var = get_string_param(MCHOOSE_VAR(i));
		if (strcmp(var, "") == 0)
			break;
		if (var[0] != '$' || strlen(var) == 1) {
			log_script_lhs_not_variable(var);
			log_script_exec_footer();
			return -1;
		}
		var_index = atoi(&var[1]);
		var_val = get_variable(var_index);
		if (var_val == 0)
			continue;

		/* ラベルを取得する */
		label = get_string_param(MCHOOSE_LABEL(i));
		if (strcmp(label, "") == 0)
			break;

		/* メッセージを取得する */
		msg = get_string_param(MCHOOSE_MESSAGE(i));
		if (strcmp(msg, "") == 0) {
			log_script_choose_no_message();
			log_script_exec_footer();
			return false;
		}

		/* ボタンの情報を保存する */
		choose_button[pos].msg = msg;
		choose_button[pos].label = label;

		/* 座標を計算する */
		get_switch_rect(pos,
				&choose_button[pos].x,
				&choose_button[pos].y,
				&choose_button[pos].w,
				&choose_button[pos].h);

		/* idle画像を作成する */
		choose_button[pos].img_idle = create_image(choose_button[pos].w, choose_button[pos].h);
		if (choose_button[pos].img_idle == NULL)
			return false;
		draw_switch_bg_image(choose_button[pos].img_idle, pos);

		/* hover画像を作成する */
		choose_button[pos].img_hover = create_image(choose_button[pos].w, choose_button[pos].h);
		if (choose_button[pos].img_hover == NULL)
			return false;
		draw_switch_fg_image(choose_button[pos].img_hover, pos);

		/* テキストを描画する */
		draw_text(choose_button[pos].img_idle, choose_button[pos].msg, choose_button[pos].w, choose_button[pos].h, true);
		draw_text(choose_button[pos].img_hover, choose_button[pos].msg, choose_button[pos].w, choose_button[pos].h, false);

		pos++;
	}

	return pos;
}

/* @michooseコマンドの初期化を行う */
static int init_michoose(void)
{
	const char *var, *label, *msg;
	int i, pen_x, pen_y, pos, var_index, var_val;

	memset(choose_button, 0, sizeof(choose_button));

	is_centered = false;
	if (conf_msgbox_tategaki) {
		pen_x = get_pen_position_x() - conf_msgbox_margin_line;
		pen_y = conf_msgbox_y + conf_msgbox_margin_top;
	} else {
		pen_x = conf_msgbox_x + conf_msgbox_margin_left;
		pen_y = get_pen_position_y() + conf_msgbox_margin_line;
	}

	/* 選択肢の情報を取得する */
	pos = 0;
	for (i = 0; i < CHOOSE_COUNT; i++) {
		/* 変数を取得する */
		var = get_string_param(MCHOOSE_VAR(i));
		if (strcmp(var, "") == 0)
			break;
		if (var[0] != '$' || strlen(var) == 1) {
			log_script_lhs_not_variable(var);
			log_script_exec_footer();
			return -1;
		}
		var_index = atoi(&var[1]);
		var_val = get_variable(var_index);
		if (var_val == 0)
			continue;

		/* ラベルを取得する */
		label = get_string_param(MCHOOSE_LABEL(i));
		if (strcmp(label, "") == 0)
			break;

		/* メッセージを取得する */
		msg = get_string_param(MCHOOSE_MESSAGE(i));
		if (strcmp(msg, "") == 0) {
			log_script_choose_no_message();
			log_script_exec_footer();
			return false;
		}

		/* ボタンの情報を保存する */
		choose_button[i].msg = msg;
		choose_button[i].label = label;

		/* 座標を計算する */
		get_switch_rect(0,
				&choose_button[i].x,
				&choose_button[i].y,
				&choose_button[i].w,
				&choose_button[i].h);
		choose_button[i].x = pen_x;
		choose_button[i].y = pen_y;
		if (conf_msgbox_tategaki)
			pen_x -= conf_msgbox_margin_line;
		else
			pen_y += conf_msgbox_margin_line;

		/* idle画像を作成する */
		choose_button[i].img_idle = create_image(choose_button[i].w, choose_button[i].h);
		if (choose_button[i].img_idle == NULL)
			return false;
		draw_switch_bg_image(choose_button[i].img_idle, i);

		/* hover画像を作成する */
		choose_button[i].img_hover = create_image(choose_button[i].w, choose_button[i].h);
		if (choose_button[i].img_hover == NULL)
			return false;
		draw_switch_fg_image(choose_button[i].img_hover, i);

		/* テキストを描画する */
		draw_text(choose_button[i].img_idle, choose_button[i].msg, choose_button[i].w, choose_button[i].h, true);
		draw_text(choose_button[i].img_hover, choose_button[i].msg, choose_button[i].w, choose_button[i].h, false);

		pos++;
	}

	return pos;
}

/* 選択肢のテキストを描画する */
static void draw_text(struct image *target, const char *text, int w, int h, bool is_bg)
{
	struct draw_msg_context context;
	pixel_t color, outline_color;
	int font_size, char_count;
	int x, y;
	int outline_width;
	bool use_outline;

	x = 0;
	y = 0;

	/* フォントサイズを取得する */
	font_size = conf_switch_font_size > 0 ? conf_switch_font_size : conf_font_size;

	/* ふちどりを決定する */
	switch (conf_switch_font_outline) {
	case 0: use_outline = !conf_font_outline_remove; outline_width = 2 + conf_font_outline_add; break;
	case 1: use_outline = true; outline_width = 2 + conf_font_outline_add; break;
	case 2: use_outline = false; outline_width = 1; break;
	case 3: use_outline = true; outline_width = 1; break;
	case 4: use_outline = true; outline_width = 2; break;
	case 5: use_outline = true; outline_width = 3; break;
	case 6: use_outline = true; outline_width = 4; break;
	default: use_outline = false; outline_width = 4; break;
	}

	/* 色を決める */
	if (is_bg) {
		if (!conf_switch_color_inactive) {
			color = make_pixel(0xff,
					   (pixel_t)conf_font_color_r,
					   (pixel_t)conf_font_color_g,
					   (pixel_t)conf_font_color_b);
			outline_color = make_pixel(0xff,
						   (pixel_t)conf_font_outline_color_r,
						   (pixel_t)conf_font_outline_color_g,
						   (pixel_t)conf_font_outline_color_b);
		} else {
			color = make_pixel(0xff,
					   (pixel_t)conf_switch_color_inactive_body_r,
					   (pixel_t)conf_switch_color_inactive_body_g,
					   (pixel_t)conf_switch_color_inactive_body_b);
			outline_color = make_pixel(0xff,
						   (pixel_t)conf_switch_color_inactive_outline_r,
						   (pixel_t)conf_switch_color_inactive_outline_g,
						   (pixel_t)conf_switch_color_inactive_outline_b);
		}
	} else {
		if (!conf_switch_color_active) {
			color = make_pixel(0xff,
					   (pixel_t)conf_font_color_r,
					   (pixel_t)conf_font_color_g,
					   (pixel_t)conf_font_color_b);
			outline_color = make_pixel(0xff,
						   (pixel_t)conf_font_outline_color_r,
						   (pixel_t)conf_font_outline_color_g,
						   (pixel_t)conf_font_outline_color_b);
		} else {
			color = make_pixel(0xff,
					   (pixel_t)conf_switch_color_active_body_r,
					   (pixel_t)conf_switch_color_active_body_g,
					   (pixel_t)conf_switch_color_active_body_b);
			outline_color = make_pixel(0xff,
						   (pixel_t)conf_switch_color_active_outline_r,
						   (pixel_t)conf_switch_color_active_outline_g,
						   (pixel_t)conf_switch_color_active_outline_b);
		}
	}

	/* 描画位置を決める */
	if (is_centered) {
		if (!conf_msgbox_tategaki) {
			x = x + (w - get_string_width(conf_switch_font_select,
						      font_size,
						      text)) / 2;
			y += conf_switch_text_margin_y;
		} else {
			x = x + (w - font_size) / 2;
			y = y + (h - get_string_height(conf_switch_font_select,
						       font_size,
						       text)) / 2;
		}
	} else {
		y += conf_switch_text_margin_y;
	}

	/* 文字を描画する */
	construct_draw_msg_context(
		&context,
		-1,
		text,
		conf_switch_font_select,
		font_size,
		font_size,		/* base_font_size */
		conf_font_ruby_size,	/* FIXME: namebox.ruby.sizeの導入 */
		use_outline,
		outline_width,
		x,
		y,
		conf_window_width,
		conf_window_height,
		x,			/* left_margin */
		0,			/* right_margin */
		conf_switch_text_margin_y,
		0,			/* bottom_margin */
		0,			/* line_margin */
		conf_msgbox_margin_char,
		color,
		outline_color,
		false,			/* is_dimming */
		true,			/* ignore_linefeed */
		false,			/* ignore_font */
		false,			/* ignore_size */
		false,			/* ignore_color */
		false,			/* ignore_size */
		false,			/* ignore_position */
		false,			/* ignore_ruby */
		true,			/* ignore_wait */
		false,			/* fill_bg */
		NULL,			/* inline_wait_hook */
		conf_msgbox_tategaki);	/* use_tategaki */
	set_alternative_target_image(&context, target);
	char_count = count_chars_common(&context, NULL);
	draw_msg_common(&context, char_count);
}

/*
 * クリック処理
 */

/* システムメニュー非表示中の入力を処理する */
static void process_main_input(void)
{
	int old_pointed_index, new_pointed_index;
	bool enter_sysmenu;

	/* 選択項目を取得する */
	old_pointed_index = pointed_index;
	new_pointed_index = get_pointed_index();

	/* 選択項目が変更され、項目が選択され、TTSが有効で、キー入力で変更された場合 */
	if (new_pointed_index != old_pointed_index &&
	    new_pointed_index != -1 &&
	    conf_tts_enable == 1 &&
	    is_selected_by_key &&
	    choose_button[new_pointed_index].msg != NULL) {
		speak_text(NULL);
		speak_text(choose_button[pointed_index].msg);
	}

	/* 選択項目が変更され、項目が選択され、sysmenuが終了したフレームではない場合 */
	if (new_pointed_index != -1 &&
	    new_pointed_index != old_pointed_index &&
	    !is_sysmenu_finished) {
		/* 最初のフレームは避ける */
		if (!is_first_frame) {
			play_se(is_left_clicked ? conf_switch_click_se : conf_switch_change_se);
			run_anime(old_pointed_index, new_pointed_index);
		}
	}

	/* 選択項目がなくなった場合 */
	if (old_pointed_index != -1 &&
	    new_pointed_index == -1) {
		run_anime(old_pointed_index, -1);
	}

	/* 選択項目の変化を変数pointed_indexにコミットする */
	pointed_index = new_pointed_index;

	/* ヒストリ画面への遷移を確認する */
	if (is_up_pressed &&
	    !conf_msgbox_history_disable &&
	    get_history_count() > 0) {
		play_se(conf_msgbox_history_se);
		need_history_mode = true;
		return;
	}

	/* キー操作を受け付ける */
	if (is_s_pressed && conf_sysmenu_hidden != 2) {
		play_se(conf_sysmenu_save_se);
		need_save_mode = true;
		return;
	} else if (is_l_pressed && conf_sysmenu_hidden != 2) {
		play_se(conf_sysmenu_load_se);
		need_load_mode = true;
		return;
	} else if (is_h_pressed &&
		   !conf_msgbox_history_disable &&
		   get_history_count() != 0) {
		play_se(conf_msgbox_history_se);
		need_history_mode = true;
		return;
	}

	/* マウスの左ボタンでクリックされた場合 */
	if (pointed_index != -1 &&
	    (is_left_clicked || is_return_pressed) &&
	    !is_sysmenu_finished) {
		play_se(conf_switch_click_se);
		stop_command_repetition();
		run_anime(pointed_index, -1);
	}

	/* システムメニューを常に使用しない場合 */
	if (conf_sysmenu_hidden == 2)
		return;

	/* システムメニューへの遷移を確認していく */
	enter_sysmenu = false;

	/* 右クリックされたとき */
	if (is_right_clicked)
		enter_sysmenu = true;

	/* エスケープキーが押下されたとき */
	if (is_escape_pressed)
		enter_sysmenu = true;

	/* 折りたたみシステムメニューがクリックされたとき */
	if (is_left_clicked && is_collapsed_sysmenu_pointed())
		enter_sysmenu = true;

	/* システムメニューを開始するとき */
	if (enter_sysmenu) {
		/* SEを再生する */
		play_se(conf_sysmenu_enter_se);

		/* システムメニューを表示する */
		is_sysmenu = true;
		is_sysmenu_first_frame = true;
		sysmenu_pointed_index = get_pointed_sysmenu_item_extended();
		old_sysmenu_pointed_index = sysmenu_pointed_index;
		is_sysmenu_finished = false;

		run_anime(pointed_index, -1);
		return;
	}
}

/* ポイントされている選択肢を取得する */
static int get_pointed_index(void)
{
	int i;

	/* システムメニュー表示中は選択しない */
	if (is_sysmenu)
		return -1;

	/* 右キーを処理する */
	if (is_right_arrow_pressed) {
		is_selected_by_key = true;
		save_mouse_pos_x = mouse_pos_x;
		save_mouse_pos_y = mouse_pos_y;
		if (pointed_index == -1)
			return 0;
		if (pointed_index == CHOOSE_COUNT - 1)
			return 0;
		if (choose_button[pointed_index + 1].msg != NULL)
			return pointed_index + 1;
		else
			return 0;
	}

	/* 左キーを処理する */
	if (is_left_arrow_pressed) {
		is_selected_by_key = true;
		save_mouse_pos_x = mouse_pos_x;
		save_mouse_pos_y = mouse_pos_y;
		if (pointed_index == -1 ||
		    pointed_index == 0) {
			for (i = CHOOSE_COUNT - 1; i >= 0; i--)
				if (choose_button[i].msg != NULL)
					return i;
		}
		return pointed_index - 1;
	}

	/* マウスポイントを処理する */
	for (i = 0; i < CHOOSE_COUNT; i++) {
		if (choose_button[i].msg == NULL)
			continue;

		if (mouse_pos_x >= choose_button[i].x &&
		    mouse_pos_x < choose_button[i].x + choose_button[i].w &&
		    mouse_pos_y >= choose_button[i].y &&
		    mouse_pos_y < choose_button[i].y + choose_button[i].h) {
			/* キーで選択済みの項目があり、マウスが移動していない場合 */
			if (is_selected_by_key &&
			    mouse_pos_x == save_mouse_pos_x &&
			    mouse_pos_y == save_mouse_pos_y)
				continue;
			is_selected_by_key = false;
			return i;
		}
	}

	/* キーによる選択が行われている場合は維持する */
	if (is_selected_by_key)
		return pointed_index;

	/* その他の場合、何も選択しない */
	return -1;
}

/* システムメニュー表示中のクリックを処理する */
static void process_sysmenu_input(void)
{
	/* キー操作を受け付ける */
	if (is_s_pressed) {
		play_se(conf_sysmenu_save_se);
		need_save_mode = true;
		is_sysmenu = false;
		is_sysmenu_finished = true;
		return;
	} else if (is_l_pressed) {
		play_se(conf_sysmenu_load_se);
		need_load_mode = true;
		is_sysmenu = false;
		is_sysmenu_finished = true;
		return;
	} else if (is_h_pressed) {
		play_se(conf_sysmenu_history_se);
		need_history_mode = true;
		is_sysmenu = false;
		is_sysmenu_finished = true;
		return;
	}

	/* 右クリックされた場合と、エスケープキーが押下されたとき */
	if (is_right_clicked || is_escape_pressed) {
		/* SEを再生する */
		play_se(conf_sysmenu_leave_se);

		/* システムメニューを終了する */
		is_sysmenu = false;
		is_sysmenu_finished = true;
		return;
	}

	/* ポイントされているシステムメニューのボタンを求める */
	old_sysmenu_pointed_index = sysmenu_pointed_index;
	sysmenu_pointed_index = get_pointed_sysmenu_item_extended();

	/* ポイントされている項目が変化した場合で、クリックではない場合 */
	if (sysmenu_pointed_index != old_sysmenu_pointed_index &&
	    sysmenu_pointed_index != SYSMENU_NONE &&
	    !is_left_clicked) {
		/* ただし最初のフレームですでにポイントされていた場合は除く */
		if (!is_sysmenu_first_frame) {
			/* SEを再生する */		     
			play_se(conf_sysmenu_change_se);
		}
	}

	/* ボタンのないところを左クリックされた場合 */
	if (sysmenu_pointed_index == SYSMENU_NONE && is_left_clicked) {
		/* SEを再生する */
		play_se(conf_sysmenu_leave_se);

		/* システムメニューを終了する */
		is_sysmenu = false;
		is_sysmenu_finished = true;
		return;
	}

	/* 左クリックされていない場合、何もしない */
	if (!is_left_clicked)
		return;

	/* ボタンを処理する */
	switch (sysmenu_pointed_index) {
	case SYSMENU_QSAVE:
		play_se(conf_sysmenu_qsave_se);
		will_quick_save = true;
		break;
	case SYSMENU_QLOAD:
		play_se(conf_sysmenu_qload_se);
		if (!quick_load(false))
			is_quick_load_failed = true;
		did_quick_load = true;
		break;
	case SYSMENU_SAVE:
		play_se(conf_sysmenu_save_se);
		need_save_mode = true;
		break;
	case SYSMENU_LOAD:
		play_se(conf_sysmenu_load_se);
		need_load_mode = true;
		break;
	case SYSMENU_HISTORY:
		/* ヒストリがある場合のみ */
		if (get_history_count() > 0) {
			play_se(conf_sysmenu_history_se);
			need_history_mode = true;
		}
		break;
	case SYSMENU_CONFIG:
		play_se(conf_sysmenu_config_se);
		need_config_mode = true;
		break;
	case SYSMENU_CUSTOM1:
		set_mixer_input(VOICE_STREAM, NULL);
		play_se(conf_sysmenu_custom1_se);
		need_custom1_mode = true;
		break;
	case SYSMENU_CUSTOM2:
		set_mixer_input(VOICE_STREAM, NULL);
		play_se(conf_sysmenu_custom2_se);
		need_custom2_mode = true;
		break;
	default:
		assert(0);
		break;
	}

	/* システムメニューを終了する */
	is_sysmenu = false;
	is_sysmenu_finished = true;
}

/* 選択中のシステムメニューのボタンを取得する */
static int get_pointed_sysmenu_item_extended(void)
{
	int index;

	/* システムメニューを表示中でない場合は非選択とする */
	if (!is_sysmenu)
		return SYSMENU_NONE;

	/* ポイントされているボタンを返す */
	index = get_pointed_sysmenu_item();

	/* セーブロードが無効な場合、セーブロードのポイントを無効にする */
	if (!is_save_load_enabled() &&
	    (index == SYSMENU_QSAVE ||
	     index == SYSMENU_QLOAD ||
	     index == SYSMENU_SAVE ||
	     index == SYSMENU_LOAD))
		index = SYSMENU_NONE;

	/* クイックセーブデータがない場合、QLOADのポイントを無効にする */
	if (!have_quick_save_data() &&
	    index == SYSMENU_QLOAD)
		index = SYSMENU_NONE;

	/* オートとスキップのポイントを無効にする */
	if (index == SYSMENU_AUTO || index == SYSMENU_SKIP)
		index  = SYSMENU_NONE;

	return index;
}

/*
 * 描画
 */

/* フレームを描画する */
static void render_frame(void)
{
	int i;

	/* セーブ画面かヒストリ画面から復帰した場合のフラグをクリアする */
	check_gui_flag();

	/* ステージを描画する */
	render_stage();

	/* 選択肢を描画する */
	for (i = 0; i < CHOOSE_COUNT; i++) {
		struct image *img;

		if (i != pointed_index)
			img = choose_button[i].img_idle;
		else
			img = choose_button[i].img_hover;
	
		if (img == NULL)
			continue;

		render_image_normal(choose_button[i].x,
				    choose_button[i].y,
				    img->width,
				    img->height,
				    img,
				    0,
				    0,
				    img->width,
				    img->height,
				    255);
	}

	/* 折りたたみシステムメニューを描画する */
	if (!is_non_interruptible() && !is_sysmenu)
		render_collapsed_sysmenu_extended();

	/* システムメニューを表示する */
	if (!is_non_interruptible() && is_sysmenu)
		render_sysmenu_extended();
}

/*
 * システムメニュー
 */

/* システムメニューを描画する */
static void render_sysmenu_extended(void)
{
	bool qsave_sel, qload_sel, save_sel, load_sel, auto_sel, skip_sel;
	bool history_sel, config_sel, custom1_sel, custom2_sel;

	/* 描画するかの判定状態を初期化する */
	qsave_sel = false;
	qload_sel = false;
	save_sel = false;
	load_sel = false;
	auto_sel = false;
	skip_sel = false;
	history_sel = false;
	config_sel = false;
	custom1_sel = false;
	custom2_sel = false;

	/* クイックセーブボタンがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_QSAVE)
		qsave_sel = true;

	/* クイックロードボタンがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_QLOAD)
		qload_sel = true;

	/* セーブボタンがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_SAVE)
		save_sel = true;

	/* ロードボタンがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_LOAD)
		load_sel = true;

	/* ヒストリがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_HISTORY)
		history_sel = true;

	/* コンフィグがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_CONFIG)
		config_sel = true;

	/* CUSTOM1がポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_CUSTOM1)
		custom1_sel = true;

	/* CUSTOM2がポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_CUSTOM2)
		custom2_sel = true;

	/* システムメニューを描画する */
	render_sysmenu(false,
		       false,
		       is_save_load_enabled(),
		       is_save_load_enabled() &&
		       have_quick_save_data(),
		       qsave_sel,
		       qload_sel,
		       save_sel,
		       load_sel,
		       auto_sel,
		       skip_sel,
		       history_sel,
		       config_sel,
		       custom1_sel,
		       custom2_sel);
	is_sysmenu_first_frame = false;
}

/* 折りたたみシステムメニューを描画する */
static void render_collapsed_sysmenu_extended(void)
{
	bool is_pointed;

 	/* システムメニューを常に使用しない場合 */
	if (conf_sysmenu_hidden == 2)
		return;

	/* 折りたたみシステムメニューがポイントされているか調べる */
	is_pointed = is_collapsed_sysmenu_pointed();

	/* 描画する */
	render_collapsed_sysmenu(is_pointed);

	/* SEを再生する */
	if (!is_sysmenu_finished &&
	    (is_collapsed_sysmenu_pointed_prev != is_pointed))
		play_se(conf_sysmenu_collapsed_se);

	/* 折りたたみシステムメニューのポイント状態を保持する */
	is_collapsed_sysmenu_pointed_prev = is_pointed;
}

/*
 * その他
 */

/* SEを再生する */
static void play_se(const char *file)
{
	struct wave *w;

	if (file == NULL || strcmp(file, "") == 0)
		return;

	w = create_wave_from_file(SE_DIR, file, false);
	if (w == NULL)
		return;

	set_mixer_input(SYS_STREAM, w);
}

/* アニメを実行する */
static void run_anime(int unfocus_index, int focus_index)
{
	/* フォーカスされなくなる項目のアニメ */
	if (unfocus_index != -1 && conf_switch_anime_unfocus[unfocus_index] != NULL)
		load_anime_from_file(conf_switch_anime_unfocus[unfocus_index], -1, NULL);

	/* フォーカスされる項目のアニメ */
	if (focus_index != -1 && conf_switch_anime_focus[focus_index] != NULL)
		load_anime_from_file(conf_switch_anime_focus[focus_index], -1, NULL);
}

/*
 * クリーンアップ
 */

/* コマンドを終了する */
static bool cleanup(void)
{
	int i;

	/* アニメを停止する */
	for (i = 0; i < CHOOSE_COUNT; i++)
		run_anime(i, -1);

	/* 画像を破棄する */
	for (i = 0; i < CHOOSE_COUNT; i++) {
		if (choose_button[i].img_idle != NULL)
			destroy_image(choose_button[i].img_idle);
		if (choose_button[i].img_hover != NULL)
			destroy_image(choose_button[i].img_hover);
		choose_button[i].img_idle = NULL;
		choose_button[i].img_hover = NULL;
	}

	/* カスタムシステムメニューのgosubを処理する */
	if ((need_custom1_mode || need_custom2_mode) && custom_gosub_target != NULL) {
		push_return_point_minus_one();
		if (!move_to_label(custom_gosub_target))
			return false;
		return true;
	}

	/* クイックロードやシステムGUIへの遷移を行う場合 */
	if (did_quick_load || need_save_mode || need_load_mode ||
	    need_history_mode || need_config_mode ||
	    need_custom1_mode || need_custom2_mode) {
		/* コマンドの移動を行わない */
		return true;
	}

	/* 時間制限に達したとき */
	if (is_bombed) {
		if (!move_to_next_command())
			return false;
		return true;
	}

	/*
	 * 選択肢が選択された場合
	 */
	return move_to_label(choose_button[pointed_index].label);
}
