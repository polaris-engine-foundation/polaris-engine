/* -*- coding: utf-8; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * The HAL for Unity.
 */

using System;
using System.Text;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using UnityEngine;
using UnityEngine.Rendering;

public class PolarisEngineScript : MonoBehaviour
{
	//
	// For Rendering
	//
	private static int viewportWidth = 1280;
	private static int viewportHeight = 720;
	private Shader _normalShader;
	private Shader _dimShader;
	private Shader _ruleShader;
	private Shader _meltShader;
	private CommandBuffer _commandBuffer;

	//
	// For Audio
	//
	private static AudioSource _audioSourceBGM;
	private static AudioSource _audioSourceSE;
	private static AudioSource _audioSourceVoice;
	private static AudioSource _audioSourceSys;

	//
	// Game Initialization
	//
	private void Awake()
	{
		// Save the instance.
		_instance = this;

		// Get the shaders.
		_normalShader = Resources.Load<Shader>("NormalShader");
		_dimShader = Resources.Load<Shader>("DimShader");
		_ruleShader = Resources.Load<Shader>("RuleShader");
		_meltShader = Resources.Load<Shader>("MeltShader");

		// Make a command buffer.
		_commandBuffer = new CommandBuffer();
		_commandBuffer.name = "FrameCommand";
		_commandBuffer.SetRenderTarget(BuiltinRenderTextureType.CameraTarget);
		_commandBuffer.SetViewport(new Rect(0, 0, viewportWidth, viewportHeight));
		_commandBuffer.SetViewMatrix(Matrix4x4.TRS(new Vector3(-1f, -1f, 0), Quaternion.identity, new Vector3(2f / viewportWidth, 2f / viewportHeight, 1f)));
		_commandBuffer.ClearRenderTarget(true, true, Color.black);

		// Make a camera.
		Camera camera = Camera.main;
		camera.clearFlags = CameraClearFlags.SolidColor;
		camera.backgroundColor = new Color(0, 0, 0, 0);
		camera.AddCommandBuffer(CameraEvent.BeforeForwardAlpha, _commandBuffer);
	}

	//
	// Frame Update
	//
	unsafe void Update()
	{
		int KEY_CONTROL = 0;
		int KEY_SPACE = 1;
		int KEY_RETURN = 2;
		int KEY_UP = 3;
		int KEY_DOWN = 4;
		int KEY_LEFT = 5;
		int KEY_RIGHT = 6;
		int KEY_ESCAPE = 7;

		// Process key down.
		if (Input.GetKeyDown(KeyCode.LeftControl))
			on_event_key_press(KEY_CONTROL);
		if (Input.GetKeyDown(KeyCode.Space))
			on_event_key_press(KEY_SPACE);
		if (Input.GetKeyDown(KeyCode.Return))
			on_event_key_press(KEY_RETURN);
		if (Input.GetKeyDown(KeyCode.UpArrow))
			on_event_key_press(KEY_UP);
		if (Input.GetKeyDown(KeyCode.DownArrow))
			on_event_key_press(KEY_DOWN);
		if (Input.GetKeyDown(KeyCode.LeftArrow))
			on_event_key_press(KEY_LEFT);
		if (Input.GetKeyDown(KeyCode.RightArrow))
			on_event_key_press(KEY_RIGHT);
		if (Input.GetKeyDown(KeyCode.Escape))
			on_event_key_press(KEY_ESCAPE);

		// Process key up.
		if (Input.GetKeyUp(KeyCode.LeftControl))
			on_event_key_release(KEY_CONTROL);
		if (Input.GetKeyUp(KeyCode.Space))
			on_event_key_release(KEY_SPACE);
		if (Input.GetKeyUp(KeyCode.Return))
			on_event_key_release(KEY_RETURN);
		if (Input.GetKeyUp(KeyCode.UpArrow))
			on_event_key_release(KEY_UP);
		if (Input.GetKeyUp(KeyCode.UpArrow))
			on_event_key_release(KEY_UP);
		if (Input.GetKeyUp(KeyCode.LeftArrow))
			on_event_key_release(KEY_LEFT);
		if (Input.GetKeyUp(KeyCode.RightArrow))
			on_event_key_release(KEY_RIGHT);
		if (Input.GetKeyUp(KeyCode.Escape))
			on_event_key_release(KEY_ESCAPE);

		// Do a frame rendering.
		_commandBuffer.Clear();
		if (on_event_frame() == 0)
		{
			// TODO
			// exit(0);
		}
	}

	//
	// ここから下のコードはC#とCを橋渡しするためのおまじないが大半で、
	// さまざまな言語処理系に精通していないと理解できないと思われるため、
	// 真剣に読む必要はありません。
	//

	//
	// The Sole Instance
	//
	private static PolarisEngineScript _instance;

