/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

#include "xengine.h"
#include "wms.h"

#include <time.h>

/*
 * Stage save area
 *  - FIXME: memory leaks when the app successfully exits.
 */

static bool is_stage_pushed;
static char *saved_layer_file_name[LAYER_EFFECT4 + 1];
static int saved_layer_x[LAYER_EFFECT4 + 1];
static int saved_layer_y[LAYER_EFFECT4 + 1];
static int saved_layer_alpha[LAYER_EFFECT4 + 1];

/*
 * FFI function declaration
 */

static bool s2_get_variable(struct wms_runtime *rt);
static bool s2_set_variable(struct wms_runtime *rt);
static bool s2_get_name_variable(struct wms_runtime *rt);
static bool s2_get_year(struct wms_runtime *rt);
static bool s2_get_month(struct wms_runtime *rt);
static bool s2_get_day(struct wms_runtime *rt);
static bool s2_get_hour(struct wms_runtime *rt);
static bool s2_get_minute(struct wms_runtime *rt);
static bool s2_get_second(struct wms_runtime *rt);
static bool s2_get_wday(struct wms_runtime *rt);
static bool s2_set_name_variable(struct wms_runtime *rt);
static bool s2_random(struct wms_runtime *rt);
static bool s2_round(struct wms_runtime *rt);
static bool s2_ceil(struct wms_runtime *rt);
static bool s2_floor(struct wms_runtime *rt);
static bool s2_set_config(struct wms_runtime *rt);
static bool s2_reflect_msgbox_and_namebox_config(struct wms_runtime *rt);
static bool s2_reflect_font_config(struct wms_runtime *rt);
static bool s2_clear_history(struct wms_runtime *rt);
static bool s2_clear_msgbox(struct wms_runtime *rt);
static bool s2_clear_namebox(struct wms_runtime *rt);
static bool s2_hide_msgbox(struct wms_runtime *rt);
static bool s2_hide_namebox(struct wms_runtime *rt);
static bool s2_save_global(struct wms_runtime *rt);
bool s2_push_stage(struct wms_runtime *rt);
bool s2_pop_stage(struct wms_runtime *rt);
static bool s2_remove_local_save(struct wms_runtime *rt);
static bool s2_remove_global_save(struct wms_runtime *rt);
static bool s2_reset_local_variables(struct wms_runtime *rt);
static bool s2_reset_global_variables(struct wms_runtime *rt);
static bool s2_quick_save_extra(struct wms_runtime *rt);
static bool s2_quick_load_extra(struct wms_runtime *rt);
static bool s2_play_midi(struct wms_runtime *rt);

/*
 * FFI function table 
 */

struct wms_ffi_func_tbl ffi_func_tbl[] = {
	{s2_get_variable, "s2_get_variable", {"index", NULL}},
	{s2_set_variable, "s2_set_variable", {"index", "value", NULL}},
	{s2_get_name_variable, "s2_get_name_variable", {"index", NULL}},
	{s2_get_year, "s2_get_year", {NULL}},
	{s2_get_month, "s2_get_month", {NULL}},
	{s2_get_day, "s2_get_day", {NULL}},
	{s2_get_hour, "s2_get_hour", {NULL}},
	{s2_get_minute, "s2_get_minute", {NULL}},
	{s2_get_second, "s2_get_second", {NULL}},
	{s2_get_wday, "s2_get_wday",{NULL}},
	{s2_set_name_variable, "s2_set_name_variable", {"index", "value", NULL}},
	{s2_random, "s2_random", {NULL}},
	{s2_round,"s2_round", {"value",NULL}},
	{s2_ceil,"s2_ceil", {"value",NULL}},
	{s2_floor,"s2_floor", {"value",NULL}},
	{s2_set_config, "s2_set_config", {"key", "value", NULL}},
	{s2_reflect_msgbox_and_namebox_config, "s2_reflect_msgbox_and_namebox_config", {NULL}},
	{s2_reflect_font_config, "s2_reflect_font_config", {NULL}},
	{s2_clear_history, "s2_clear_history", {NULL}},
	{s2_clear_msgbox, "s2_clear_msgbox", {NULL}},
	{s2_clear_namebox, "s2_clear_namebox", {NULL}},
	{s2_hide_msgbox, "s2_hide_msgbox", {NULL}},
	{s2_hide_namebox, "s2_hide_namebox", {NULL}},
	{s2_save_global, "s2_save_global", {NULL}},
	{s2_push_stage, "s2_push_stage", {NULL}},
	{s2_pop_stage, "s2_pop_stage", {NULL}},
	{s2_remove_local_save, "s2_remove_local_save", {"index", NULL}},
	{s2_remove_global_save, "s2_remove_global_save", {NULL}},
	{s2_reset_local_variables, "s2_reset_local_variables", {NULL}},
	{s2_reset_global_variables, "s2_reset_global_variables", {NULL}},
	{s2_quick_save_extra, "s2_quick_save_extra", {NULL}},
	{s2_quick_load_extra, "s2_quick_load_extra", {NULL}},
	{s2_play_midi, "s2_play_midi", {"file"}},
};

