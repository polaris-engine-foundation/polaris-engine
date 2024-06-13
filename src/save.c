/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * Save data management.
 */

#include "polarisengine.h"

#if defined(USE_EDITOR)
#include "pro.h"
#endif

/* セーブデータの互換性バージョン(Suika2 12.42で導入) */
#define SAVE_VER	(0x010216)

#ifdef XENGINE_TARGET_WASM
#include <emscripten/emscripten.h>
#endif

/* False assertion */
#define CONFIG_TYPE_ERROR	(0)

/* クイックセーブデータのインデックス */
#define QUICK_SAVE_INDEX	(SAVE_SLOTS)

/* コンフィグの終端記号 */
#define END_OF_CONFIG		"eoc"

/* ロードされた直後であるかのフラグ */
static bool load_flag;

/*
 * このモジュールで保持する設定値
 */

/* 章題*/
static char *chapter_name;

/* 最後のメッセージ */
static char *last_message;

/* 最後のメッセージ(最後の継続部分を除く) */
static char *prev_last_message;

/* ロード直後に復元されるメッセージ */
static char *pending_message;

/* テキストの表示スピード */
static float msg_text_speed;

/* オートモードの待ち時間の長さ */
static float msg_auto_speed;

/*
 * メモリ上に読み込んだセーブデータ
 */

/* セーブデータの日付 (0のときデータなし) */
static time_t save_time[SAVE_SLOTS];

/* 最後にセーブされたセーブデータ番号 */
static int latest_index;

/* セーブデータの章タイトル */
static char *save_title[SAVE_SLOTS];

/* セーブデータのメッセージ */
static char *save_message[SAVE_SLOTS];

/* セーブデータのサムネイル */
static struct image *save_thumb[SAVE_SLOTS];

/* クイックセーブデータの日付 */
static time_t quick_save_time;

/* 最後の+en+コマンドの位置 */
static int last_en_command = -1;

/*
 * 作業用バッファ
 */

/* 文字列読み込み用バッファ */
static char tmp_str[4096];

/* サムネイル読み書き用バッファ */
static unsigned char *tmp_pixels;

/* 前方参照 */
static void load_basic_save_data(void);
static void load_basic_save_data_file(struct rfile *rf, int index);
static bool serialize_all(const char *fname, uint64_t *timestamp, int index);
static bool serialize_title(struct wfile *wf, int index);
static bool serialize_message(struct wfile *wf, int index);
static bool serialize_thumb(struct wfile *wf, int index);
static bool serialize_command(struct wfile *wf);
static bool serialize_stage(struct wfile *wf);
static bool serialize_anime(struct wfile *wf);
static bool serialize_sound(struct wfile *wf);
static bool serialize_volumes(struct wfile *wf);
static bool serialize_vars(struct wfile *wf);
static bool serialize_name_vars(struct wfile *wf);
static bool serialize_local_config(struct wfile *wf);
static bool serialize_config_helper(struct wfile *wf, bool is_global);
static bool deserialize_all(const char *fname);
static bool deserialize_command(struct rfile *rf);
static bool deserialize_stage(struct rfile *rf);
static bool deserialize_anime(struct rfile *rf);
static bool deserialize_sound(struct rfile *rf);
static bool deserialize_volumes(struct rfile *rf);
static bool deserialize_vars(struct rfile *rf);
static bool deserialize_name_vars(struct rfile *rf);
static bool deserialize_config_common(struct rfile *rf);
static void load_global_data(void);

/*
 * 初期化
 */

/*
 * セーブデータに関する初期化処理を行う
 */
bool init_save(void)
{
	int i;

	/* 再利用時のための初期化を行う */
	msg_text_speed = 0.5f;
	msg_auto_speed = 0.5f;

	/* セーブスロットを初期化する */
	for (i = 0; i < SAVE_SLOTS; i++) {
		save_time[i] = 0;
		if (save_title[i] != NULL) {
			free(save_title[i]);
			save_title[i] = NULL;
		}
		if (save_message[i] != NULL) {
			free(save_message[i]);
			save_message[i] = NULL;
		}
		if (save_thumb[i] != NULL) {
			destroy_image(save_thumb[i]);
			save_thumb[i] = NULL;
		}
	}

	/* 文字列を初期化する */
	if (chapter_name != NULL) {
		free(chapter_name);
		chapter_name = NULL;
	}
	if (last_message != NULL) {
		free(last_message);
		last_message = NULL;
	}
	if (prev_last_message != NULL) {
		free(prev_last_message);
		prev_last_message = NULL;
	}
	chapter_name = strdup("");
	last_message = strdup("");
	if (chapter_name == NULL || last_message == NULL) {
		log_memory();
		return false;
	}

	/* サムネイルの読み書きのためのヒープを確保する */
	if (tmp_pixels != NULL)
		free(tmp_pixels);
	tmp_pixels = malloc((size_t)(conf_save_data_thumb_width *
				     conf_save_data_thumb_height * 3));
	if (tmp_pixels == NULL) {
		log_memory();
		return false;
	}

	/* セーブデータから基本情報を取得する */
	load_basic_save_data();

	/* グローバルデータのロードを行う */
	load_global_data();

	last_en_command = -1;

	return true;
}

/*
 * セーブデータに関する終了処理を行う
 */
void cleanup_save(void)
{
	int i;

	for (i = 0; i < SAVE_SLOTS; i++) {
		if (save_title[i] != NULL) {
			free(save_title[i]);
			save_title[i] = NULL;
		}
		if (save_message[i] != NULL) {
			free(save_message[i]);
			save_message[i] = NULL;
		}
		if (save_thumb[i] != NULL) {
			destroy_image(save_thumb[i]);
			save_thumb[i] = NULL;
		}
	}

	free(chapter_name);
	chapter_name = NULL;

	free(last_message);
	last_message = NULL;

	free(tmp_pixels);
	tmp_pixels = NULL;

	/* グローバル変数のセーブを行う */
	save_global_data();
}

