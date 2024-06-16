/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * History management (to be moved to main.c)
 */

#include "polarisengine.h"

/* テキストのサイズ(名前とメッセージを連結するため) */
#define TEXT_SIZE	(1024)

/* 表示する履歴の数 */
#define HISTORY_SIZE	(100)

/* ヒストリ項目 */
static struct history {
	char *text;
	char *voice;
	int y_top;
	int y_bottom;
} history[HISTORY_SIZE];

/* ヒストリ項目の個数 */
static int history_count;

/* ヒストリ項目の先頭 */
static int history_index;

/* 前回格納したヒストリ項目 */
static int last_history_index;

/* 一時領域 */
static char tmp_text[TEXT_SIZE];

/*
 * 初期化
 */

/*
 * ヒストリに関する初期化処理を行う
 */
bool init_history(void)
{
#ifdef POLARIS_ENGINE_DLL
	/* DLLが再利用されたときのために初期化する */
	clear_history();
#endif

	last_history_index = -1;

	return true;
}

/*
 * ヒストリに関する終了処理を行う
 */
void cleanup_history(void)
{
	clear_history();
}

/*
 * cmd_message.cからのメッセージ登録
 */

/*
 * メッセージを登録する
 */
bool register_message(const char *name, const char *msg, const char *voice,
		      pixel_t body_color, pixel_t body_outline_color,
		      pixel_t name_color, pixel_t name_outline_color)
{
	struct history *h;
	const char *quote_prefix, *quote_start, *quote_end;

	/* 改行だけの場合などを除外する */
	if (strcmp(msg, "") == 0)
		return true;

	/* 引用符を取得する */
	quote_prefix = conf_gui_history_quote_prefix;
	quote_start = conf_gui_history_quote_start;
	quote_end = conf_gui_history_quote_end;
	if (quote_prefix == NULL)
		quote_prefix = "";
	if (quote_start == NULL)
		quote_start = conf_locale == LOCALE_JA ? U8("「") : U8(" \"");
	if (quote_end == NULL)
		quote_end = conf_locale == LOCALE_JA ? U8("」") : U8("\"");

	/* 格納位置を求める */
	h = &history[history_index];

	/* 以前の情報を消去する */
	if (h->text != NULL) {
		free(h->text);
		h->text = NULL;
	}
	if (h->voice != NULL) {
		free(h->voice);
		h->voice = NULL;
	}

	/* ボイスが指定されている場合 */
	if (voice != NULL && strcmp(voice, "") != 0) {
		h->voice = strdup(voice);
		if (h->voice == NULL) {
			log_memory();
			return false;
		}
	}

	/* 色のアルファ値をゼロにする */
	body_color &= 0xffffff;
	body_outline_color &= 0xffffff;
	name_color &= 0xffffff;
	name_outline_color &= 0xffffff;

	/* ヒストリの色を使う場合 */
	if (conf_gui_history_disable_color == 2) {
		body_color = make_pixel(0,
					(pixel_t)conf_gui_history_font_color_r,
					(pixel_t)conf_gui_history_font_color_g,
					(pixel_t)conf_gui_history_font_color_b);
		body_outline_color = make_pixel(0,
						(pixel_t)conf_gui_history_font_outline_color_r,
						(pixel_t)conf_gui_history_font_outline_color_g,
						(pixel_t)conf_gui_history_font_outline_color_b);
		name_color = body_color;
		name_outline_color = body_outline_color;
	}

	/* 名前が指定されいる場合 */
	if (name != NULL) {
		/* "名前「メッセージ」"の形式に連結して保存する */
		if (conf_locale == LOCALE_JA || conf_serif_quote) {
			/* 日本語 */
			if (!is_quote_started(msg)) {
				/* カッコがない場合 */
				snprintf(tmp_text, TEXT_SIZE,
					 "\\#{%06x}%s\\#{%06x}%s%s%s%s",
					 name_color,
					 name,
					 body_color,
					 quote_prefix,
					 quote_start,
					 msg,
					 quote_end);
			} else {
				/* すでにカッコがある場合 */
				snprintf(tmp_text, TEXT_SIZE,
					 "\\#{%06x}%s\\#{%06x}%s%s",
					 name_color,
					 name,
					 body_color,
					 quote_prefix,
					 msg);
			}
		} else {
			/* 日本語以外 */
			snprintf(tmp_text, TEXT_SIZE,
				 "\\#{%06x}%s\\#{%06x}%s: %s",
				 name_color,
				 name,
				 body_color,
				 quote_prefix,
				 msg);
		}
		h->text = strdup(tmp_text);
		if (h->text == NULL) {
			log_memory();
			return false;
		}
	} else {
		/* メッセージのみを保存する */
		h->text = strdup(msg);
		if (h->text == NULL) {
			log_memory();
			return false;
		}
	}

	/* 格納位置を更新する */
	last_history_index = history_index;
	history_index = (history_index + 1) % HISTORY_SIZE;
	history_count = (history_count + 1) >= HISTORY_SIZE ? HISTORY_SIZE :
			(history_count + 1);

	UNUSED_PARAMETER(body_outline_color);
	UNUSED_PARAMETER(name_outline_color);

    return true;
}