#define FFI_FUNC_TBL_SIZE (sizeof(ffi_func_tbl) / sizeof(ffi_func_tbl[0]))

/*
 * Forward declaration
 */

/* TODO: Declare static functions here. */

/*
 * FFI function definition
 */

/* Get the value of the specified variable. */
static bool s2_get_variable(struct wms_runtime *rt)
{
	struct wms_value *index;
	int index_i;
	int value;

	assert(rt != NULL);

	/* Get the argument pointer. */
	if (!wms_get_var_value(rt, "index", &index))
		return false;

	/* Get the argument value. */
	if (!wms_get_int_value(rt, index, &index_i))
		return false;

	/* Get the value of the x-engine variable. */
	value = get_variable(index_i);

	/* Set the return value. */
	if (!wms_make_int_var(rt, "__return", value, NULL))
		return false;

	return true;
}

/* Set the value of the specified variable. */
static bool s2_set_variable(struct wms_runtime *rt)
{
	struct wms_value *index, *value;
	int index_i, value_i;

	assert(rt != NULL);

	/* Get the argument pointers. */
	if (!wms_get_var_value(rt, "index", &index))
		return false;
	if (!wms_get_var_value(rt, "value", &value))
		return false;

	/* Get the argument values. */
	if (!wms_get_int_value(rt, index, &index_i))
		return false;
	if (!wms_get_int_value(rt, value, &value_i))
		return false;

	/* Set the value of the x-engine variable. */
	set_variable(index_i, value_i);

	return true;
}

/* Get the value of the specified name variable. */
static bool s2_get_name_variable(struct wms_runtime *rt)
{
	struct wms_value *index;
	const char *value;
	int index_i;

	assert(rt != NULL);

	/* Get the argument pointer. */
	if (!wms_get_var_value(rt, "index", &index))
		return false;

	/* Get the argument value. */
	if (!wms_get_int_value(rt, index, &index_i))
		return false;

	/* Get the value of the x-engine name variable. */
	value = get_name_variable(index_i);

	/* Set the return value. */
	if (!wms_make_str_var(rt, "__return", value, NULL))
		return false;

	return true;
}

/* Set the value of the specified name variable. */
static bool s2_set_name_variable(struct wms_runtime *rt)
{
	struct wms_value *index, *value;
	const char *value_s;
	int index_i;

	assert(rt != NULL);

	/* Get the argument pointers. */
	if (!wms_get_var_value(rt, "index", &index))
		return false;
	if (!wms_get_var_value(rt, "value", &value))
		return false;

	/* Get the argument values. */
	if (!wms_get_int_value(rt, index, &index_i))
		return false;
	if (!wms_get_str_value(rt, value, &value_s))
		return false;

	/* Set the value of the x-engine name variable. */
	if (!set_name_variable(index_i, value_s))
		return false;

	return true;
}

static bool s2_get_year(struct wms_runtime *rt)
{
	int now_year;
	time_t now_unixtime;
	struct tm *local;

	assert(rt != NULL);

	now_unixtime = time(NULL);

	local = localtime(&now_unixtime);

	now_year = local->tm_year + 1900;

	/* Set the return value. */
	if (!wms_make_int_var(rt, "__return" , now_year, NULL))
		return false;

	return true;
}

/* Returns now month*/
static bool s2_get_month(struct wms_runtime *rt)
{
	int now_month;
	time_t now_unixtime;
	struct tm *local;

	assert(rt != NULL);

	now_unixtime = time(NULL);

	local = localtime(&now_unixtime);

	now_month = local->tm_mon + 1;

	/* Set the return value. */
	if (!wms_make_int_var(rt, "__return", now_month, NULL))
		return false;

	return true;
}