/*
 * ロードが終了した直後であるかを調べる
 */
bool check_load_flag(void)
{
	if (load_flag) {
		load_flag = false;
		return true;
	}
	return false;
}

/*
 * GUIからの問い合わせ
 */

/*
 * セーブデータの日付を取得する
 */
time_t get_save_date(int index)
{
	assert(index >= 0);
	if (index >= SAVE_SLOTS)
		return 0;

	return save_time[index];
}

/*
 * 最新のセーブデータの番号を取得する
 */
int get_latest_save_index(void)
{
	return latest_index;
}

/*
 * セーブデータの章タイトルを取得する
 */
const char *get_save_chapter_name(int index)
{
	assert(index >= 0);
	if (index >= SAVE_SLOTS)
		return 0;

	return save_title[index];
}

/*
 * セーブデータの最後のメッセージを取得する
 */
const char *get_save_last_message(int index)
{
	assert(index >= 0);
	if (index >= SAVE_SLOTS)
		return 0;

	return save_message[index];
}

/*
 * セーブデータのサムネイルを取得する
 */
struct image *get_save_thumbnail(int index)
{
	assert(index >= 0);
	if (index >= SAVE_SLOTS)
		return NULL;

	return save_thumb[index];
}

/*
 * セーブの実際の処理
 */

/*
 * クイックセーブを行う
 */
bool quick_save(bool extra)
{
	const char *fname;
	uint64_t timestamp;

	/*
	 * サムネイルを作成する
	 *  - GUIを経由しないのでここで作成する
	 *  - ただし、現状ではクイックセーブデータのサムネイルは未使用
	 */
	/*
	  In message command, use draw_stage_to_thumb().
	  In switch command, use draw_stage_fo_thumb().
	*/

	fname = !extra ? QUICK_SAVE_FILE : QUICK_SAVE_EXTRA_FILE;

	/* ローカルデータのシリアライズを行う */
	if (!serialize_all(fname, &timestamp, -1))
		return false;

	/* 既読フラグのセーブを行う */
	save_seen();

	/* グローバル変数のセーブを行う */
	save_global_data();

	/* クイックセーブの時刻を更新する */
	quick_save_time = (time_t)timestamp;

#ifdef XENGINE_TARGET_WASM
	EM_ASM_({
		if (window.navigator.userAgent.indexOf('iPad') != -1 || window.navigator.userAgent.indexOf('iPhone') != -1) {
			FS.syncfs(function (err) {});
		} else {
			FS.syncfs(function (err) { alert('Saved!'); });
		}
	});
#endif
	return true;
}

/*
 * セーブを実行する
 */
bool execute_save(int index)
{
	char s[128];
	uint64_t timestamp;

	/* ファイル名を求める */
	snprintf(s, sizeof(s), "%03d.sav", index);

	/* ローカルデータのシリアライズを行う */
	if (!serialize_all(s, &timestamp, index))
		return false;

	/* 既読フラグのセーブを行う */
	save_seen();

	/* グローバル変数のセーブを行う */
	save_global_data();

	/* 時刻を保存する */
	save_time[index] = (time_t)timestamp;

#ifdef XENGINE_TARGET_WASM
	EM_ASM_({
		if (window.navigator.userAgent.indexOf('iPad') != -1 || window.navigator.userAgent.indexOf('iPhone') != -1) {
			FS.syncfs(function (err) {});
		} else {
			FS.syncfs(function (err) { alert('Saved!'); });
		}
	});
#endif

	latest_index = index;

	return true;
}

/* すべてのシリアライズを行う */
static bool serialize_all(const char *fname, uint64_t *timestamp, int index)
{
	struct wfile *wf;
	uint64_t t;
	uint32_t ver;
	bool success;

	/* セーブディレクトリを作成する */
	make_sav_dir();

	/* ファイルを開く */
	wf = open_wfile(SAVE_DIR, fname);
	if (wf == NULL)
		return false;

	success = false;
	t = 0;
	do {
		/* セーブデータバージョンを書き込む */
		ver = (uint32_t)SAVE_VER;
		if (write_wfile(wf, &ver, sizeof(ver)) < sizeof(ver))
			break;

		/* 日付を書き込む */
		t = (uint64_t)time(NULL);
		if (write_wfile(wf, &t, sizeof(t)) < sizeof(t))
			break;

		/* 章題のシリアライズを行う */
		if (!serialize_title(wf, index))
			break;

		/* メッセージのシリアライズを行う */
		if (!serialize_message(wf, index))
			break;

		/* サムネイルのシリアライズを行う */
		if (!serialize_thumb(wf, index))
			break;

		/* コマンド位置のシリアライズを行う */
		if (!serialize_command(wf))
			break;

		/* ステージのシリアライズを行う */
		if (!serialize_stage(wf))
			break;

		/* アニメのシリアライズを行う */
		if (!serialize_anime(wf))
			break;

		/* サウンドのシリアライズを行う */
		if (!serialize_sound(wf))
			break;

		/* ボリュームのシリアライズを行う */
		if (!serialize_volumes(wf))
			break;

		/* 変数のシリアライズを行う */
		if (!serialize_vars(wf))
			break;

		/* 名前変数のシリアライズを行う */
		if (!serialize_name_vars(wf))
			break;

		/* Cielの仮ステージのシリアライズを行う */
		if (!ciel_serialize_hook(wf))
			break;

		/* コンフィグのシリアライズを行う */
		if (!serialize_local_config(wf))
			break;

		/* 成功 */
		success = true;
	} while (0);

	/* ファイルをクローズする */
	close_wfile(wf);

	/* 時刻を保存する */
	*timestamp = t;

	if (!success)
		log_file_write(fname);

	return success;
}