	//
	// HAL delegate types.
	//
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_log_info(byte *s);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_log_warn(byte *s);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_log_error(byte *s);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_make_sav_dir();
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_make_valid_path(byte *dir, byte* fname, byte *dst, int len);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_notify_image_update(int id, int width, int height, uint *pixels);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_notify_image_free(int id);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_render_image_normal(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_render_image_add(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_render_image_dim(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_render_image_rule(int src_img, int rule_img, int threshold);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_render_image_melt(int src_img, int rule_img, int progress);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_render_image_3d_normal(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_render_image_3d_add(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_reset_lap_timer(IntPtr origin);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate Int64 delegate_get_lap_timer_millisec(IntPtr origin);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_play_sound(int stream, byte *wave);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_stop_sound(int stream);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_set_sound_volume(int stream, float vol);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate int delegate_is_sound_finished(int stream);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate int delegate_play_video(byte *fname, int is_skippable);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_stop_video();
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate int delegate_is_video_playing();
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_update_window_title();
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate int delegate_is_full_screen_supported();
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate int delegate_is_full_screen_mode();
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_enter_full_screen_mode();
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_leave_full_screen_mode();
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate IntPtr delegate_get_system_locale();
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_speak_text(byte *text);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_set_continuous_swipe_enabled(int is_enabled);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_free_shared(IntPtr p);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate IntPtr delegate_get_file_contents(byte *pFileName, int *len);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_open_save_file(byte *pFileName);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_write_save_file(int b);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_close_save_file();

	//
	// Init delegate types for C code calls.
	//
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	unsafe delegate void delegate_init_hal_func_table(IntPtr log_info,
													  IntPtr log_warn,
													  IntPtr log_error,
													  IntPtr make_sav_dir,
													  IntPtr make_valid_path,
													  IntPtr notify_image_update,
													  IntPtr notify_image_free,
													  IntPtr render_image_normal,
													  IntPtr render_image_add,
													  IntPtr render_image_dim,
													  IntPtr render_image_rule,
													  IntPtr render_image_melt,
													  IntPtr render_image_3d_normal,
													  IntPtr render_image_3d_add,
													  IntPtr reset_lap_timer,
													  IntPtr get_lap_timer_millisec,
													  IntPtr play_sound,
													  IntPtr stop_sound,
													  IntPtr set_sound_volume,
													  IntPtr is_sound_finished,
													  IntPtr play_video,
													  IntPtr stop_video,
													  IntPtr is_video_playing,
													  IntPtr update_window_title,
													  IntPtr is_full_screen_supported,
													  IntPtr is_full_screen_mode,
													  IntPtr enter_full_screen_mode,
													  IntPtr leave_full_screen_mode,
													  IntPtr get_system_locale,
													  IntPtr speak_text,
													  IntPtr set_continuous_swipe_enabled,
													  IntPtr free_shared,
													  IntPtr get_file_contents,
													  IntPtr open_save_file,
													  IntPtr write_save_file,
													  IntPtr close_save_file);
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate int delegate_init_conf();
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_init_locale_code();
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate int delegate_on_event_init();
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate void delegate_on_event_cleanup();
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] unsafe delegate int delegate_on_event_frame();

	//
	// Delegate objects for C code calls.
	//
	static delegate_log_info d_log_info;
	static delegate_log_warn d_log_warn;
	static delegate_log_error d_log_error;
	static delegate_make_sav_dir d_make_sav_dir;
	static delegate_make_valid_path d_make_valid_path;
	static delegate_notify_image_update d_notify_image_update;
	static delegate_notify_image_free d_notify_image_free;
	static delegate_render_image_normal d_render_image_normal;
	static delegate_render_image_add d_render_image_add;
	static delegate_render_image_dim d_render_image_dim;
	static delegate_render_image_rule d_render_image_rule;
	static delegate_render_image_melt d_render_image_melt;
	static delegate_render_image_3d_normal d_render_image_3d_normal;
	static delegate_render_image_3d_add d_render_image_3d_add;
	static delegate_reset_lap_timer d_reset_lap_timer;
	static delegate_get_lap_timer_millisec d_get_lap_timer_millisec;
	static delegate_play_sound d_play_sound;
	static delegate_stop_sound d_stop_sound;
	static delegate_set_sound_volume d_set_sound_volume;
	static delegate_is_sound_finished d_is_sound_finished;
	static delegate_play_video d_play_video;
	static delegate_stop_video d_stop_video;
	static delegate_is_video_playing d_is_video_playing;
	static delegate_update_window_title d_update_window_title;
	static delegate_is_full_screen_supported d_is_full_screen_supported;
	static delegate_is_full_screen_mode d_is_full_screen_mode;
	static delegate_enter_full_screen_mode d_enter_full_screen_mode;
	static delegate_leave_full_screen_mode d_leave_full_screen_mode;
	static delegate_get_system_locale d_get_system_locale;
	static delegate_speak_text d_speak_text;
	static delegate_set_continuous_swipe_enabled d_set_continuous_swipe_enabled;
	static delegate_free_shared d_free_shared;
	static delegate_get_file_contents d_get_file_contents;
	static delegate_open_save_file d_open_save_file;
	static delegate_write_save_file d_write_save_file;
	static delegate_close_save_file d_close_save_file;
	static delegate_init_hal_func_table d_init_hal_func_table;
	static delegate_init_conf d_init_conf;
	static delegate_init_locale_code d_init_locale_code;
	static delegate_on_event_init d_on_event_init;
	static delegate_on_event_cleanup d_on_event_cleanup;
	static delegate_on_event_frame d_on_event_frame;

	//
	// Delegate pointers.
	//
	static IntPtr p_log_info;
	static IntPtr p_log_warn;
	static IntPtr p_log_error;
	static IntPtr p_make_sav_dir;
	static IntPtr p_make_valid_path;
	static IntPtr p_notify_image_update;
	static IntPtr p_notify_image_free;
	static IntPtr p_render_image_normal;
	static IntPtr p_render_image_add;
	static IntPtr p_render_image_dim;
	static IntPtr p_render_image_rule;
	static IntPtr p_render_image_melt;
	static IntPtr p_render_image_3d_normal;
	static IntPtr p_render_image_3d_add;
	static IntPtr p_reset_lap_timer;
	static IntPtr p_get_lap_timer_millisec;
	static IntPtr p_play_sound;
	static IntPtr p_stop_sound;
	static IntPtr p_set_sound_volume;
	static IntPtr p_is_sound_finished;
	static IntPtr p_play_video;
	static IntPtr p_stop_video;
	static IntPtr p_is_video_playing;
	static IntPtr p_update_window_title;
	static IntPtr p_is_full_screen_supported;
	static IntPtr p_is_full_screen_mode;
	static IntPtr p_enter_full_screen_mode;
	static IntPtr p_leave_full_screen_mode;
	static IntPtr p_get_system_locale;
	static IntPtr p_speak_text;
	static IntPtr p_set_continuous_swipe_enabled;
	static IntPtr p_free_shared;
	static IntPtr p_get_file_contents;
	static IntPtr p_open_save_file;
	static IntPtr p_write_save_file;
	static IntPtr p_close_save_file;
	static IntPtr p_init_hal_func_table;
	static IntPtr p_init_conf;
	static IntPtr p_init_locale_code;
	static IntPtr p_on_event_init;
	static IntPtr p_on_event_cleanup;
	static IntPtr p_on_event_frame;

	//
	// C# Image structure.
	//
	public struct ManagedImage {
		public int width;
		public int height;
		public Color[] pixels;
		public Texture2D texture;
		public bool need_upload;
	};

	//
	// Image lists.
	//
	private static Dictionary<int, ManagedImage> imageDict = new Dictionary<int, ManagedImage>();

	//
	// Initialize the calling bridges on loading.
	//
	unsafe void Start()
	{
		GC.KeepAlive(this);

		// Set delegate objects.
		d_log_info = new delegate_log_info(log_info);
		d_log_warn = new delegate_log_warn(log_warn);
		d_log_error = new delegate_log_error(log_error);
		d_make_sav_dir = new delegate_make_sav_dir(make_sav_dir);
		d_make_valid_path = new delegate_make_valid_path(make_valid_path);
		d_notify_image_update = new delegate_notify_image_update(notify_image_update);
		d_notify_image_free = new delegate_notify_image_free(notify_image_free);
		d_render_image_normal = new delegate_render_image_normal(render_image_normal);
		d_render_image_add = new delegate_render_image_add(render_image_add);
		d_render_image_dim = new delegate_render_image_dim(render_image_dim);
		d_render_image_rule = new delegate_render_image_rule(render_image_rule);
		d_render_image_melt = new delegate_render_image_melt(render_image_melt);
		d_render_image_3d_normal = new delegate_render_image_3d_normal(render_image_3d_normal);
		d_render_image_3d_add = new delegate_render_image_3d_add(render_image_3d_add);
		d_reset_lap_timer = new delegate_reset_lap_timer(reset_lap_timer);
		d_get_lap_timer_millisec = new delegate_get_lap_timer_millisec(get_lap_timer_millisec);
		d_play_sound = new delegate_play_sound(play_sound);
		d_stop_sound = new delegate_stop_sound(stop_sound);
		d_set_sound_volume = new delegate_set_sound_volume(set_sound_volume);
		d_is_sound_finished = new delegate_is_sound_finished(is_sound_finished);
		d_play_video = new delegate_play_video(play_video);
		d_stop_video = new delegate_stop_video(stop_video);
		d_is_video_playing = new delegate_is_video_playing(is_video_playing);
		d_update_window_title = new delegate_update_window_title(update_window_title);
		d_is_full_screen_supported = new delegate_is_full_screen_supported(is_full_screen_supported);
		d_is_full_screen_mode = new delegate_is_full_screen_mode(is_full_screen_supported);
		d_enter_full_screen_mode = new delegate_enter_full_screen_mode(enter_full_screen_mode);
		d_leave_full_screen_mode = new delegate_leave_full_screen_mode(leave_full_screen_mode);
		d_get_system_locale = new delegate_get_system_locale(get_system_locale);
		d_speak_text = new delegate_speak_text(speak_text);
		d_set_continuous_swipe_enabled = new delegate_set_continuous_swipe_enabled(set_continuous_swipe_enabled);
		d_free_shared = new delegate_free_shared(free_shared);
		d_get_file_contents = new delegate_get_file_contents(get_file_contents);
		d_open_save_file = new delegate_open_save_file(open_save_file);
		d_write_save_file = new delegate_write_save_file(write_save_file);
		d_close_save_file = new delegate_close_save_file(close_save_file);
		d_init_hal_func_table = new delegate_init_hal_func_table(init_hal_func_table);
		d_init_conf = new delegate_init_conf(init_conf);
		d_init_locale_code = new delegate_init_locale_code(init_locale_code);
		d_on_event_init = new delegate_on_event_init(on_event_init);
		d_on_event_cleanup = new delegate_on_event_cleanup(on_event_cleanup);
		d_on_event_frame = new delegate_on_event_frame(on_event_frame);

		// Get function pointers by resolving the native library.
		p_log_info = Marshal.GetFunctionPointerForDelegate(d_log_info);
		p_log_warn = Marshal.GetFunctionPointerForDelegate(d_log_warn);
		p_log_error = Marshal.GetFunctionPointerForDelegate(d_log_error);
		p_make_sav_dir = Marshal.GetFunctionPointerForDelegate(d_make_sav_dir);
		p_make_valid_path = Marshal.GetFunctionPointerForDelegate(d_make_valid_path);
		p_notify_image_update = Marshal.GetFunctionPointerForDelegate(d_notify_image_update);
		p_notify_image_free = Marshal.GetFunctionPointerForDelegate(d_notify_image_free);
		p_render_image_normal = Marshal.GetFunctionPointerForDelegate(d_render_image_normal);
		p_render_image_add = Marshal.GetFunctionPointerForDelegate(d_render_image_add);
		p_render_image_dim = Marshal.GetFunctionPointerForDelegate(d_render_image_dim);
		p_render_image_rule = Marshal.GetFunctionPointerForDelegate(d_render_image_rule);
		p_render_image_melt = Marshal.GetFunctionPointerForDelegate(d_render_image_melt);
		p_render_image_3d_normal = Marshal.GetFunctionPointerForDelegate(d_render_image_3d_normal);
		p_render_image_3d_add = Marshal.GetFunctionPointerForDelegate(d_render_image_3d_add);
		p_reset_lap_timer = Marshal.GetFunctionPointerForDelegate(d_reset_lap_timer);
		p_get_lap_timer_millisec = Marshal.GetFunctionPointerForDelegate(d_get_lap_timer_millisec);
		p_play_sound = Marshal.GetFunctionPointerForDelegate(d_play_sound);
		p_stop_sound = Marshal.GetFunctionPointerForDelegate(d_stop_sound);
		p_set_sound_volume = Marshal.GetFunctionPointerForDelegate(d_set_sound_volume);
		p_is_sound_finished = Marshal.GetFunctionPointerForDelegate(d_is_sound_finished);
		p_play_video = Marshal.GetFunctionPointerForDelegate(d_play_video);
		p_stop_video = Marshal.GetFunctionPointerForDelegate(d_stop_video);
		p_is_video_playing = Marshal.GetFunctionPointerForDelegate(d_is_video_playing);
		p_update_window_title = Marshal.GetFunctionPointerForDelegate(d_update_window_title);
		p_is_full_screen_supported = Marshal.GetFunctionPointerForDelegate(d_is_full_screen_supported);
		p_is_full_screen_mode = Marshal.GetFunctionPointerForDelegate(d_is_full_screen_mode);
		p_enter_full_screen_mode = Marshal.GetFunctionPointerForDelegate(d_enter_full_screen_mode);
		p_leave_full_screen_mode = Marshal.GetFunctionPointerForDelegate(d_leave_full_screen_mode);
		p_get_system_locale = Marshal.GetFunctionPointerForDelegate(d_get_system_locale);
		p_speak_text = Marshal.GetFunctionPointerForDelegate(d_speak_text);
		p_set_continuous_swipe_enabled = Marshal.GetFunctionPointerForDelegate(d_set_continuous_swipe_enabled);
		p_free_shared = Marshal.GetFunctionPointerForDelegate(d_free_shared);
		p_get_file_contents = Marshal.GetFunctionPointerForDelegate(d_get_file_contents);
		p_open_save_file = Marshal.GetFunctionPointerForDelegate(d_open_save_file);
		p_write_save_file = Marshal.GetFunctionPointerForDelegate(d_write_save_file);
		p_close_save_file = Marshal.GetFunctionPointerForDelegate(d_close_save_file);
		p_init_hal_func_table = Marshal.GetFunctionPointerForDelegate(d_init_hal_func_table);
		p_init_conf = Marshal.GetFunctionPointerForDelegate(d_init_conf);
		p_init_locale_code = Marshal.GetFunctionPointerForDelegate(d_init_locale_code);
		p_on_event_init = Marshal.GetFunctionPointerForDelegate(d_on_event_init);
		p_on_event_cleanup = Marshal.GetFunctionPointerForDelegate(d_on_event_cleanup);
		p_on_event_frame = Marshal.GetFunctionPointerForDelegate(d_on_event_frame);

		// Lock function pointers by Alloc().
		GCHandle.Alloc(p_log_info, GCHandleType.Pinned);
		GCHandle.Alloc(p_log_warn, GCHandleType.Pinned);
		GCHandle.Alloc(p_log_error, GCHandleType.Pinned);
		GCHandle.Alloc(p_make_sav_dir, GCHandleType.Pinned);
		GCHandle.Alloc(p_make_valid_path, GCHandleType.Pinned);
		GCHandle.Alloc(p_notify_image_update, GCHandleType.Pinned);
		GCHandle.Alloc(p_notify_image_free, GCHandleType.Pinned);
		GCHandle.Alloc(p_render_image_normal, GCHandleType.Pinned);
		GCHandle.Alloc(p_render_image_add, GCHandleType.Pinned);
		GCHandle.Alloc(p_render_image_dim, GCHandleType.Pinned);
		GCHandle.Alloc(p_render_image_rule, GCHandleType.Pinned);
		GCHandle.Alloc(p_render_image_melt, GCHandleType.Pinned);
		GCHandle.Alloc(p_render_image_3d_normal, GCHandleType.Pinned);
		GCHandle.Alloc(p_render_image_3d_add, GCHandleType.Pinned);
		GCHandle.Alloc(p_reset_lap_timer, GCHandleType.Pinned);
		GCHandle.Alloc(p_get_lap_timer_millisec, GCHandleType.Pinned);
		GCHandle.Alloc(p_play_sound, GCHandleType.Pinned);
		GCHandle.Alloc(p_stop_sound, GCHandleType.Pinned);
		GCHandle.Alloc(p_set_sound_volume, GCHandleType.Pinned);
		GCHandle.Alloc(p_is_sound_finished, GCHandleType.Pinned);
		GCHandle.Alloc(p_play_video, GCHandleType.Pinned);
		GCHandle.Alloc(p_stop_video, GCHandleType.Pinned);
		GCHandle.Alloc(p_is_video_playing, GCHandleType.Pinned);
		GCHandle.Alloc(p_update_window_title, GCHandleType.Pinned);
		GCHandle.Alloc(p_is_full_screen_supported, GCHandleType.Pinned);
		GCHandle.Alloc(p_is_full_screen_mode, GCHandleType.Pinned);
		GCHandle.Alloc(p_enter_full_screen_mode, GCHandleType.Pinned);
		GCHandle.Alloc(p_leave_full_screen_mode, GCHandleType.Pinned);
		GCHandle.Alloc(p_get_system_locale, GCHandleType.Pinned);
		GCHandle.Alloc(p_speak_text, GCHandleType.Pinned);
		GCHandle.Alloc(p_set_continuous_swipe_enabled, GCHandleType.Pinned);
		GCHandle.Alloc(p_free_shared, GCHandleType.Pinned);
		GCHandle.Alloc(p_get_file_contents, GCHandleType.Pinned);
		GCHandle.Alloc(p_open_save_file, GCHandleType.Pinned);
		GCHandle.Alloc(p_write_save_file, GCHandleType.Pinned);
		GCHandle.Alloc(p_close_save_file, GCHandleType.Pinned);
		GCHandle.Alloc(p_init_locale_code, GCHandleType.Pinned);
		GCHandle.Alloc(p_init_hal_func_table, GCHandleType.Pinned);
		GCHandle.Alloc(p_init_conf, GCHandleType.Pinned);
		GCHandle.Alloc(p_init_locale_code, GCHandleType.Pinned);
		GCHandle.Alloc(p_on_event_init, GCHandleType.Pinned);
		GCHandle.Alloc(p_on_event_cleanup, GCHandleType.Pinned);
		GCHandle.Alloc(p_on_event_frame, GCHandleType.Pinned);

		// Lock function pointers by KeepAlive().
		GC.KeepAlive(p_log_info);
		GC.KeepAlive(p_log_warn);
		GC.KeepAlive(p_log_error);
		GC.KeepAlive(p_make_sav_dir);
		GC.KeepAlive(p_make_valid_path);
		GC.KeepAlive(p_notify_image_update);
		GC.KeepAlive(p_notify_image_free);
		GC.KeepAlive(p_render_image_normal);
		GC.KeepAlive(p_render_image_add);
		GC.KeepAlive(p_render_image_dim);
		GC.KeepAlive(p_render_image_rule);
		GC.KeepAlive(p_render_image_melt);
		GC.KeepAlive(p_render_image_3d_normal);
		GC.KeepAlive(p_render_image_3d_add);
		GC.KeepAlive(p_reset_lap_timer);
		GC.KeepAlive(p_get_lap_timer_millisec);
		GC.KeepAlive(p_play_sound);
		GC.KeepAlive(p_stop_sound);
		GC.KeepAlive(p_set_sound_volume);
		GC.KeepAlive(p_is_sound_finished);
		GC.KeepAlive(p_play_video);
		GC.KeepAlive(p_stop_video);
		GC.KeepAlive(p_is_video_playing);
		GC.KeepAlive(p_update_window_title);
		GC.KeepAlive(p_is_full_screen_supported);
		GC.KeepAlive(p_is_full_screen_mode);
		GC.KeepAlive(p_enter_full_screen_mode);
		GC.KeepAlive(p_leave_full_screen_mode);
		GC.KeepAlive(p_get_system_locale);
		GC.KeepAlive(p_speak_text);
		GC.KeepAlive(p_set_continuous_swipe_enabled);
		GC.KeepAlive(p_free_shared);
		GC.KeepAlive(p_get_file_contents);
		GC.KeepAlive(p_open_save_file);
		GC.KeepAlive(p_write_save_file);
		GC.KeepAlive(p_close_save_file);

		// Initialize the HAL function table.
		d_init_hal_func_table(
			p_log_info,
			p_log_warn,
			p_log_error,
			p_make_sav_dir,
			p_make_valid_path,
			p_notify_image_update,
			p_notify_image_free,
			p_render_image_normal,
			p_render_image_add,
			p_render_image_dim,
			p_render_image_rule,
			p_render_image_melt,
			p_render_image_3d_normal,
			p_render_image_3d_add,
			p_reset_lap_timer,
			p_get_lap_timer_millisec,
			p_play_sound,
			p_stop_sound,
			p_set_sound_volume,
			p_is_sound_finished,
			p_play_video,
			p_stop_video,
			p_is_video_playing,
			p_update_window_title,
			p_is_full_screen_supported,
			p_is_full_screen_mode,
			p_enter_full_screen_mode,
			p_leave_full_screen_mode,
			p_get_system_locale,
			p_speak_text,
			p_set_continuous_swipe_enabled,
			p_free_shared,
			p_get_file_contents,
			p_open_save_file,
			p_write_save_file,
			p_close_save_file);
		GC.KeepAlive(this);

		// Initialize the locale code.
		d_init_locale_code();
		GC.KeepAlive(this);

		// Initialize the config subsystem.
		d_init_conf();
		GC.KeepAlive(this);

		// Initialize the event subsystem.
		d_on_event_init();
		GC.KeepAlive(this);
	}

	//
	// Native Code
	//

	[DllImport("libpolarisengine")]
	static extern unsafe void init_hal_func_table(
		IntPtr log_info,
		IntPtr log_warn,
		IntPtr log_error,
		IntPtr make_sav_dir,
		IntPtr make_valid_path,
		IntPtr notify_image_update,
		IntPtr notify_image_free,
		IntPtr render_image_normal,
		IntPtr render_image_add,
		IntPtr render_image_dim,
		IntPtr render_image_rule,
		IntPtr render_image_melt,
		IntPtr render_image_3d_normal,
		IntPtr render_image_3d_add,
		IntPtr reset_lap_timer,
		IntPtr get_lap_timer_millisec,
		IntPtr play_sound,
		IntPtr stop_sound,
		IntPtr set_sound_volume,
		IntPtr is_sound_finished,
		IntPtr play_video,
		IntPtr stop_video,
		IntPtr is_video_playing,
		IntPtr update_window_title,
		IntPtr is_full_screen_supported,
		IntPtr is_full_screen_mode,
		IntPtr enter_full_screen_mode,
		IntPtr leave_full_screen_mode,
		IntPtr get_system_locale,
		IntPtr speak_text,
		IntPtr set_continuous_swipe_enabled,
		IntPtr free_shared,
		IntPtr get_file_contents,
		IntPtr open_save_file,
		IntPtr write_save_file,
		IntPtr close_save_file);

	[DllImport("libpolarisengine")]
	static extern unsafe void init_locale_code();

	[DllImport("libpolarisengine")]
	static extern unsafe int init_conf();

	[DllImport("libpolarisengine")]
	static extern unsafe int on_event_init();

	[DllImport("libpolarisengine")]
	static extern unsafe void on_event_cleanup();

	[DllImport("libpolarisengine")]
	static extern unsafe int on_event_frame();

	[DllImport("libpolarisengine")]
	static extern unsafe void on_event_key_press(int key);

	[DllImport("libpolarisengine")]
	static extern unsafe void on_event_key_release(int key);

	[DllImport("libpolarisengine")]
	static extern unsafe void on_event_mouse_press(int button, int x, int y);

	[DllImport("libpolarisengine")]
	static extern unsafe void on_event_mouse_release(int button, int x, int y);

	[DllImport("libpolarisengine")]
	static extern unsafe void on_event_mouse_move(int x, int y);

	[DllImport("libpolarisengine")]
	static extern unsafe void on_event_touch_cancel();

	[DllImport("libpolarisengine")]
	static extern unsafe void on_event_swipe_down();

	[DllImport("libpolarisengine")]
	static extern unsafe void on_event_swipe_up();

	[DllImport("libpolarisengine")]
	public static extern unsafe int get_wave_samples(byte *w, uint *buf, int samples);

	[DllImport("libpolarisengine")]
	public static extern unsafe bool is_wave_eos(byte *w);

	//
	// HAL functions
	//

	[AOT.MonoPInvokeCallback(typeof(delegate_log_info))]
	static unsafe void log_info(byte *s)
	{
		string str = BytePtrToString(s);
		Debug.Log(str);
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_log_warn))]
	static unsafe void log_warn(byte *s)
	{
		string str = BytePtrToString(s);
		Debug.Log(str);
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_log_error))]
	static unsafe void log_error(byte *s)
	{
		string str = BytePtrToString(s);
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_make_sav_dir))]
	static unsafe void make_sav_dir()
	{
		// Do nothing.
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_make_valid_path))]
	static unsafe void make_valid_path(byte *dir, byte *fname, byte *dst, int len)
	{
		string Path = "";
		if (dir != null)
			Path = Path + BytePtrToString(dir) + "/";
		if (fname != null)
			Path = Path + BytePtrToString(fname);

		byte[] bytes = System.Text.Encoding.UTF8.GetBytes(Path);
		for (int i = 0; i < bytes.Length && i < len; i++)
		{
			dst[i] = bytes[i];
		}
		dst[bytes.Length] = 0;
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_notify_image_update))]
	static unsafe void notify_image_update(int id, int width, int height, uint *pixels)
	{
		if (!imageDict.ContainsKey(id))
		{
			ManagedImage storeImage = new ManagedImage();
			storeImage.width = width;
			storeImage.height = height;
			storeImage.pixels = new Color[storeImage.width * storeImage.height];
			storeImage.texture = new Texture2D(storeImage.width, storeImage.height, TextureFormat.ARGB32, false);
			storeImage.need_upload = true;
			imageDict.Add(id, storeImage);
		}

		ManagedImage dstImage = imageDict[id];
		for (int y = 0; y < dstImage.height; y++)
		{
			for (int x = 0; x < dstImage.width; x++)
			{
				uint p = pixels[y * dstImage.width + x];
				Color cl = new Color(((p >> 16) & 0xff) / 255.0f,
				 	  	   	   		 ((p >> 8) & 0xff) / 255.0f,
				 					 (p & 0xff) / 255.0f,
				 					 ((p >> 24) & 0xff) / 255.0f);
				dstImage.pixels[y * dstImage.width + x] = cl;
			}
		}
		dstImage.need_upload = true;
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_notify_image_free))]
	static unsafe void notify_image_free(int id)
	{
		if (imageDict.ContainsKey(id))
		{
			ManagedImage image = imageDict[id];
			MonoBehaviour.Destroy(image.texture);
			imageDict.Remove(id);
		}
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_render_image_normal))]
	static unsafe void render_image_normal(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha)
	{
		ManagedImage srcImage = imageDict[src_img];
		if (srcImage.need_upload)
		{
			srcImage.texture.SetPixels(srcImage.pixels, 0);
			srcImage.texture.Apply();
			srcImage.need_upload = false;
		}

		Vector3[] vertices = new Vector3[] {
			new Vector3(dst_left / 1280.0f - 0.5f, 0.5f - dst_top / 720.0f, 0),
			new Vector3((dst_left + dst_width) / 1280.0f - 0.5f, 0.5f - dst_top / 720.0f, 0),
			new Vector3(dst_left / 1280.0f - 0.5f, 0.5f - (dst_top + dst_height) / 720.0f, 0),
			new Vector3((dst_left + dst_width) / 1280.0f - 0.5f, 0.5f - (dst_top + dst_height) / 720.0f, 0)
		};

		Vector2[] uv = new Vector2[] {
			new Vector2((float)src_left / (float)srcImage.width, (float)src_top / (float)srcImage.height),
			new Vector2((float)(src_left + src_width) / (float)srcImage.width, (float)src_top / (float)srcImage.height),
			new Vector2((float)src_left / (float)srcImage.width, (float)(src_top + src_height) / (float)srcImage.height),
			new Vector2((float)(src_left + src_width) / (float)srcImage.width, (float)(src_top + src_height) / (float)srcImage.height)
		};

		Color[] colors = new Color[] {
			new Color(0, 0, 0, alpha / 255.0f),
			new Color(0, 0, 0, alpha / 255.0f),
			new Color(0, 0, 0, alpha / 255.0f),
			new Color(0, 0, 0, alpha / 255.0f)
		};

		int[] triangles = new int[] {0, 1, 2, 1, 3, 2};

		Vector3[] normals = new Vector3[] {
			new Vector3(0, 0, -1),
			new Vector3(0, 0, -1),
			new Vector3(0, 0, -1),
			new Vector3(0, 0, -1)
		};

		Material material = new Material(_instance._normalShader);
		material.mainTexture = srcImage.texture;

		Mesh mesh = new Mesh();
		mesh.vertices = vertices;
		mesh.triangles = triangles;
		mesh.uv = uv;
		mesh.colors = colors;
		mesh.normals = normals;
		mesh.RecalculateBounds();

		_instance._commandBuffer.DrawMesh(mesh, Matrix4x4.identity, material);
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_render_image_add))]
	static unsafe void render_image_add(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha)
	{
		// TODO
		render_image_normal(dst_left, dst_top, dst_width, dst_height, src_img, src_left, src_top, src_width, src_height, alpha);
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_render_image_dim))]
	static unsafe void render_image_dim(int dst_left, int dst_top, int dst_width, int dst_height, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha)
	{
		ManagedImage srcImage = imageDict[src_img];
		if (srcImage.need_upload)
		{
			srcImage.texture.SetPixels(srcImage.pixels, 0);
			srcImage.texture.Apply();
			srcImage.need_upload = false;
		}

		Vector3[] vertices = new Vector3[] {
			new Vector3(dst_left / 1280.0f - 0.5f, 0.5f - dst_top / 720.0f, 0),
			new Vector3((dst_left + dst_width) / 1280.0f - 0.5f, 0.5f - dst_top / 720.0f, 0),
			new Vector3(dst_left / 1280.0f - 0.5f, 0.5f - (dst_top + dst_height) / 720.0f, 0),
			new Vector3((dst_left + dst_width) / 1280.0f - 0.5f, 0.5f - (dst_top + dst_height) / 720.0f)
		};

		Vector2[] uv = new Vector2[] {
			new Vector2((float)src_left / (float)srcImage.width, (float)src_top / (float)srcImage.height),
			new Vector2((float)(src_left + src_width) / (float)srcImage.width, (float)src_top / (float)srcImage.height),
			new Vector2((float)src_left / (float)srcImage.width, (float)(src_top + src_height) / (float)srcImage.height),
			new Vector2((float)(src_left + src_width) / (float)srcImage.width, (float)(src_top + src_height) / (float)srcImage.height)
		};

		Color[] colors = new Color[] {
			new Color(0, 0, 0, alpha / 255.0f),
			new Color(0, 0, 0, alpha / 255.0f),
			new Color(0, 0, 0, alpha / 255.0f),
			new Color(0, 0, 0, alpha / 255.0f)
		};

		int[] triangles = new int[] {0, 1, 2, 1, 3, 2};

		Vector3[] normals = new Vector3[] {
			new Vector3(0, 0, -1),
			new Vector3(0, 0, -1),
			new Vector3(0, 0, -1),
			new Vector3(0, 0, -1)
		};

		Material material = new Material(_instance._dimShader);
		material.mainTexture = srcImage.texture;

		Mesh mesh = new Mesh();
		mesh.vertices = vertices;
		mesh.triangles = triangles;
		mesh.uv = uv;
		mesh.colors = colors;
		mesh.normals = normals;
		mesh.RecalculateBounds();

		_instance._commandBuffer.DrawMesh(mesh, Matrix4x4.identity, material);
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_render_image_rule))]
	static unsafe void render_image_rule(int src_img, int rule_img, int threshold)
	{
		ManagedImage srcImage = imageDict[src_img];
		if (srcImage.need_upload)
		{
			srcImage.texture.SetPixels(srcImage.pixels, 0);
			srcImage.texture.Apply();
			srcImage.need_upload = false;
		}

		ManagedImage ruleImage = imageDict[rule_img];
		if (ruleImage.need_upload)
		{
			ruleImage.texture.SetPixels(ruleImage.pixels, 0);
			ruleImage.texture.Apply();
			ruleImage.need_upload = false;
		}

		Vector3[] vertices = new Vector3[] {
			new Vector3(-0.5f, 0.5f, 0),
			new Vector3(0.5f, 0.5f, 0),
			new Vector3(-0.5f, -0.5f, 0),
			new Vector3(0.5f, -0.5f, 0)
		};

		Vector2[] uv = new Vector2[] {
			new Vector2(0, 0),
			new Vector2(1, 0),
			new Vector2(0, 1),
			new Vector2(1, 1)
		};

		Color[] colors = new Color[] {
			new Color(0, 0, 0, alpha / 255.0f),
			new Color(0, 0, 0, alpha / 255.0f),
			new Color(0, 0, 0, alpha / 255.0f),
			new Color(0, 0, 0, alpha / 255.0f)
		};

		int[] triangles = new int[] {0, 1, 2, 1, 3, 2};

		Vector3[] normals = new Vector3[] {
			new Vector3(0, 0, -1),
			new Vector3(0, 0, -1),
			new Vector3(0, 0, -1),
			new Vector3(0, 0, -1)
		};

		Material material = new Material(_instance._ruleShader);
		material.mainTexture = srcImage.texture;
		material.bumpMap = ruleImage.texture;

		Mesh mesh = new Mesh();
		mesh.vertices = vertices;
		mesh.triangles = triangles;
		mesh.uv = uv;
		mesh.colors = colors;
		mesh.normals = normals;
		mesh.RecalculateBounds();

		_instance._commandBuffer.DrawMesh(mesh, Matrix4x4.identity, material);
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_render_image_melt))]
	static unsafe void render_image_melt(int src_img, int rule_img, int progress)
	{
		ManagedImage srcImage = imageDict[src_img];
		if (srcImage.need_upload)
		{
			srcImage.texture.SetPixels(srcImage.pixels, 0);
			srcImage.texture.Apply();
			srcImage.need_upload = false;
		}

		ManagedImage ruleImage = imageDict[rule_img];
		if (ruleImage.need_upload)
		{
			ruleImage.texture.SetPixels(ruleImage.pixels, 0);
			ruleImage.texture.Apply();
			ruleImage.need_upload = false;
		}

		Vector3[] vertices = new Vector3[] {
			new Vector3(-0.5f, 0.5f, 0),
			new Vector3(0.5f, 0.5f, 0),
			new Vector3(-0.5f, -0.5f, 0),
			new Vector3(0.5f, -0.5f, 0)
		};

		Vector2[] uv = new Vector2[] {
			new Vector2(0, 0),
			new Vector2(1, 0),
			new Vector2(0, 1),
			new Vector2(1, 1)
		};

		Color[] colors = new Color[] {
			new Color(0, 0, 0, alpha / 255.0f),
			new Color(0, 0, 0, alpha / 255.0f),
			new Color(0, 0, 0, alpha / 255.0f),
			new Color(0, 0, 0, alpha / 255.0f)
		};

		int[] triangles = new int[] {0, 1, 2, 1, 3, 2};

		Vector3[] normals = new Vector3[] {
			new Vector3(0, 0, -1),
			new Vector3(0, 0, -1),
			new Vector3(0, 0, -1),
			new Vector3(0, 0, -1)
		};

		Material material = new Material(_instance._meltShader);
		material.mainTexture = srcImage.texture;
		material.bumpMap = ruleImage.texture;

		Mesh mesh = new Mesh();
		mesh.vertices = vertices;
		mesh.triangles = triangles;
		mesh.uv = uv;
		mesh.colors = colors;
		mesh.normals = normals;
		mesh.RecalculateBounds();

		_instance._commandBuffer.DrawMesh(mesh, Matrix4x4.identity, material);
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_render_image_3d_normal))]
	static unsafe void render_image_3d_normal(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha)
	{
		ManagedImage srcImage = imageDict[src_img];
		if (srcImage.need_upload)
		{
			srcImage.texture.SetPixels(srcImage.pixels, 0);
			srcImage.texture.Apply();
			srcImage.need_upload = false;
		}

		Vector3[] vertices = new Vector3[] {
			new Vector3(x1 / 1280.0f - 0.5f, 0.5f - y1 / 720.0f, 0),
			new Vector3(x2 / 1280.0f - 0.5f, 0.5f - y2 / 720.0f, 0),
			new Vector3(x3 / 1280.0f - 0.5f, 0.5f - y3 / 720.0f, 0),
			new Vector3(x4 / 1280.0f - 0.5f, 0.5f - y4 / 720.0f, 0)
		};

		Vector2[] uv = new Vector2[] {
			new Vector2((float)x1 / (float)srcImage.width, (float)y1 / (float)srcImage.height),
			new Vector2((float)x2 / (float)srcImage.width, (float)y2 / (float)srcImage.height),
			new Vector2((float)x3 / (float)srcImage.width, (float)y3 / (float)srcImage.height),
			new Vector2((float)x4 / (float)srcImage.width, (float)y4 / (float)srcImage.height)
		};

		Color[] colors = new Color[] {
			new Color(0, 0, 0, alpha / 255.0f),
			new Color(0, 0, 0, alpha / 255.0f),
			new Color(0, 0, 0, alpha / 255.0f),
			new Color(0, 0, 0, alpha / 255.0f)
		};

		int[] triangles = new int[] {0, 1, 2, 1, 3, 2};

		Vector3[] normals = new Vector3[] {
			new Vector3(0, 0, -1),
			new Vector3(0, 0, -1),
			new Vector3(0, 0, -1),
			new Vector3(0, 0, -1)
		};

		Material material = new Material(_instance._normalShader);
		material.mainTexture = srcImage.texture;

		Mesh mesh = new Mesh();
		mesh.vertices = vertices;
		mesh.triangles = triangles;
		mesh.uv = uv;
		mesh.colors = colors;
		mesh.normals = normals;
		mesh.RecalculateBounds();

		_instance._commandBuffer.DrawMesh(mesh, Matrix4x4.identity, material);
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_render_image_3d_add))]
	static unsafe void render_image_3d_add(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int src_img, int src_left, int src_top, int src_width, int src_height, int alpha)
	{
		// TODO
		render_image_3d_normal(x1, y1, x2, y2, x3, y3, x4, y4, src_img, src_left, src_top, src_width, src_height, alpha);
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_reset_lap_timer))]
	static unsafe void reset_lap_timer(IntPtr origin)
	{
		Marshal.WriteInt64(origin, Environment.TickCount);
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_get_lap_timer_millisec))]
	static unsafe Int64 get_lap_timer_millisec(IntPtr origin)
	{
		Int64 ret = Environment.TickCount - Marshal.ReadInt64(origin);
		return ret;
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_play_sound))]
	static unsafe void play_sound(int stream, byte *wave)
	{
		if (stream == 0)
			GameObject.Find("BGM").GetComponent<PolarisEngineAudio>().SetSource(wave);
		else if (stream == 1)
			GameObject.Find("SE").GetComponent<PolarisEngineAudio>().SetSource(wave);
		else if (stream == 2)
			GameObject.Find("CV").GetComponent<PolarisEngineAudio>().SetSource(wave);
		else
			GameObject.Find("SYSSE").GetComponent<PolarisEngineAudio>().SetSource(wave);
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_stop_sound))]
	static unsafe void stop_sound(int stream)
	{
		if (stream == 0)
			GameObject.Find("BGM").GetComponent<PolarisEngineAudio>().SetSource(null);
		else if (stream == 1)
			GameObject.Find("SE").GetComponent<PolarisEngineAudio>().SetSource(null);
		else if (stream == 2)
			GameObject.Find("CV").GetComponent<PolarisEngineAudio>().SetSource(null);
		else
			GameObject.Find("SYSSE").GetComponent<PolarisEngineAudio>().SetSource(null);
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_set_sound_volume))]
	static unsafe void set_sound_volume(int stream, float vol)
	{
		// TODO
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_is_sound_finished))]
	static unsafe int is_sound_finished(int stream)
	{
		return 1;
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_play_video))]
	static unsafe int play_video(byte *fname, int is_skippable)
	{
		// TODO
		return 1;
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_stop_video))]
	static unsafe void stop_video()
	{
		// TODO
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_is_video_playing))]
	static unsafe int is_video_playing()
	{
		// TODO
		return 0;
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_update_window_title))]
	static unsafe void update_window_title()
	{
		// TODO
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_is_full_screen_supported))]
	static unsafe int is_full_screen_supported()
	{
		// TODO
		return 0;
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_is_full_screen_mode))]
	static unsafe int is_full_screen_mode()
	{
		// TODO
		return 0;
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_enter_full_screen_mode))]
	static unsafe void enter_full_screen_mode()
	{
		// TODO
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_leave_full_screen_mode))]
	static unsafe void leave_full_screen_mode()
	{
		// TODO
	}

	static unsafe IntPtr locale = IntPtr.Zero;

	[AOT.MonoPInvokeCallback(typeof(delegate_get_system_locale))]
	static unsafe IntPtr get_system_locale()
	{
		// TODO
		if (locale == null)
		{
			locale = Marshal.StringToCoTaskMemUTF8("ja");
			GCHandle.Alloc(locale, GCHandleType.Pinned);
			GC.KeepAlive(locale);
		}
		return locale;
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_speak_text))]
	static unsafe void speak_text(byte *text)
	{
		// TODO
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_set_continuous_swipe_enabled))]
	static unsafe void set_continuous_swipe_enabled(int is_enabled)
	{
		// TODO
	}

	//
	// HAL helpers
	//

	static unsafe string SaveFile;
	static unsafe string SaveData;

    [AOT.MonoPInvokeCallback(typeof(delegate_free_shared))]
	static unsafe void free_shared(IntPtr p)
	{
		Marshal.FreeCoTaskMem(p);
	}

    [AOT.MonoPInvokeCallback(typeof(delegate_get_file_contents))]
    static unsafe IntPtr get_file_contents(byte *pFileName, int *len)
	{
		IntPtr ret = IntPtr.Zero;
		string FileName = BytePtrToString(pFileName);
		if (FileName.StartsWith("sav/"))
		{
			string s = PlayerPrefs.GetString(FileName.Split("/")[1], "");
			if (s == "")
				return IntPtr.Zero;
			byte[] base64Utf8 = Convert.FromBase64String(s);
			if (base64Utf8 == null)
				return IntPtr.Zero;
			ret = Marshal.AllocCoTaskMem(base64Utf8.Length);
			Marshal.Copy(base64Utf8, 0, ret, base64Utf8.Length);
			*len = base64Utf8.Length;
		}
		else
		{
			try
			{
				byte[] fileBody = System.IO.File.ReadAllBytes(Application.streamingAssetsPath + "/" + FileName);
				if (fileBody == null)
					return IntPtr.Zero;
				ret = Marshal.AllocCoTaskMem(fileBody.Length);
				Marshal.Copy(fileBody, 0, ret, fileBody.Length);
				*len = fileBody.Length;
			}
			catch(Exception)
			{
			}
		}

		GC.KeepAlive(ret);

		return ret;
    }

    private static unsafe string BytePtrToString(byte *s)
	{
		byte *b = s;
		int len = 0;
		while (*b != 0)
		{
			len++;
			b++;
		}
		byte[] managed = new byte[len];
		for (int i = 0; i < len; i++)
		{
			managed[i] = *s++;
		}
		string ret = Encoding.UTF8.GetString(managed);
 		return ret;
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_open_save_file))]
    static unsafe void open_save_file(byte *pFileName) {
		SaveFile = BytePtrToString(pFileName).Split("/")[1];
		SaveData = "";
	}

	[AOT.MonoPInvokeCallback(typeof(delegate_write_save_file))]
    static unsafe void write_save_file(int b) {
		byte[] InArray = new byte[1];
		char[] OutArray = new char[1];
		InArray[0] = 1;
		SaveData = SaveData + Convert.ToBase64CharArray(InArray, 0, 1, OutArray, 0, 0).ToString();
    }

	[AOT.MonoPInvokeCallback(typeof(delegate_close_save_file))]
    static unsafe void close_save_file() {
		string s = PlayerPrefs.GetString(SaveFile, SaveData);
    }
}