/* Returns now day*/
static bool s2_get_day(struct wms_runtime *rt)
{
	int now_day;
	time_t now_unixtime;
	struct tm *local;

	assert(rt != NULL);

	now_unixtime = time(NULL);

	local = localtime(&now_unixtime);

	now_day = local->tm_mday;

	/* Set the return value. */
	if (!wms_make_int_var(rt, "__return", now_day, NULL))
		return false;

	return true;
}

/* Returns now hour*/
static bool s2_get_hour(struct wms_runtime *rt)
{
	int now_hour;
	time_t now_unixtime;
	struct tm *local;

	assert(rt != NULL);

	now_unixtime = time(NULL);

	local = localtime(&now_unixtime);

	now_hour = local->tm_hour;

	/* Set the return value. */
	if (!wms_make_int_var(rt, "__return", now_hour, NULL))
		return false;

	return true;
}

/* Returns now minute*/
static bool s2_get_minute(struct wms_runtime *rt)
{
	int now_minute;
	time_t now_unixtime;
	struct tm *local;

	assert(rt != NULL);

	now_unixtime = time(NULL);

	local = localtime(&now_unixtime);

	now_minute = local->tm_min;

	/* Set the return value. */
	if (!wms_make_int_var(rt, "__return", now_minute, NULL))
		return false;

	return true;
}

/* Returns now second*/
static bool s2_get_second(struct wms_runtime *rt)
{
	int now_second;
	time_t now_unixtime;
	struct tm *local;

	assert(rt != NULL);

	now_unixtime = time(NULL);

	local = localtime(&now_unixtime);

	now_second = local->tm_sec;

	/* Set the return value. */
	if (!wms_make_int_var(rt, "__return", now_second, NULL))
		return false;

	return true;
}

/* Returns day of week (Sunday=0) */
static bool s2_get_wday(struct wms_runtime *rt)
{
	int now_wday;
	time_t now_unixtime;
	struct tm *local;

	assert(rt != NULL);

	now_unixtime = time(NULL);

	local = localtime(&now_unixtime);

	now_wday = local->tm_wday;

	/* Set the return value. */
	if (!wms_make_int_var(rt, "__return", now_wday, NULL))
		return false;

	return true;
}

/* Returns a random number that ranges from 0 to 99999. */
static bool s2_random(struct wms_runtime *rt)
{
	int rand_value;

	assert(rt != NULL);

	rand_value = (int)((float)rand() / (float)RAND_MAX * 99999);

	/* Set the return value. */
	if (!wms_make_int_var(rt, "__return", rand_value, NULL))
		return false;

	return true;
}

/* Returns a rounded value. */
static bool s2_round(struct wms_runtime *rt)
{
	struct wms_value *value;
	double value_i;
	double value_round;

	assert(rt != NULL);

	if (!wms_get_var_value(rt, "value", &value))
		return false;

	/* Get the argument value. */
	if (!wms_get_float_value(rt, value , &value_i))
		return false;

	/* Round the value. */
	value_round = round(value_i);

	/* Set the return value. */
	if (!wms_make_int_var(rt, "__return", (int)value_round, NULL))
		return false;

	return true;
}

/* Returns a value rounded up. */
static bool s2_ceil(struct wms_runtime *rt)
{
	struct wms_value *value;
	double value_i;
	double value_round_up;

	assert(rt != NULL);

	if (!wms_get_var_value(rt, "value", &value))
		return false;

	/* Get the argument value. */
	if (!wms_get_float_value(rt, value , &value_i))
		return false;

	/* Round the value up. */
	value_round_up = ceil(value_i);

	/* Set the return value. */
	if (!wms_make_int_var(rt, "__return", (int)value_round_up, NULL))
		return false;

	return true;
}

/* Returns a value rounded down. */
static bool s2_floor(struct wms_runtime *rt)
{
	struct wms_value *value;
	double value_i;
	double value_round_down;

	assert(rt != NULL);

	if (!wms_get_var_value(rt, "value", &value))
		return false;

	/* Get the argument value. */
	if (!wms_get_float_value(rt, value , &value_i))
		return false;

	/* Round the value down. */
	value_round_down = floor(value_i);

	/* Set the return value. */
	if (!wms_make_int_var(rt, "__return", (int)value_round_down, NULL))
		return false;

	return true;
}