/* 章題のシリアライズを行う */
static bool serialize_title(struct wfile *wf, int index)
{
	size_t len;

	/* 文字列を準備する */
	strncpy(tmp_str, chapter_name, sizeof(tmp_str));
	tmp_str[sizeof(tmp_str) - 1] = '\0';

	/* 書き出す */
	len = strlen(tmp_str) + 1;
	if (write_wfile(wf, tmp_str, len) < len)
		return false;

	/* 章題を保存する */
	if (index != -1) {
		if (save_title[index] != NULL)
			free(save_title[index]);
		save_title[index] = strdup(tmp_str);
		if (save_title[index] == NULL) {
			log_memory();
			return false;
		}
	}

	return true;
}

/* メッセージのシリアライズを行う */
static bool serialize_message(struct wfile *wf, int index)
{
	const char *t;
	size_t len;

	/* メッセージを書き出す */
	t = last_message != NULL ? last_message : "";
	len = strlen(t) + 1;
	if (write_wfile(wf, t, len) < len)
		return false;

	/* メッセージを保存する */
	if (index != -1) {
		if (save_message[index] != NULL)
			free(save_message[index]);
		save_message[index] = strdup(t);
		if (save_message[index] == NULL) {
			log_memory();
			return false;
		}
	}

	/* 継続行用のメッセージを書き出す */
	t = prev_last_message != NULL ? prev_last_message : "";
	len = strlen(t) + 1;
	if (write_wfile(wf, t, len) < len)
		return false;

	return true;
}

/* サムネイルのシリアライズを行う */
static bool serialize_thumb(struct wfile *wf, int index)
{
	pixel_t *src, pix;
	unsigned char *dst;
	size_t len;
	int x, y;

	/* クイックセーブの場合 */
	if (index == -1) {
		/* 内容は気にせず書き出す */
		len = (size_t)(conf_save_data_thumb_width *
			       conf_save_data_thumb_height * 3);
		if (write_wfile(wf, tmp_pixels, len) < len)
			return false;
		return true;
	}

	/* stage.cのサムネイルをsave.cのイメージにコピーする */
	if (save_thumb[index] == NULL) {
		save_thumb[index] = create_image(conf_save_data_thumb_width,
						 conf_save_data_thumb_height);
		if (save_thumb[index] == NULL)
			return false;
	}
	draw_image_copy(save_thumb[index], 0, 0, get_thumb_image(),
			conf_save_data_thumb_width, conf_save_data_thumb_height, 0, 0);
	notify_image_update(save_thumb[index]);

	/* ピクセル列を準備する */
	src = get_thumb_image()->pixels;
	dst = tmp_pixels;
	for (y = 0; y < conf_save_data_thumb_height; y++) {
		for (x = 0; x < conf_save_data_thumb_width; x++) {
			pix = *src++;
			*dst++ = (unsigned char)get_pixel_r(pix);
			*dst++ = (unsigned char)get_pixel_g(pix);
			*dst++ = (unsigned char)get_pixel_b(pix);
		}
	}

	/* 書き出す */
	len = (size_t)(conf_save_data_thumb_width * conf_save_data_thumb_height * 3);
	if (write_wfile(wf, tmp_pixels, len) < len)
		return false;

	return true;
}

/* コマンド位置をシリアライズする */
static bool serialize_command(struct wfile *wf)
{
	const char *s;
	int n, m;

	/* スクリプトファイル名を取得してシリアライズする */
	s = get_script_file_name();
	if (write_wfile(wf, s, strlen(s) + 1) < strlen(s) + 1)
		return false;

	/* コマンドインデックスを取得してシリアライズする */
	n = get_command_index();
	if (last_en_command != -1 &&
	    n >= last_en_command && n < last_en_command + 10)
		n = last_en_command;
	if (write_wfile(wf, &n, sizeof(n)) < sizeof(n))
		return false;

	/* '@gosub'のリターンポイントを取得してシリアライズする */
	m = get_return_point();
	if (write_wfile(wf, &m, sizeof(m)) < sizeof(m))
		return false;

	return true;
}

/* ステージをシリアライズする */
static bool serialize_stage(struct wfile *wf)
{
	const char *file, *text;
	size_t len;
	int i, x, y, alpha;

	for (i = 0; i < STAGE_LAYERS; i++) {
		/* Exclude the following layers. */
		switch (i) {
		case LAYER_MSG: continue;
		case LAYER_NAME: continue;
		case LAYER_CLICK: continue;
		case LAYER_AUTO: continue;
		case LAYER_SKIP: continue;
		case LAYER_CHB_EYE: continue;
		case LAYER_CHL_EYE: continue;
		case LAYER_CHLC_EYE: continue;
		case LAYER_CHR_EYE: continue;
		case LAYER_CHRC_EYE: continue;
		case LAYER_CHC_EYE: continue;
		case LAYER_CHF_EYE: continue;
		case LAYER_CHB_LIP: continue;
		case LAYER_CHL_LIP: continue;
		case LAYER_CHLC_LIP: continue;
		case LAYER_CHR_LIP: continue;
		case LAYER_CHRC_LIP: continue;
		case LAYER_CHC_LIP: continue;
		case LAYER_CHF_LIP: continue;
		default: break;
		}

		file = get_layer_file_name(i);
		if (file == NULL)
			file = "none";
		if (strcmp(file, "") == 0)
			file = "none";
		if (write_wfile(wf, file, strlen(file) + 1) < strlen(file) + 1)
			return false;

		x = get_layer_x(i);
		y = get_layer_y(i);
		if (write_wfile(wf, &x, sizeof(x)) < sizeof(x))
			return false;
		if (write_wfile(wf, &y, sizeof(y)) < sizeof(y))
			return false;

		alpha = get_layer_alpha(i);
		if (write_wfile(wf, &alpha, sizeof(alpha)) < sizeof(alpha))
			return false;

		if (i >= LAYER_TEXT1 && i <= LAYER_TEXT8) {
			text = get_layer_text(i);
			if (text == NULL)
				text = "";

			len = strlen(text) + 1;
			if (write_wfile(wf, text, len) < len)
				return false;
		}
	}

	return true;
}