/*
 * メッセージを末尾に追記する
 */
bool append_message(const char *msg)
{
	struct history *h;
	char *new_text;

	/* 改行だけの場合などを除外する */
	if (strcmp(msg, "") == 0)
		return true;

	/* ヒストリがない状態で追記されたとき */
	if (last_history_index == -1) {
		last_history_index = 0;
		history[0].text = strdup("");
		if (history[0].text == NULL) {
			log_memory();
			return false;
		}
	}

	/* 追記するヒストリ項目を求める */
	h = &history[last_history_index];
	if (h->text == NULL)
		h->text = strdup("");

	/* メモリを確保する */
	new_text = malloc(strlen(h->text) + strlen(msg) + 1);
	if (new_text == NULL) {
		log_memory();
		return false;
	}

	/* 文字列をコピーする */
	strcpy(new_text, h->text);
	free(h->text);
	h->text = new_text;
	strcat(h->text, msg);

	return true;
}

/*
 * ロード時のクリア
 */

/*
 * ヒストリをクリアする
 */
void clear_history(void)
{
	int i;

	for (i = 0; i < HISTORY_SIZE; i++) {
		if (history[i].text != NULL) {
			free(history[i].text);
			history[i].text = NULL;
		}

		if (history[i].voice != NULL) {
			free(history[i].voice);
			history[i].voice = NULL;
		}
	}

	history_count = 0;
	history_index = 0;
}

/*
 * 取得
 */

/*
 * ヒストリの数を取得する
 */
int get_history_count(void)
{
	if (conf_gui_history_ignore_last)
		return history_count == 0 ? 0 : history_count - 1;

	return history_count;
}

/*
 * ヒストリのメッセージを取得する
 */
const char *get_history_message(int offset)
{
	int index;

	assert(offset >= 0);
	assert(offset < history_count);

	if (conf_gui_history_ignore_last)
		offset++;

	if (history_index - offset - 1 < 0)
		index = HISTORY_SIZE + (history_index - offset - 1);
	else
		index = history_index - offset - 1;
	assert(index >= 0 && index < history_count);

	return history[index].text;
}

/*
 * ヒストリのボイスを取得する
 */
const char *get_history_voice(int offset)
{
	int index;

	assert(offset >= 0);
	assert(offset < history_count);

	if (history_index - 1 - offset < 0)
		index = HISTORY_SIZE + (history_index - offset - 1);
	else
		index = history_index - offset - 1;
	assert(index >= 0 && index < history_count);

	return history[index].voice;
}

/*
 * セリフがカッコで始まりカッコで終わるかチェックする
 */
bool is_quoted_serif(const char *msg)
{
	struct item {
		const char *prefix;
		const char *suffix;
	} items[] = {
		{U8("（"), U8("）")},
		{U8("「"), U8("」")},
		{U8("『"), U8("』")},
		{U8("『"), U8("』")},
		{U8("︵"), U8("︶")},
		{U8("﹁"), U8("﹂")},
		{U8("﹃"), U8("﹄")}
	};

	size_t i;

	while (*msg == '\\') {
		if (*(msg + 1) == 'n' ||
		    *(msg + 1) == 'c' || *(msg + 1) == 'r' || *(msg + 1) == 'l') {
			msg += 2;
			continue;
		}
		if (*(msg + 1) != '\0' && *(msg + 2) == '{') {
			msg += 3;
			while (*msg++ != '}')
				;
			continue;
		}
		msg++;
	}

	for (i = 0; i < sizeof(items) / sizeof(struct item); i++) {
		if (strncmp(msg, items[i].prefix, strlen(items[i].prefix)) == 0 &&
		    strncmp(msg + strlen(msg) - strlen(items[i].suffix), items[i].suffix,
		    strlen(items[i].suffix)) == 0)
			return true;
	}

	return false;
}

/* セリフがカッコで始まるかチェックする */
bool is_quote_started(const char *msg)
{
	const char *items[] = {
		U8("（"),
		U8("「"),
		U8("『"),
		U8("『"),
		U8("︵"),
		U8("﹁"),
		U8("﹃"),
	};
	size_t i;

	for (i = 0; i < sizeof(items) / sizeof(const char *); i++) {
		if (strncmp(msg, items[i], strlen(items[i])) == 0)
			return true;
	}
	return false;
}

/* セリフがカッコで終わるかチェックする */
bool is_quote_ended(const char *msg)
{
	const char *items[] = {
		U8("）"),
		U8("」"),
		U8("』"),
		U8("』"),
		U8("︶"),
		U8("﹂"),
		U8("﹄"),
	};
	size_t i;

	for (i = 0; i < sizeof(items) / sizeof(const char *); i++) {
		if (strncmp(msg + strlen(msg) - strlen(items[i]), items[i], strlen(items[i])) == 0)
			return true;
	}
	return false;
}