/* Set the value of the config. */
static bool s2_set_config(struct wms_runtime *rt)
{
	struct wms_value *key, *value;
	const char *key_s, *value_s;

	assert(rt != NULL);

	/* Get the argument pointers. */
	if (!wms_get_var_value(rt, "key", &key))
		return false;
	if (!wms_get_var_value(rt, "value", &value))
		return false;

	/* Get the argument values. */
	if (!wms_get_str_value(rt, key, &key_s))
		return false;
	if (!wms_get_str_value(rt, value, &value_s))
		return false;

	/* Set the value of the x-engine variable. */
	if (!overwrite_config(key_s, value_s))
		return false;

	return true;
}

/* Reflect the changed configs for the message box and the name box. */
static bool s2_reflect_msgbox_and_namebox_config(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	fill_namebox();
	fill_msgbox();

	return true;
}

/* Reflect the changed font configs. */
static bool s2_reflect_font_config(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	/*
	 * This function does nothing and exists for the compatibility.
	 * Now we change font in overwrite_config_font_file().
	 */

	return true;
}

/* Clear the message history. */
static bool s2_clear_history(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	/* Clear the message history. */
	clear_history();

	return true;
}

/* Clear the message box. */
static bool s2_clear_msgbox(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	/* Clear the message box. */
	fill_msgbox();

	return true;
}

/* Clear the name box. */
static bool s2_clear_namebox(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	/* Clear the name box. */
	fill_namebox();

	return true;
}

/* Hide the message box. */
static bool s2_hide_msgbox(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	/* Hide the message box. */
	show_msgbox(false);

	return true;
}

/* Hide the name box. */
static bool s2_hide_namebox(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	/* Hide the name box. */
	show_namebox(false);

	return true;
}

/* Save the global data. */
static bool s2_save_global(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	save_global_data();

	return true;
}

/* Push the stage. */
bool s2_push_stage(struct wms_runtime *rt)
{
	const char *s;
	int i;

	UNUSED_PARAMETER(rt);

	is_stage_pushed = true;

	for (i = LAYER_BG; i <= LAYER_EFFECT4; i++) {
		/* Exclude the following layers. */
		switch (i) {
		case LAYER_MSG:		/* fall-thru */
		case LAYER_NAME:	/* fall-thru */
		case LAYER_CLICK:	/* fall-thru */
		case LAYER_AUTO:	/* fall-thru */
		case LAYER_SKIP:
			continue;
		default:
			break;
		}

		saved_layer_x[i] = get_layer_x(i);
		saved_layer_y[i] = get_layer_y(i);
		saved_layer_alpha[i] = get_layer_alpha(i);

		if (saved_layer_file_name[i] != NULL) {
			free(saved_layer_file_name[i]);
			saved_layer_file_name[i] = NULL;
		}

		s = get_layer_file_name(i);
		if (s != NULL) {
			saved_layer_file_name[i] = strdup(s);
			if (saved_layer_file_name[i] == NULL) {
				log_memory();
				return false;
			}
		} else {
			saved_layer_file_name[i] = NULL;
		}
	}

	return true;
}