/* アニメをシリアライズする */
static bool serialize_anime(struct wfile *wf)
{
	const char *file;
	int i;

	for (i = 0; i < REG_ANIME_COUNT; i++) {
		file = get_reg_anime_file_name(i);
		if (file == NULL)
			file = "none";
		if (strcmp(file, "") == 0)
			file = "none";
		if (write_wfile(wf, file, strlen(file) + 1) < strlen(file) + 1)
			return false;
	}

	return true;
}

/* サウンドをシリアライズする */
static bool serialize_sound(struct wfile *wf)
{
	const char *s;

	/* BGMをシリアライズする */
	s = get_bgm_file_name();
	if (s == NULL)
		s = "none";
	if (write_wfile(wf, s, strlen(s) + 1) < strlen(s) + 1)
		return false;

	/* SEをシリアライズする(ループ再生時のみ) */
	s = get_se_file_name();
	if (s == NULL)
		s = "none";
	if (write_wfile(wf, s, strlen(s) + 1) < strlen(s) + 1)
		return false;

	return true;
}

/* ボリュームをシリアライズする */
static bool serialize_volumes(struct wfile *wf)
{
	float vol;
	int n;

	for (n = 0; n < MIXER_STREAMS; n++) {
		vol = get_mixer_volume(n);
		if (write_wfile(wf, &vol, sizeof(vol)) < sizeof(vol))
			return false;
	}

	return true;
}

/* ローカル変数をシリアライズする */
static bool serialize_vars(struct wfile *wf)
{
	size_t len;

	len = LOCAL_VAR_SIZE * sizeof(int32_t);
	if (write_wfile(wf, get_local_variables_pointer(), len) < len)
		return false;

	return true;
}

/* 名前変数をシリアライズする */
static bool serialize_name_vars(struct wfile *wf)
{
	size_t len;
	const char *name;
	int i;

	for (i = 0; i < NAME_VAR_SIZE; i++) {
		name = get_name_variable(i);
		assert(name != NULL);

		len = strlen(name) + 1;
		if (write_wfile(wf, name, len) < len)
			return false;
	}

	return true;
}

/* ローカルコンフィグをシリアライズする */
static bool serialize_local_config(struct wfile *wf)
{
	if (!serialize_config_helper(wf, false))
		return false;

	return true;
}

/* コンフィグをシリアライズする */
static bool serialize_config_helper(struct wfile *wf, bool is_global)
{
	char val[1024];
	const char *key, *val_s;
	size_t len;
	int key_index;

	/* 保存可能なキーを列挙してループする */
	key_index = 0;
	while (1) {
		/* セーブするキーを取得する */
		key = get_config_key_for_save_data(key_index++);
		if (key == NULL) {
			/* キー列挙が終了した */
			break;
		}

		/* グローバル/ローカルの対象をチェックする */
		if (!is_global) {
			if (is_config_key_global(key))
				continue;
		} else {
			if (!is_config_key_global(key))
				continue;
		}

		/* キーを出力する */
		len = strlen(key) + 1;
		if (write_wfile(wf, key, len) < len)
			return false;

		/* 型ごとに値を出力する */
		switch (get_config_type_for_key(key)) {
		case 's':
			val_s = get_string_config_value_for_key(key);
			if (val_s == NULL)
				val_s = "";
			len = strlen(val_s) + 1;
			if (write_wfile(wf, val_s, len) < len)
				return false;
			break;
		case 'i':
			snprintf(val, sizeof(val), "%d",
				 get_int_config_value_for_key(key));
			len = strlen(val) + 1;
			if (write_wfile(wf, &val, len) < len)
				return false;
			break;
		case 'f':
			snprintf(val, sizeof(val), "%f",
				 get_float_config_value_for_key(key));
			len = strlen(val) + 1;
			if (write_wfile(wf, &val, len) < len)
				return false;
			break;
		default:
			assert(CONFIG_TYPE_ERROR);
			break;
		}
	}

	/* 終端記号を出力する */
	len = strlen(END_OF_CONFIG);
	if (write_wfile(wf, END_OF_CONFIG, len) < len)
		return false;

	return true;
}

/*
 * ロードの実際の処理
 */

/*
 * クイックセーブデータがあるか
 */
bool have_quick_save_data(void)
{
	if (quick_save_time == 0)
		return false;

	return true;
}

/*
 * クイックロードを行う Do quick load
 */
bool quick_load(bool extra)
{
	const char *fname;

	/* 既読フラグのセーブを行う */
	save_seen();

	/* グローバル変数のセーブを行う */
	save_global_data();

	/* ステージをクリアする */
	clear_stage();

	/* アニメを停止する */
	cleanup_anime();

	/* ローカルデータのデシリアライズを行う */
	fname = !extra ? QUICK_SAVE_FILE : QUICK_SAVE_EXTRA_FILE;
	if (!deserialize_all(fname))
		return false;

	/* ステージを初期化する */
	if (!reload_stage())
		abort();

	/* 名前ボックス、メッセージボックス、選択ボックスを非表示とする */
	show_namebox(false);
	show_msgbox(false);

	/* ウィンドウタイトルをアップデートする */
	if (!conf_window_title_chapter_disable)
		update_window_title();

	/* オートモードを解除する */
	if (is_auto_mode())
		stop_auto_mode();

	/* スキップモードを解除する */
	if (is_skip_mode())
		stop_skip_mode();

#ifdef USE_EDITOR
	clear_variable_changed();
	on_load_script();
	on_change_position();
#endif

	load_flag = true;

	if (is_message_active())
		clear_message_active();

	return true;
}

