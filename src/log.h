/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * Logging
 */

#ifndef XENGINE_LOG_H
#define XENGINE_LOG_H

/*
 * ログを出力してよいのはメインスレッドのみとする。
 */

#define log_memory()	log_memory_helper(__FILE__, __LINE__)

void log_api_error(const char *api);
void log_audio_file_error(const char *dir, const char *file);
void log_dir_file_open(const char *dir, const char *file);
void log_file_name_case(const char *dir, const char *file);
void log_file_open(const char *fname);
void log_file_read(const char *dir, const char *file);
void log_font_file_error(const char *font);
void log_image_file_error(const char *dir, const char *file);
void log_memory_helper(const char *file, int line);
void log_package_file_error(void);
void log_duplicated_conf(const char *key);
void log_undefined_conf(const char *key);
void log_unknown_conf(const char *key);
void log_empty_conf_string(const char *key);
void log_wave_error(const char *fname);
void log_invalid_msgbox_size(void);
void log_save_ver(void);
void log_script_exec_footer(void);
void log_script_deep_include(const char *inc_name);
void log_script_command_not_found(const char *name);
void log_script_empty_serif(void);
void log_script_ch_position(const char *pos);
void log_script_fade_method(const char *method);
void log_script_label_not_found(const char *name);
void log_script_lhs_not_variable(const char *lhs);
void log_script_no_command(const char *file);
void log_script_not_variable(const char *lhs);
void log_script_non_positive_size(int val);
void log_script_too_few_param(int min, int real);
void log_script_too_many_param(int max, int real);
void log_script_op_error(const char *op);
void log_script_parse_footer(const char *file, int line, const char *buf);
void log_script_return_error(void);
void log_script_rgb_negative(int val);
void log_script_size(int size);
void log_script_switch_no_label(void);
void log_script_switch_no_item(void);
void log_script_var_index(int index);
void log_script_vol_value(float vol);
void log_script_mixer_stream(const char *stream);
void log_script_cha_accel(const char *accel);
void log_script_shake_move(const char *move);
void log_script_enable_disable(const char *param);
void log_script_final_command(void);
void log_script_param_mismatch(const char *name);
void log_script_config_not_found(const char *name);
void log_script_cha_no_image(const char *pos);
void log_script_parameter_name_not_specified(void);
void log_script_param_order_mismatch(void);
void log_script_too_many_files(void);
void log_script_close_before_break(void);
void log_video_error(const char *reason);
void log_script_choose_no_message(void);
void log_script_empty_string(void);
void log_file_write(const char *file);
void log_script_rule(void);
void log_gui_parse_char(char c);
void log_gui_parse_long_word(void);
void log_gui_parse_empty_word(void);
void log_gui_parse_invalid_eof(void);
void log_gui_unknown_global_key(const char *key);
void log_gui_too_many_buttons(void);
void log_gui_unknown_button_type(const char *type);
void log_gui_unknown_button_property(const char *key);
void log_gui_parse_property_before_type(const char *prop);
void log_gui_parse_footer(const char *file, int line);
void log_gui_image_not_loaded(void);
void log_wms_syntax_error(const char *file, int line, int column);
void log_wms_runtime_error(const char *file, int line, const char *msg);
void log_anime_layer_not_specified(const char *key);
void log_anime_parse_char(char c);
void log_anime_parse_long_word(void);
void log_anime_parse_empty_word(void);
void log_anime_parse_invalid_eof(void);
void log_anime_parse_property_before_type(const char *prop);
void log_anime_parse_footer(const char *file, int line);
void log_anime_unknown_key(const char *key);
void log_anime_long_sequence(void);
void log_invalid_layer_name(const char *name);
void log_cl_invalid_action(const char *action);

#ifdef USE_EDITOR
void log_inform_translated_commands(void);
void log_script_line_size(void);
void log_dir_not_found(const char *dir);
void log_too_many_files(void);
#endif

#endif