/* Pop the stage. */
bool s2_pop_stage(struct wms_runtime *rt)
{
	struct image *img;
	int i;

	UNUSED_PARAMETER(rt);

	if (!is_stage_pushed)
		return true;
	is_stage_pushed = false;

	for (i = LAYER_BG; i <= LAYER_EFFECT4; i++) {
		/* Exclude the following layers. */
		switch (i) {
		case LAYER_MSG:		/* fall-thru */
		case LAYER_NAME:	/* fall-thru */
		case LAYER_CLICK:	/* fall-thru */
		case LAYER_AUTO:	/* fall-thru */
		case LAYER_SKIP:
			continue;
		default:
			break;
		}

		if (i == LAYER_BG && saved_layer_file_name[i] == NULL) {
			/* Restore an empty background. */
			img = create_initial_bg();
			if (img == NULL)
				return false;
			set_layer_file_name(i, NULL);
			set_layer_image(i, img);
		} else if (i == LAYER_BG &&
			   saved_layer_file_name[i][0] == '#') {
			/* Restore a color background. */
			img = create_image_from_color_string(
				conf_window_width,
				conf_window_height,
				&saved_layer_file_name[i][1]);
			if (img == NULL)
				return false;
			if (!set_layer_file_name(i, saved_layer_file_name[i]))
				return false;
			set_layer_image(i, img);
		} else if (i == LAYER_BG) {
			/* Restore an image background. */
			if (strncmp(saved_layer_file_name[i], "cg/", 3) == 0) {
				img = create_image_from_file(
					CG_DIR,
					&saved_layer_file_name[i][3]);
			} else {
				img = create_image_from_file(
					BG_DIR,
					saved_layer_file_name[i]);
			}
			if (img == NULL)
				return false;
			if (!set_layer_file_name(i, saved_layer_file_name[i]))
				return false;
			set_layer_image(i, img);
		} else if ((i >= LAYER_CHB && i <= LAYER_CHC_EYE) ||
			   i == LAYER_CHF) {
			/* Restore an character. */
			if (saved_layer_file_name[i] != NULL) {
				img = create_image_from_file(CH_DIR,
							     saved_layer_file_name[i]);
				if (img == NULL)
					return false;
			} else {
				img = NULL;
			}
			if (!set_layer_file_name(i, saved_layer_file_name[i]))
				return false;
			set_layer_image(i, img);
		} else {
			/* Restore an image. */
			if (saved_layer_file_name[i] != NULL) {
				img = create_image_from_file(CG_DIR,
							     saved_layer_file_name[i]);
				if (img == NULL)
					return false;
			} else {
				img = NULL;
			}
			if (!set_layer_file_name(i, saved_layer_file_name[i]))
				return false;
			set_layer_image(i, img);
		}

		set_layer_position(i, saved_layer_x[i], saved_layer_y[i]);
		set_layer_alpha(i, saved_layer_alpha[i]);

		if (saved_layer_file_name[i] != NULL) {
			free(saved_layer_file_name[i]);
			saved_layer_file_name[i] = NULL;
		}
	}

	return true;
}

/* Removes all local save data. */
static bool s2_remove_local_save(struct wms_runtime *rt)
{
	struct wms_value *index;
	int index_i;

	assert(rt != NULL);

	/* Get the argument pointer. */
	if (!wms_get_var_value(rt, "index", &index))
		return false;

	/* Get the argument value. */
	if (!wms_get_int_value(rt, index, &index_i))
		return false;

	/* Delete a save file. */
	delete_local_save(index_i);

	return true;
}

/* Removes a global save data. */
static bool s2_remove_global_save(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	delete_global_save();

	return true;
}

static bool s2_reset_local_variables(struct wms_runtime *rt)
{
	int i;

	UNUSED_PARAMETER(rt);

	for (i = 0; i < LOCAL_VAR_SIZE; i++)
		set_variable(i, 0);

	return true;
}

static bool s2_reset_global_variables(struct wms_runtime *rt)
{
	int i;

	UNUSED_PARAMETER(rt);

	for (i = GLOBAL_VAR_OFFSET; i < GLOBAL_VAR_SIZE; i++)
		set_variable(i, 0);

	return true;
}

static bool s2_quick_save_extra(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	quick_save(true);

	return true;
}

static bool s2_quick_load_extra(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	quick_load(true);

	return true;
}

static bool s2_play_midi(struct wms_runtime *rt)
{
	struct wms_value *file_name;
	const char *file_name_s;

	assert(rt != NULL);

	/* Get the argument pointer. */
	if (!wms_get_var_value(rt, "file", &file_name))
		return false;

	/* Get the argument value. */
	if (!wms_get_str_value(rt, file_name, &file_name_s))
		return false;

#if defined(XENGINE_TARGET_WIN32) && defined(USE_EDITOR)
	bool play_midi(const char *dir, const char *fname);
	play_midi("bgm", file_name_s);
#endif

	return true;
}

/*
 * FFI function registration
 */

bool register_s2_functions(struct wms_runtime *rt)
{
	if (!wms_register_ffi_func_tbl(rt, ffi_func_tbl, FFI_FUNC_TBL_SIZE)) {
		log_wms_runtime_error("", 0, wms_get_runtime_error_message(rt));
		return false;
	}
	return true;
}

/*
 * For intrinsic
 */

#include <stdio.h>

/*
 * Printer. (for print() intrinsic)
 */
int wms_printf(const char *s, ...)
{
	char buf[1024];
	va_list ap;
	int ret;

	va_start(ap, s);
	ret = vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	if (strcmp(buf, "\n") == 0)
		return 0;

	log_warn(buf);
	return ret;
}