/*
 * ロードを処理する
 */
bool execute_load(int index)
{
	char s[128];

	/* ファイル名を求める */
	snprintf(s, sizeof(s), "%03d.sav", index);

	/* 既読フラグのセーブを行う */
	save_seen();

	/* グローバル変数のセーブを行う */
	save_global_data();

	/* ステージをクリアする */
	clear_stage();

	/* アニメを停止する */
	cleanup_anime();

	/* ローカルデータのデシリアライズを行う */
	if (!deserialize_all(s))
		return false;

	/* ステージを初期化する */
	if (!reload_stage())
		abort();

	/* 名前ボックス、メッセージボックス、選択ボックスを非表示とする */
	show_namebox(false);
	show_msgbox(false);

	/* ウィンドウタイトルをアップデートする */
	if (!conf_window_title_chapter_disable)
		update_window_title();
	
#ifdef USE_EDITOR
	clear_variable_changed();
	on_load_script();
	on_change_position();
#endif

	load_flag = true;

	if (is_message_active())
		clear_message_active();

	return true;
}

/* すべてをデシリアライズする */
static bool deserialize_all(const char *fname)
{
	struct rfile *rf;
	uint64_t t;
	size_t img_size;
	uint32_t ver;
	bool success;

	/* ファイルを開く */
	rf = open_rfile(SAVE_DIR, fname, true);
	if (rf == NULL)
		return false;

	success = false;
	do {
		/* セーブデータバージョンを読み込む */
		if (read_rfile(rf, &ver, sizeof(ver)) < sizeof(ver))
			break;
		if (ver != SAVE_VER) {
			log_save_ver();
			break;
		}

		/* 日付を読み込む (読み飛ばす) */
		if (read_rfile(rf, &t, sizeof(t)) < sizeof(t))
			break;

		/* 章題を読み込む */
		if (gets_rfile(rf, tmp_str, sizeof(tmp_str)) != NULL)
			if (!set_chapter_name(tmp_str))
				break;

		/* メッセージを読み込む(無視) */
		if (gets_rfile(rf, tmp_str, sizeof(tmp_str)) == NULL)
			break;

		/* 直前の継続メッセージを読み込む */
		gets_rfile(rf, tmp_str, sizeof(tmp_str));
		if (strcmp(tmp_str, "") != 0) {
			pending_message = strdup(tmp_str);
			if (pending_message == NULL) {
				log_memory();
				break;
			}
		} else {
			if (pending_message != NULL)
				free(pending_message);
			pending_message = NULL;
		}

		/* サムネイルを読み込む (読み飛ばす) */
		img_size = (size_t)(conf_save_data_thumb_width *
				    conf_save_data_thumb_height * 3);
		if (read_rfile(rf, tmp_pixels, img_size) < img_size)
			break;

		/* コマンド位置のデシリアライズを行う */
		if (!deserialize_command(rf))
			break;

		/* ステージのデシリアライズを行う */
		if (!deserialize_stage(rf))
			break;

		/* アニメのデシリアライズを行う */
		if (!deserialize_anime(rf))
			break;

		/* サウンドのデシリアライズを行う */
		if (!deserialize_sound(rf))
			break;

		/* ボリュームのデシリアライズを行う */
		if (!deserialize_volumes(rf))
			break;

		/* 変数のデシリアライズを行う */
		if (!deserialize_vars(rf))
			break;
		
		/* 名前変数のデシリアライズを行う */
		if (!deserialize_name_vars(rf))
			break;

		/* Cielの仮ステージのデシリアライズを行う */
		if (!ciel_deserialize_hook(rf))
			break;

		/* コンフィグのデシリアライズを行う */
		if (!deserialize_config_common(rf))
			break;

		/* ヒストリをクリアする */
		clear_history();

		/* 成功 */
		success = true;
	} while (0);

	/* ファイルをクローズする */
	close_rfile(rf);

	return success;
}

/* コマンド位置のデシリアライズを行う */
static bool deserialize_command(struct rfile *rf)
{
	char s[1024];
	int n, m;

	if (gets_rfile(rf, s, sizeof(s)) == NULL)
		return false;

	if (!load_script(s))
		return false;

	if (read_rfile(rf, &n, sizeof(n)) < sizeof(n))
		return false;

	if (read_rfile(rf, &m, sizeof(m)) < sizeof(m))
		return false;

	if (!move_to_command_index(n))
		return false;

	if (!set_return_point(m))
		return false;

	return true;
}

/* ステージのデシリアライズを行う */
static bool deserialize_stage(struct rfile *rf)
{
	char text[4096];
	struct image *img;
	const char *fname;
	int i, x, y, alpha, layer;

	for (i = 0; i < STAGE_LAYERS; i++) {
		/* Exclude the following layers. */
		switch (i) {
		case LAYER_MSG: continue;
		case LAYER_NAME: continue;
		case LAYER_CLICK: continue;
		case LAYER_AUTO: continue;
		case LAYER_SKIP: continue;
		case LAYER_CHB_EYE: continue;
		case LAYER_CHL_EYE: continue;
		case LAYER_CHLC_EYE: continue;
		case LAYER_CHR_EYE: continue;
		case LAYER_CHRC_EYE: continue;
		case LAYER_CHC_EYE: continue;
		case LAYER_CHF_EYE: continue;
		case LAYER_CHB_LIP: continue;
		case LAYER_CHL_LIP: continue;
		case LAYER_CHLC_LIP: continue;
		case LAYER_CHR_LIP: continue;
		case LAYER_CHRC_LIP: continue;
		case LAYER_CHC_LIP: continue;
		case LAYER_CHF_LIP: continue;
		default: break;
		}

		/* File name. */
		text[0] = '\0';
		if (gets_rfile(rf, text, sizeof(text)) == NULL)
			strcpy(text, "none");
		if (i == LAYER_BG) {
			if (strcmp(text, "none") == 0 ||
			    strcmp(text, "") == 0) {
				fname = NULL;
				img = create_initial_bg();
				if (img == NULL)
					return false;;
			} else if (text[0] == '#') {
				fname = &text[0];
				img = create_image_from_color_string(
					conf_window_width,
					conf_window_height,
					&text[1]);
				if (img == NULL)
					return false;
			} else {
				fname = &text[0];
				if (strncmp(text, "cg/", 3) == 0) {
					img = create_image_from_file(
						CG_DIR, &text[3]);
				} else {
					img = create_image_from_file(
						BG_DIR, text);
				}
				if (img == NULL)
					return false;
			}
		} else {
			const char *dir;
			switch (i) {
			case LAYER_BG2:
				dir = BG_DIR;
				break;
			case LAYER_CHB:
			case LAYER_CHL:
			case LAYER_CHLC:
			case LAYER_CHR:
			case LAYER_CHRC:
			case LAYER_CHC:
			case LAYER_CHF:
				dir = CH_DIR;
				break;
			default:
				dir = CG_DIR;
				break;
			}

			if (strcmp(text, "none") == 0 || strcmp(text, "") == 0) {
				fname = NULL;
				img = NULL;
			} else {
				fname = &text[0];
				img = create_image_from_file(dir, text);
				if (img == NULL)
					return false;
			}
		}
		set_layer_file_name(i, fname);
		set_layer_image(i, img);

		/* Position. */
		if (read_rfile(rf, &x, sizeof(x)) < sizeof(x))
			return false;
		if (read_rfile(rf, &y, sizeof(y)) < sizeof(y))
			return false;
		set_layer_position(i, x, y);

		/* Alpha. */
		if (read_rfile(rf, &alpha, sizeof(alpha)) < sizeof(alpha))
			return false;
		set_layer_alpha(i, alpha);

		/* Text. */
		if (i >= LAYER_TEXT1 && i <= LAYER_TEXT8) {
			if (gets_rfile(rf, text, sizeof(text)) != NULL)
				set_layer_text(i, text);
			else
				set_layer_text(i, NULL);
		}
	}

	for (i = 0; i < CH_ALL_LAYERS; i++) {
		layer = chpos_to_layer(i);
		fname = get_layer_file_name(layer);
		if (!load_eye_image_if_exists(i, fname))
			return false;
		if (!load_lip_image_if_exists(i, fname))
			return false;
	}

	return true;
}

/* アニメをデシリアライズする */
static bool deserialize_anime(struct rfile *rf)
{
	char text[4096];
	int i;

	for (i = 0; i < REG_ANIME_COUNT; i++) {
		if (gets_rfile(rf, text, sizeof(text)) == NULL)
			continue;
		if (strcmp(text, "none") == 0)
			continue;
		if (!load_anime_from_file(text, i, NULL))
			return false;
	}

	return true;
}

/* サウンドをデシリアライズする */
static bool deserialize_sound(struct rfile *rf)
{
	char s[1024];
	struct wave *w;

	/* BGMをデシリアライズする */
	if (gets_rfile(rf, s, sizeof(s)) == NULL)
		return false;
	if (strcmp(s, "none") == 0) {
		set_bgm_file_name(NULL);
		w = NULL;
	} else {
		set_bgm_file_name(s);
		w = create_wave_from_file(BGM_DIR, s, true);
		if (w == NULL)
			return false;
	}
	set_mixer_input(BGM_STREAM, w);

	/* SEをデシリアライズする */
	if (gets_rfile(rf, s, sizeof(s)) == NULL)
		return false;
	if (strcmp(s, "none") == 0) {
		set_se_file_name(NULL);
		w = NULL;
	} else {
		set_se_file_name(s);
		w = create_wave_from_file(SE_DIR, s, true);
		if (w == NULL)
			return false;
	}
	set_mixer_input(SE_STREAM, w);

	return true;
}

/* ボリュームをデシリアライズする */
static bool deserialize_volumes(struct rfile *rf)
{
	float vol;
	int n;

	for (n = 0; n < MIXER_STREAMS; n++) {
		if (read_rfile(rf, &vol, sizeof(vol)) < sizeof(vol))
			return false;
		set_mixer_volume(n, vol, 0);
	}

	return true;
}

/* ローカル変数をデシリアライズする */
static bool deserialize_vars(struct rfile *rf)
{
	size_t len;

	len = LOCAL_VAR_SIZE * sizeof(int32_t);
	if (read_rfile(rf, get_local_variables_pointer(), len) < len)
		return false;

	return true;
}

/* 名前変数をデシリアライズする */
static bool deserialize_name_vars(struct rfile *rf)
{
	char name[1024];
	int i;

	for (i = 0; i < NAME_VAR_SIZE; i++) {
		if (gets_rfile(rf, name, sizeof(name)) == NULL)
			return false;
		set_name_variable(i, name);
	}

	return true;
}

/* コンフィグをデシリアライズする */
static bool deserialize_config_common(struct rfile *rf)
{
	char key[1024];
	char val[1024];

	/* 終端記号が現れるまでループする */
	while (1) {
		/* ロードするキーを取得する */
		if (gets_rfile(rf, key, sizeof(key)) == NULL)
			return false;

		/* 終端記号の場合はループを終了する */
		if (strcmp(key, END_OF_CONFIG) == 0)
			break;

		/* 値を取得する(文字列として保存されている) */
		if (gets_rfile(rf, val, sizeof(val)) == NULL)
			return false;

		/* コンフィグを上書きする */
		if (!overwrite_config(key, val))
			return false;
	}

	return true;
}

/* セーブデータから基本情報を読み込む */
static void load_basic_save_data(void)
{
	struct rfile *rf;
	char buf[128];
	uint64_t t;
	int i;

	latest_index = -1;

	/* セーブスロットごとに読み込む */
	for (i = 0; i < SAVE_SLOTS; i++) {
		/* セーブデータファイルを開く */
		snprintf(buf, sizeof(buf), "%03d.sav", i);
		rf = open_rfile(SAVE_DIR, buf, true);
		if (rf != NULL) {
			/* 読み込む */
			load_basic_save_data_file(rf, i);
			close_rfile(rf);
		}
	}

	/* セーブデータファイルを開く */
	rf = open_rfile(SAVE_DIR, QUICK_SAVE_FILE, true);
	if (rf != NULL) {
		/* セーブ時刻を取得する */
		if (read_rfile(rf, &t, sizeof(t)) == sizeof(t))
			quick_save_time = (time_t)t;
		close_rfile(rf);
	}
}

/* セーブデータファイルから基本情報を読み込む */
static void load_basic_save_data_file(struct rfile *rf, int index)
{
	uint64_t t;
	size_t img_size;
	uint32_t ver;
	pixel_t *dst;
	const unsigned char *src;
	uint32_t r, g, b;
	int x, y;

	/* セーブデータのバージョンを読む */
	read_rfile(rf, &ver, sizeof(uint32_t));
	if (ver != SAVE_VER) {
		/* セーブデータの互換性がないので読み込まない */
		return;
	}

	/* セーブ時刻を取得する */
	if (read_rfile(rf, &t, sizeof(t)) < sizeof(t))
		return;
	save_time[index] = (time_t)t;
	if (latest_index == -1)
		latest_index = index;
	else if ((time_t)t > save_time[latest_index])
		latest_index = index;

	/* 章題を取得する */
	if (gets_rfile(rf, tmp_str, sizeof(tmp_str)) == NULL)
		return;
	save_title[index] = strdup(tmp_str);
	if (save_title[index] == NULL) {
		log_memory();
		return;
	}

	/* メッセージを取得する */
	if (gets_rfile(rf, tmp_str, sizeof(tmp_str)) == NULL)
		return;
	save_message[index] = strdup(tmp_str);
	if (save_message[index] == NULL) {
		log_memory();
		return;
	}

	/* 直前の継続メッセージを取得する */
	if (gets_rfile(rf, tmp_str, sizeof(tmp_str)) == NULL)
		return;

	/* サムネイルを取得する */
	img_size = (size_t)(conf_save_data_thumb_width *
			    conf_save_data_thumb_height * 3);
	if (read_rfile(rf, tmp_pixels, img_size) < img_size)
		return;

	/* サムネイルの画像を生成する */
	save_thumb[index] = create_image(conf_save_data_thumb_width,
					 conf_save_data_thumb_height);
	if (save_thumb[index] == NULL)
		return;
	dst = save_thumb[index]->pixels;
	src = tmp_pixels;
	for (y = 0; y < conf_save_data_thumb_height; y++) {
		for (x = 0; x < conf_save_data_thumb_width; x++) {
			r = *src++;
			g = *src++;
			b = *src++;
			*dst++ = make_pixel(0xff, r, g, b);
		}
	}
	notify_image_update(save_thumb[index]);
}

/*
 * グローバル変数
 */

/* グローバルデータのロードを行う */
static void load_global_data(void)
{
	struct rfile *rf;
	float f;
	uint32_t ver;
	int i;

	/* ファイルを開く */
	rf = open_rfile(SAVE_DIR, GLOBAL_SAVE_FILE, true);
	if (rf == NULL)
		return;

	/* セーブデータのバージョンを読む */
	read_rfile(rf, &ver, sizeof(uint32_t));
	if (ver != SAVE_VER) {
		/* セーブデータの互換性がないので読み込まない */
		log_save_ver();
		close_rfile(rf);
		return;
	}

	/* グローバル変数をデシリアライズする */
	read_rfile(rf, get_global_variables_pointer(),
		   GLOBAL_VAR_SIZE * sizeof(int32_t));

	/*
	 * load_global_data()はinit_mixer()より後に呼ばれる
	 */

	/* マスターボリュームをデシリアライズする */
	if (read_rfile(rf, &f, sizeof(f)) < sizeof(f))
		return;
	f = (f < 0 || f > 1.0f) ? 1.0f : f;
	set_master_volume(f);

	/* グローバルボリュームをデシリアライズする */
	for (i = 0; i < MIXER_STREAMS; i++) {
		if (read_rfile(rf, &f, sizeof(f)) < sizeof(f))
			break;
		f = (f < 0 || f > 1.0f) ? 1.0f : f;
		set_mixer_global_volume(i, f);
	}

	/* キャラクタボリュームをデシリアライズする */
	for (i = 0; i < CH_VOL_SLOTS; i++) {
		if (read_rfile(rf, &f, sizeof(f)) < sizeof(f))
			break;
		f = (f < 0 || f > 1.0f) ? 1.0f : f;
		set_character_volume(i, f);
	}

	/* テキストスピードをデシリアライズする */
	read_rfile(rf, &msg_text_speed, sizeof(f));
	msg_text_speed =
		(msg_text_speed < 0 || msg_text_speed > 10000.0f) ?
		1.0f : msg_text_speed;

	/* オートモードスピードをデシリアライズする */
	read_rfile(rf, &msg_auto_speed, sizeof(f));
	msg_auto_speed =
		(msg_auto_speed < 0 || msg_auto_speed > 10000.0f) ?
		1.0f : msg_auto_speed;

	/* コンフィグをデシリアライズする */
	deserialize_config_common(rf);

	/* ファイルを閉じる */
	close_rfile(rf);
}

/*
 * グローバルデータのセーブを行う
 */
void save_global_data(void)
{
	struct wfile *wf;
	uint32_t ver;
	float f;
	int i;

	/* セーブディレクトリを作成する */
	make_sav_dir();

	/* ファイルを開く */
	wf = open_wfile(SAVE_DIR, GLOBAL_SAVE_FILE);
	if (wf == NULL)
		return;

	/* セーブデータのバージョンを書き出す */
	ver = SAVE_VER;
	write_wfile(wf, &ver, sizeof(uint32_t));

	/* グローバル変数をシリアライズする */
	write_wfile(wf, get_global_variables_pointer(),
		    GLOBAL_VAR_SIZE * sizeof(int32_t));

	/* マスターボリュームをシリアライズする */
	f = get_master_volume();
	if (write_wfile(wf, &f, sizeof(f)) < sizeof(f))
		return;

	/* グローバルボリュームをシリアライズする */
	for (i = 0; i < MIXER_STREAMS; i++) {
		f = get_mixer_global_volume(i);
		if (write_wfile(wf, &f, sizeof(f)) < sizeof(f))
			break;
	}

	/* キャラクタボリュームをシリアライズする */
	for (i = 0; i < CH_VOL_SLOTS; i++) {
		f = get_character_volume(i);
		if (write_wfile(wf, &f, sizeof(f)) < sizeof(f))
			break;
	}

	/* テキストスピードをシリアライズする */
	write_wfile(wf, &msg_text_speed, sizeof(f));
	
	/* オートモードスピードをシリアライズする */
	write_wfile(wf, &msg_auto_speed, sizeof(f));

	/* コンフィグをデシリアライズする */
	serialize_config_helper(wf, true);

	/* ファイルを閉じる */
	close_wfile(wf);
}

/*
 * ローカルセーブデータの削除を行う
 */
void delete_local_save(int index)
{
	char s[128];

	if (index == -1)
		index = QUICK_SAVE_INDEX;

	/* セーブデータがない場合、何もしない */
	if (save_time[index] == 0)
		return;

	/* ファイル名を求める */
	snprintf(s, sizeof(s), "%03d.sav", index);

	/* セーブファイルを削除する */
	remove_file(SAVE_DIR, s);

	/* セーブデータを消去する */
	save_time[index] = 0;
	if (save_title[index] != NULL) {
		free(save_title[index]);
		save_title[index] = NULL;
	}
	if (save_message[index] != NULL) {
		free(save_message[index]);
		save_message[index] = NULL;
	}
	if (save_thumb[index] != NULL) {
		destroy_image(save_thumb[index]);
		save_thumb[index] = NULL;
	}
}

/*
 * グローバルセーブデータの削除を処理する
 */
void delete_global_save(void)
{
	/* セーブファイルを削除する */
	remove_file(SAVE_DIR, GLOBAL_SAVE_FILE);
}

/*
 * 章題と最後のメッセージ
 */

/*
 * 章題を設定する
 */
bool set_chapter_name(const char *name)
{
	free(chapter_name);

	chapter_name = strdup(name);
	if (chapter_name == NULL) {
		log_memory();
		return false;
	}

	return true;
}

/*
 * 章題を取得する
 */
const char *get_chapter_name(void)
{
	if (chapter_name == NULL)
		return "";

	return chapter_name;
}

/*
 * 最後のメッセージを設定する
 */
bool set_last_message(const char *msg, bool is_append)
{
	/* 継続行のとき */
	if (is_append) {
		char *new_text;
		size_t prev_len = 0, next_len = 0;
		if (last_message != NULL)
			prev_len = strlen(last_message);
		next_len = strlen(msg);
		new_text = malloc(prev_len + next_len + 1);
		if (new_text == NULL) {
			log_memory();
			return false;
		}
		if (last_message != NULL) {
			strcpy(new_text, last_message);
			prev_last_message = last_message;
			last_message = NULL;
			strcat(new_text, msg);
		} else {
			strcpy(new_text, msg);
		}
		last_message = new_text;
		return true;
	}

	/* 継続行でないとき */
	free(last_message);
	last_message = strdup(msg);
	if (last_message == NULL) {
		log_memory();
		return false;
	}
	free(prev_last_message);
	prev_last_message = NULL;

	return true;
}

/*
 * テキストスピードを設定する
 */
void set_text_speed(float val)
{
	assert(val >= 0 && val <= 1.0f);

	msg_text_speed = val;
}
/*
 * テキストスピードを取得する
 */
float get_text_speed(void)
{
	return msg_text_speed;
}

/*
 * オートスピードを設定する
 */
void set_auto_speed(float val)
{
	assert(val >= 0 && val <= 1.0f);

	msg_auto_speed = val;
}

/*
 * オートスピードを取得する
 */
float get_auto_speed(void)
{
	return msg_auto_speed;
}

/*
 * 最後の+en+コマンドの位置を記録する
 */
void set_last_en_command(void)
{
	last_en_command = get_command_index();
}

/*
 * 最後の+en+コマンドの位置を消去する
 */
void clear_last_en_command(void)
{
	last_en_command = -1;
}

/*
 * メッセージボックスのテキスト
 */

char *get_pending_message(void)
{
	char *ret;

	if (pending_message != NULL) {
		ret = strdup(pending_message);
		if (ret == NULL) {
			log_memory();
			return NULL;
		}
		free(pending_message);
		pending_message = NULL;
	} else {
		ret = NULL;
	}

	return ret;
}
