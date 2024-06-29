/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * The HAL for X11
 */

/*
 * Xlib
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/Xatom.h>
#include <X11/Xlocale.h>

/*
 * POSIX
 */
#include <sys/types.h>
#include <sys/stat.h>	/* stat(), mkdir() */
#include <sys/time.h>	/* gettimeofday() */
#include <unistd.h>	/* usleep(), access() */

/* Base */
#include "../polarisengine.h"

/*
 * HAL implementation for OpenGL 3.0
 */
#include <GL/gl.h>
#include <GL/glx.h>
#include "../khronos/glrender.h"

/* HAL implementation for ALSA sound */
#include "asound.h"

/* HAL implementation for Gstreamer video */
#include "gstplay.h"

/* App Icon */
#include "icon.xpm"

/*
 * Color Format
 */
#define DEPTH		(24)
#define BPP		(32)

/*
 * Frame Time
 */
#define FRAME_MILLI	(16)	/* 1フレームの時間 */
#define SLEEP_MILLI	(5)	/* 1回にスリープする時間 */

/*
 * GLX Objects
 */
static GLXWindow glx_window = None;
static GLXContext glx_context = None;

/*
 * OpenGL 3.0 API
 */
GLuint (APIENTRY *glCreateShader)(GLenum type);
void (APIENTRY *glShaderSource)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
void (APIENTRY *glCompileShader)(GLuint shader);
void (APIENTRY *glGetShaderiv)(GLuint shader, GLenum pname, GLint *params);
void (APIENTRY *glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
void (APIENTRY *glAttachShader)(GLuint program, GLuint shader);
void (APIENTRY *glLinkProgram)(GLuint program);
void (APIENTRY *glGetProgramiv)(GLuint program, GLenum pname, GLint *params);
void (APIENTRY *glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GLuint (APIENTRY *glCreateProgram)(void);
void (APIENTRY *glUseProgram)(GLuint program);
void (APIENTRY *glGenVertexArrays)(GLsizei n, GLuint *arrays);
void (APIENTRY *glBindVertexArray)(GLuint array);
void (APIENTRY *glGenBuffers)(GLsizei n, GLuint *buffers);
void (APIENTRY *glBindBuffer)(GLenum target, GLuint buffer);
GLint (APIENTRY *glGetAttribLocation)(GLuint program, const GLchar *name);
void (APIENTRY *glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
void (APIENTRY *glEnableVertexAttribArray)(GLuint index);
GLint (APIENTRY *glGetUniformLocation)(GLuint program, const GLchar *name);
void (APIENTRY *glUniform1i)(GLint location, GLint v0);
void (APIENTRY *glBufferData)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
void (APIENTRY *glDeleteShader)(GLuint shader);
void (APIENTRY *glDeleteProgram)(GLuint program);
void (APIENTRY *glDeleteVertexArrays)(GLsizei n, const GLuint *arrays);
void (APIENTRY *glDeleteBuffers)(GLsizei n, const GLuint *buffers);
#if 0
void (APIENTRY *glActiveTexture)(GLenum texture);
#endif
struct API
{
	void **func;
	const char *name;
};
static struct API api[] =
{
	{(void **)&glCreateShader, "glCreateShader"},
	{(void **)&glShaderSource, "glShaderSource"},
	{(void **)&glCompileShader, "glCompileShader"},
	{(void **)&glGetShaderiv, "glGetShaderiv"},
	{(void **)&glGetShaderInfoLog, "glGetShaderInfoLog"},
	{(void **)&glAttachShader, "glAttachShader"},
	{(void **)&glLinkProgram, "glLinkProgram"},
	{(void **)&glGetProgramiv, "glGetProgramiv"},
	{(void **)&glGetProgramInfoLog, "glGetProgramInfoLog"},
	{(void **)&glCreateProgram, "glCreateProgram"},
	{(void **)&glUseProgram, "glUseProgram"},
	{(void **)&glGenVertexArrays, "glGenVertexArrays"},
	{(void **)&glBindVertexArray, "glBindVertexArray"},
	{(void **)&glGenBuffers, "glGenBuffers"},
	{(void **)&glBindBuffer, "glBindBuffer"},
	{(void **)&glGetAttribLocation, "glGetAttribLocation"},
	{(void **)&glVertexAttribPointer, "glVertexAttribPointer"},
	{(void **)&glEnableVertexAttribArray, "glEnableVertexAttribArray"},
	{(void **)&glGetUniformLocation, "glGetUniformLocation"},
	{(void **)&glUniform1i, "glUniform1i"},
	{(void **)&glBufferData, "glBufferData"},
	{(void **)&glDeleteShader, "glDeleteShader"},
	{(void **)&glDeleteProgram, "glDeleteProgram"},
	{(void **)&glDeleteVertexArrays, "glDeleteVertexArrays"},
	{(void **)&glDeleteBuffers, "glDeleteBuffers"},
#if 0
	{(void **)&glActiveTexture, "glActiveTexture"},
#endif
};

/*
 * X11 Objects
 */
static Display *display;
static Window window = BadAlloc;
static Pixmap icon = BadAlloc;
static Pixmap icon_mask = BadAlloc;
static Atom delete_message = BadAlloc;

/* Frame Start Time */
static struct timeval tv_start;

/*
 * Log File
 */
#define LOG_BUF_SIZE	(4096)
static FILE *log_fp;

/*
 * Window Title Buffer
 */
#define TITLE_BUF_SIZE	(1024)
static char title_buf[TITLE_BUF_SIZE];

/* Flag to indicate whether we are playing a video or not */
static bool is_gst_playing;

/* Flag to indicate whether a video is skippable or not */
static bool is_gst_skippable;

/*
 * forward declaration
 */
static bool init_subsystems(int argc, char *argv[]);
static void cleanup_subsystems(void);
static bool open_log_file(void);
static void close_log_file(void);
static bool open_display(void);
static void close_display(void);
static bool create_window(void);
static void destroy_window(void);
static bool create_icon_image(void);
static void destroy_icon_image(void);
static bool init_glx(void);
static void cleanup_glx(void);
static void run_game_loop(void);
static bool run_frame(void);
static bool wait_for_next_frame(void);
static bool next_event(void);
static void event_key_press(XEvent *event);
static void event_key_release(XEvent *event);
static int get_key_code(XEvent *event);
static void event_button_press(XEvent *event);
static void event_button_release(XEvent *event);
static void event_motion_notify(XEvent *event);

/*
 * Main
 */
int main(int argc, char *argv[])
{
	int ret;

	setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "C");

	/* Initialize all subsystems. */
	if (init_subsystems(argc, argv)) {
		/* Initialize app. */
		if (on_event_init()) {
			/* Run game loop. */
			run_game_loop();

			/* Success. */
			ret = 0;
		} else {
			/* Failure. */
			ret = 1;
		}

		/* Cleanup app. */
		on_event_cleanup();
	} else {
		/* Print an error message. */
		if (log_fp != NULL)
			printf("Check " LOG_FILE "\n");

		/* Failure. */
		ret = 1;
	}

	/* Cleanup the subsystems. */
	cleanup_subsystems();

	return ret;
}

/* Initialize the subsystems. */
static bool init_subsystems(int argc, char *argv[])
{
	/* Initialize locale. */
	init_locale_code();

	/* Open the log file. */
	if (!open_log_file())
		return false;

	/* Initialize the file subsystem. */
	if (!init_file())
		return false;

	/* Initialize the config subsystem. */
	if (!init_conf())
		return false;

	/* Initialize the ALSA sound output. */
	if (!init_asound())
		log_warn("Can't initialize sound.\n");

	/* Open an X11 display. */
	if (!open_display()) {
		log_error("Can't open display.\n");
		return false;
	}

	/* Initialize GLX. */
	if (!init_glx()) {
		log_error("Failed to initialize OpenGL.");
		return false;
	}

	/* Create a window. */
	if (!create_window()) {
		log_error("Can't open window.\n");
		return false;
	}

	/* Create an icon. */
	if (!create_icon_image()) {
		log_error("Can't create icon.\n");
		return false;
	}

	/* Initialize Gstreamer. */
	gstplay_init(argc, argv);

	return true;
}

/* Cleanup the subsystems. */
static void cleanup_subsystems(void)
{
	/* Cleanup ALSA. */
	cleanup_asound();

	/* Cleanup GLX. */
	cleanup_glx();

	/* Destroy the window. */
	destroy_window();

	/* Destroy the icon. */
	destroy_icon_image();

	/* Close the display. */
	close_display();

	/* Cleanup the config subsystem. */
	cleanup_conf();

	/* Cleanup the file subsystem. */
	cleanup_file();

	/* Close the log file. */
	close_log_file();
}

/*
 * Logging
 */

/* Open the log file. */
static bool open_log_file(void)
{
	if (log_fp == NULL) {
		log_fp = fopen(LOG_FILE, "w");
		if (log_fp == NULL) {
			printf("Can't open log file.\n");
			return false;
		}
	}
	return true;
}

/* Close the log file. */
static void close_log_file(void)
{
	if (log_fp != NULL)
		fclose(log_fp);
}

/*
 * X11
 */

/* Open an X11 display. */
static bool open_display(void)
{
	display = XOpenDisplay(NULL);
	if (display == NULL) {
		log_api_error("XOpenDisplay");
		return false;
	}
	return true;
}

/* Close the X11 display. */
static void close_display(void)
{
	if (display != NULL)
		XCloseDisplay(display);
}

/* Create a window. */
static bool create_window(void)
{
	XSizeHints *sh;
	XTextProperty tp;
	int ret;

	/* Set the window title. */
	ret = XmbTextListToTextProperty(display, &conf_window_title, 1, XCompoundTextStyle, &tp);
	if (ret == XNoMemory || ret == XLocaleNotSupported) {
		log_api_error("XmbTextListToTextProperty");
		return false;
	}
	XSetWMName(display, window, &tp);
	XFree(tp.value);

	/* Show the window. */
	ret = XMapWindow(display, window);
	if (ret == BadWindow) {
		log_api_error("XMapWindow");
		return false;
	}

	/* Set the fixed window size. */
	sh = XAllocSizeHints();
	sh->flags = PMinSize | PMaxSize;
	sh->min_width = conf_window_width;
	sh->min_height = conf_window_height;
	sh->max_width = conf_window_width;
	sh->max_height = conf_window_height;
	XSetWMSizeHints(display, window, sh, XA_WM_NORMAL_HINTS);
	XFree(sh);

	/* Set the event mask. */
	ret = XSelectInput(display,
			   window,
			   KeyPressMask |
			   ExposureMask |
			   ButtonPressMask |
			   ButtonReleaseMask |
			   KeyReleaseMask |
			   PointerMotionMask);
	if (ret == BadWindow) {
		log_api_error("XSelectInput");
		return false;
	}

	/* Capture close button events if possible. */
	delete_message = XInternAtom(display, "WM_DELETE_WINDOW", True);
	if (delete_message != None && delete_message != BadAlloc && delete_message != BadValue)
		XSetWMProtocols(display, window, &delete_message, 1);

	return true;
}

/* Destroy the window. */
static void destroy_window(void)
{
	if (display != NULL)
		if (window != BadAlloc)
			XDestroyWindow(display, window);
}

/* Create an icon image. */
static bool create_icon_image(void)
{
	XWMHints *win_hints;
	XpmAttributes attr;
	Colormap cm;
	int ret;

	/* Create a color map. */
	cm = XCreateColormap(display, window,
			     DefaultVisual(display, DefaultScreen(display)),
			     AllocNone);
	if (cm == BadAlloc || cm == BadMatch || cm == BadValue ||
	    cm == BadWindow) {
		log_api_error("XCreateColorMap");
		return false;
	}

	/* Create a pixmap. */
	attr.valuemask = XpmColormap;
	attr.colormap = cm;
	ret = XpmCreatePixmapFromData(display, window, icon_xpm, &icon, &icon_mask, &attr);
	if (ret != XpmSuccess) {
		log_api_error("XpmCreatePixmapFromData");
		XFreeColormap(display, cm);
		return false;
	}

	/* Allocate for a WMHints. */
	win_hints = XAllocWMHints();
	if (!win_hints) {
		XFreeColormap(display, cm);
		return false;
	}

	/* Set the icon. */
	win_hints->flags = IconPixmapHint | IconMaskHint;
	win_hints->icon_pixmap = icon;
	win_hints->icon_mask = icon_mask;
	XSetWMHints(display, window, win_hints);

	/* Free the temporary objects. */
	XFree(win_hints);
	XFreeColormap(display,cm);
	return true;
}

/* Destroy the icon image. */
static void destroy_icon_image(void)
{
	if (display != NULL) {
		if (icon != BadAlloc)
			XFreePixmap(display, icon);
		if (icon_mask != BadAlloc)
			XFreePixmap(display, icon_mask);
	}
}

/*
 * OpenGL (GLX)
 */

/* Initialize GLX. */
static bool init_glx(void)
{
	int pix_attr[] = {
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_DOUBLEBUFFER, True,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		None
	};
	int ctx_attr[]= {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 2,
		GLX_CONTEXT_MINOR_VERSION_ARB, 0,
		GLX_CONTEXT_FLAGS_ARB, 0,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
	};
	GLXContext (*glXCreateContextAttribsARB)(Display *dpy,
						 GLXFBConfig config,
						 GLXContext share_context,
						 Bool direct,
						 const int *attrib_list);
	GLXFBConfig *config;
	XVisualInfo *vi;
	XSetWindowAttributes swa;
	XEvent event;
	int i, n;

	/* Choose a framebuffer format. */
	config = glXChooseFBConfig(display, DefaultScreen(display), pix_attr, &n);
	if (config == NULL)
		return false;
	vi = glXGetVisualFromFBConfig(display, config[0]);

	/* Create a window. */
	swa.border_pixel = 0;
	swa.event_mask = StructureNotifyMask;
	swa.colormap = XCreateColormap(display,
				       RootWindow(display, vi->screen),
				       vi->visual,
				       AllocNone);
	window = XCreateWindow(display,
			       RootWindow(display, vi->screen),
			       0,
			       0,
			       (unsigned int)conf_window_width,
			       (unsigned int)conf_window_height,
			       0,
			       vi->depth,
			       InputOutput,
			       vi->visual,
			       CWBorderPixel | CWColormap | CWEventMask,
			       &swa);
	XFree(vi);

	/* Create a GLX context. */
	glXCreateContextAttribsARB = (void *)glXGetProcAddress((const unsigned char *)"glXCreateContextAttribsARB");
	if (glXCreateContextAttribsARB == NULL) {
		XDestroyWindow(display, window);
		return false;
	}
	glx_context = glXCreateContextAttribsARB(display, config[0], 0, True, ctx_attr);
	if (glx_context == NULL) {
		XDestroyWindow(display, window);
		return false;
	}

	/* Create a GLX window. */
	glx_window = glXCreateWindow(display, config[0], window, NULL);
	XFree(config);

	/* Map the window to the screen, and wait for showing. */
	XMapWindow(display, window);
	XNextEvent(display, &event);

	/* Bind the GLX context to the window. */
	glXMakeContextCurrent(display, glx_window, glx_window, glx_context);

	/* Get the API pointers. */
	for (i = 0; i < (int)(sizeof(api)/sizeof(struct API)); i++) {
		*api[i].func = (void *)glXGetProcAddress((const unsigned char *)api[i].name);
		if(*api[i].func == NULL) {
			log_info("Failed to get %s", api[i].name);
			glXMakeContextCurrent(display, None, None, None);
			glXDestroyContext(display, glx_context);
			glXDestroyWindow(display, glx_window);
			glx_context = None;
			glx_window = None;
			return false;
		}
	}

	/* Initialize the OpenGL rendering subsystem. */
	if (!init_opengl()) {
		glXMakeContextCurrent(display, None, None, None);
		glXDestroyContext(display, glx_context);
		glXDestroyWindow(display, glx_window);
		glx_context = None;
		glx_window = None;
		return false;
	}

	return true;
}

/* Cleanup GLX. */
static void cleanup_glx(void)
{
	glXMakeContextCurrent(display, None, None, None);
	if (glx_context != None) {
		glXDestroyContext(display, glx_context);
		glx_context = None;
	}
	if (glx_window != None) {
		glXDestroyWindow(display, glx_window);
		glx_window = None;
	}

	cleanup_opengl();
}

/*
 * X11 Event Handling
 */

/* Run the event loop. */
static void run_game_loop(void)
{
	/* Get the frame start time. */
	gettimeofday(&tv_start, NULL);

	/* Main Loop. */
	while (true) {
		/* Process video playback. */
		if (is_gst_playing) {
			gstplay_loop_iteration();
			if (!gstplay_is_playing()) {
				gstplay_stop();
				is_gst_playing = false;
			}
		}

		/* Run a frame. */
		if (!run_frame())
			break;

		/* Wait for the next frame timing. */
		if (!wait_for_next_frame())
			break;	/* Close button was pressed. */

		/* Get the frame start time. */
		gettimeofday(&tv_start, NULL);
	}
}

/* Run a frame. */
static bool run_frame(void)
{
	bool cont;

	/* Start rendering. */
	if (!is_gst_playing) {
		opengl_start_rendering();
	}

	/* Call a frame event. */
	cont = on_event_frame();

	/* End rendering. */
	if (!is_gst_playing) {
		opengl_end_rendering();
		glXSwapBuffers(display, glx_window);
	}

	return cont;
}

/* Wait for the next frame timing. */
static bool wait_for_next_frame(void)
{
	struct timeval tv_end;
	uint32_t lap, wait, span;

	span = FRAME_MILLI;

	/* Do event processing and sleep until the time of next frame start. */
	do {
		/* Process events if exist. */
		while (XEventsQueued(display, QueuedAfterFlush) > 0)
			if (!next_event())
				return false;

		/* Get a lap time. */
		gettimeofday(&tv_end, NULL);
		lap = (uint32_t)((tv_end.tv_sec - tv_start.tv_sec) * 1000 +
				 (tv_end.tv_usec - tv_start.tv_usec) / 1000);

		/* Break if the time has come. */
		if (lap > span) {
			tv_start = tv_end;
			break;
		}

		/* Calc a sleep time. */
		wait = (span - lap > SLEEP_MILLI) ? SLEEP_MILLI : span - lap;

		/* Do sleep. */
		usleep(wait * 1000);

	} while(wait > 0);

	return true;
}

/* Process an event. */
static bool next_event(void)
{
	XEvent event;

	XNextEvent(display, &event);
	switch (event.type) {
	case KeyPress:
		event_key_press(&event);
		break;
	case KeyRelease:
		event_key_release(&event);
		break;
	case ButtonPress:
		event_button_press(&event);
		break;
	case ButtonRelease:
		event_button_release(&event);
		break;
	case MotionNotify:
		event_motion_notify(&event);
		break;
	case MappingNotify:
		XRefreshKeyboardMapping(&event.xmapping);
		break;
	case ClientMessage:
		/* Close button was pressed. */
		if ((Atom)event.xclient.data.l[0] == delete_message)
			return false;
		break;
	}
	return true;
}

/* Process a KeyPress event. */
static void event_key_press(XEvent *event)
{
	int key;

	/* Get a key code. */
	key = get_key_code(event);
	if (key == -1)
		return;

	/* Call an event handler. */
	on_event_key_press(key);
}

/* Process a KeyRelease event. */
static void event_key_release(XEvent *event)
{
	int key;

	/* Ignore auto repeat events. */
	if (XEventsQueued(display, QueuedAfterReading) > 0) {
		XEvent next;
		XPeekEvent(display, &next);
		if (next.type == KeyPress &&
		    next.xkey.keycode == event->xkey.keycode &&
		    next.xkey.time == event->xkey.time) {
			XNextEvent(display, &next);
			return;
		}
	}

	/* Get a key code. */
	key = get_key_code(event);
	if (key == -1)
		return;

	/* Call an event handler. */
	on_event_key_release(key);
}

/* Convert 'KeySym' to 'enum key_code'. */
static int get_key_code(XEvent *event)
{
	char text[255];
	KeySym keysym;

	/* Get a KeySym. */
	XLookupString(&event->xkey, text, sizeof(text), &keysym, 0);

	/* Convert to key_code. */
	switch (keysym) {
	case XK_Return:
	case XK_KP_Enter:
		return KEY_RETURN;
	case XK_space:
		return KEY_SPACE;
		break;
	case XK_Control_L:
	case XK_Control_R:
		return KEY_CONTROL;
	case XK_Down:
		return KEY_DOWN;
	case XK_Up:
		return KEY_UP;
	case XK_Left:
		return KEY_LEFT;
	case XK_Right:
		return KEY_RIGHT;
	default:
		break;
	}
	return -1;
}

/* Process a ButtonPress event. */
static void event_button_press(XEvent *event)
{
	/* See the button type and dispatch. */
	switch (event->xbutton.button) {
	case Button1:
		on_event_mouse_press(MOUSE_LEFT,
				     event->xbutton.x,
				     event->xbutton.y);
		break;
	case Button3:
		on_event_mouse_press(MOUSE_RIGHT,
				     event->xbutton.x,
				     event->xbutton.y);
		break;
	case Button4:
		on_event_key_press(KEY_UP);
		on_event_key_release(KEY_UP);
		break;
	case Button5:
		on_event_key_press(KEY_DOWN);
		on_event_key_release(KEY_DOWN);
		break;
	default:
		break;
	}
}

/* Process a ButtonPress event. */
static void event_button_release(XEvent *event)
{
	/* See the button type and dispatch. */
	switch (event->xbutton.button) {
	case Button1:
		on_event_mouse_release(MOUSE_LEFT,
				       event->xbutton.x,
				       event->xbutton.y);
		break;
	case Button3:
		on_event_mouse_release(MOUSE_RIGHT,
				       event->xbutton.x,
				       event->xbutton.y);
		break;
	}
}

/* Process a MotionNotify event. */
static void event_motion_notify(XEvent *event)
{
	/* Call an event handler. */
	on_event_mouse_move(event->xmotion.x, event->xmotion.y);
}

/*
 * HAL
 */

/*
 * Put an INFO log.
 */
bool log_info(const char *s, ...)
{
	char buf[LOG_BUF_SIZE];
	va_list ap;

	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	if (log_fp != NULL) {
		fprintf(log_fp, "%s\n", buf);
		fflush(log_fp);
		if (ferror(log_fp))
			return false;
	}
	return true;
}

/*
 * Put a WARN log.
 */
bool log_warn(const char *s, ...)
{
	char buf[LOG_BUF_SIZE];
	va_list ap;

	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	if (log_fp != NULL) {
		fprintf(log_fp, "%s\n", buf);
		fflush(log_fp);
		if (ferror(log_fp))
			return false;
	}
	return true;
}

/*
 * Put an ERROR log.
 */
bool log_error(const char *s, ...)
{
	char buf[LOG_BUF_SIZE];
	va_list ap;

	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	if (log_fp != NULL) {
		fprintf(log_fp, "%s\n", buf);
		fflush(log_fp);
		if (ferror(log_fp))
			return false;
	}
	return true;
}

/*
 * Notify an image update.
 */
void notify_image_update(struct image *img)
{
	opengl_notify_image_update(img);
}

/*
 * Notify an image free.
 */
void notify_image_free(struct image *img)
{
	opengl_notify_image_free(img);
}

/*
 * Render an image to the screen with the normal pipeline.
 */
void render_image_normal(int dst_left,
			 int dst_top,
			 int dst_width,
			 int dst_height,
			 struct image *src_image,
			 int src_left,
			 int src_top,
			 int src_width,
			 int src_height,
			 int alpha)
{
	opengl_render_image_normal(dst_left,
				   dst_top,
				   dst_width,
				   dst_height,
				   src_image,
				   src_left,
				   src_top,
				   src_width,
				   src_height,
				   alpha);
}

/*
 * Render an image to the screen with the add pipeline.
 */
void render_image_add(int dst_left,
		      int dst_top,
		      int dst_width,
		      int dst_height,
		      struct image *src_image,
		      int src_left,
		      int src_top,
		      int src_width,
		      int src_height,
		      int alpha)
{
	opengl_render_image_add(dst_left,
				dst_top,
				dst_width,
				dst_height,
				src_image,
				src_left,
				src_top,
				src_width,
				src_height,
				alpha);
}

/*
 * Render an image to the screen with the dim pipeline.
 */
void render_image_dim(int dst_left,
		      int dst_top,
		      int dst_width,
		      int dst_height,
		      struct image *src_image,
		      int src_left,
		      int src_top,
		      int src_width,
		      int src_height,
		      int alpha)
{
	opengl_render_image_dim(dst_left,
				dst_top,
				dst_width,
				dst_height,
				src_image,
				src_left,
				src_top,
				src_width,
				src_height,
				alpha);
}

/*
 * Render an image to the screen with the rule pipeline.
 */
void render_image_rule(struct image *src_img,
		       struct image *rule_img,
		       int threshold)
{
	opengl_render_image_rule(src_img, rule_img, threshold);
}

/*
 * Render an image to the screen with the melt pipeline.
 */
void render_image_melt(struct image *src_img,
		       struct image *rule_img,
		       int progress)
{
	opengl_render_image_melt(src_img, rule_img, progress);
}

/*
 * Render an image to the screen with the normal pipeline.
 */
void
render_image_3d_normal(
	float x1,
	float y1,
	float x2,
	float y2,
	float x3,
	float y3,
	float x4,
	float y4,
	struct image *src_image,
	int src_left,
	int src_top,
	int src_width,
	int src_height,
	int alpha)
{
	opengl_render_image_3d_normal(x1,
				      y1,
				      x2,
				      y2,
				      x3,
				      y3,
				      x4,
				      y4,
				      src_image,
				      src_left,
				      src_top,
				      src_width,
				      src_height,
				      alpha);
}

/*
 * Render an image to the screen with the add pipeline.
 */
void
render_image_3d_add(
	float x1,
	float y1,
	float x2,
	float y2,
	float x3,
	float y3,
	float x4,
	float y4,
	struct image *src_image,
	int src_left,
	int src_top,
	int src_width,
	int src_height,
	int alpha)
{
	opengl_render_image_3d_add(x1,
				   y1,
				   x2,
				   y2,
				   x3,
				   y3,
				   x4,
				   y4,
				   src_image,
				   src_left,
				   src_top,
				   src_width,
				   src_height,
				   alpha);
}

/*
 * Make a save directory.
 */
bool make_sav_dir(void)
{
	struct stat st = {0};

	if (stat(SAVE_DIR, &st) == -1)
		mkdir(SAVE_DIR, 0700);

	return true;
}

/*
 * Make an effective path from a directory name and a file name.
 */
char *make_valid_path(const char *dir, const char *fname)
{
	char *buf;
	size_t len;

	if (dir == NULL)
		dir = "";

	/* パスのメモリを確保する */
	len = strlen(dir) + 1 + strlen(fname) + 1;
	buf = malloc(len);
	if (buf == NULL) {
		log_memory();
		return NULL;
	}

	strcpy(buf, dir);
	if (strlen(dir) != 0)
		strcat(buf, "/");
	strcat(buf, fname);

	return buf;
}

/*
 * Reset a timer.
 */
void reset_lap_timer(uint64_t *t)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	*t = (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/*
 * Get a timer lap.
 */
uint64_t get_lap_timer_millisec(uint64_t *t)
{
	struct timeval tv;
	uint64_t end;
	
	gettimeofday(&tv, NULL);

	end = (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);

	return (uint64_t)(end - *t);
}

/*
 * Show a exit dialog.
 */
bool exit_dialog(void)
{
	/* stub */
	return true;
}

/*
 * Show a back-to-title dialog.
 */
bool title_dialog(void)
{
	/* stub */
	return true;
}

/*
 * Show a delete confirmation dialog.
 */
bool delete_dialog(void)
{
	/* stub */
	return true;
}

/*
 * Show an overwrite confirmation dialog.
 */
bool overwrite_dialog(void)
{
	/* stub */
	return true;
}

/*
 * Show a reset-to-default dialog.
 */
bool default_dialog(void)
{
	/* stub */
	return true;
}

/*
 * Play a video.
 */
bool play_video(const char *fname, bool is_skippable)
{
	char *path;

	path = make_valid_path(MOV_DIR, fname);

	is_gst_playing = true;
	is_gst_skippable = is_skippable;

	gstplay_play(path, window);

	free(path);

	return true;
}

/*
 * Stop the video.
 */
void stop_video(void)
{
	gstplay_stop();

	is_gst_playing = false;
}

/*
 * Check whether a video is playing.
 */
bool is_video_playing(void)
{
	return is_gst_playing;
}

/*
 * Update the window title.
 */
void update_window_title(void)
{
	XTextProperty tp;
	char *buf;
	const char *sep;
	int ret;

	/* Get the separator. */
	sep = conf_window_title_separator;
	if (sep == NULL)
		sep = " ";

	/* Make a title. */
	strncpy(title_buf, conf_window_title, TITLE_BUF_SIZE - 1);
	strncat(title_buf, sep, TITLE_BUF_SIZE - 1);
	strncat(title_buf, get_chapter_name(), TITLE_BUF_SIZE - 1);
	title_buf[TITLE_BUF_SIZE - 1] = '\0';

	/* Set the title for the window. */
	buf = title_buf;
	ret = XmbTextListToTextProperty(display, &buf, 1, XCompoundTextStyle, &tp);
	if (ret == XNoMemory || ret == XLocaleNotSupported) {
		log_api_error("XmbTextListToTextProperty");
		return;
	}
	XSetWMName(display, window, &tp);
	XFree(tp.value);
}

/*
 * Check whether full screen mode is supported.
 */
bool is_full_screen_supported(void)
{
	return false;
}

/*
 * Check whether we are in full screen mode.
 */
bool is_full_screen_mode(void)
{
	return false;
}

/*
 * Enter full screen mode.
 */
void enter_full_screen_mode(void)
{
	/* stub */
}

/*
 * Leave full screen mode.
 */
void leave_full_screen_mode(void)
{
	/* stub */
}

/*
 * Get the system locale.
 */
const char *get_system_locale(void)
{
	const char *locale;

	locale = setlocale(LC_ALL, NULL);
	if (locale == NULL)
		return "en";
	else if (locale[0] == '\0' || locale[1] == '\0')
		return "en";
	else if (strncmp(locale, "en", 2) == 0)
		return "en";
	else if (strncmp(locale, "fr", 2) == 0)
		return "fr";
	else if (strncmp(locale, "de", 2) == 0)
		return "de";
	else if (strncmp(locale, "it", 2) == 0)
		return "it";
	else if (strncmp(locale, "es", 2) == 0)
		return "es";
	else if (strncmp(locale, "el", 2) == 0)
		return "el";
	else if (strncmp(locale, "ru", 2) == 0)
		return "ru";
	else if (strncmp(locale, "zh_CN", 5) == 0)
		return "zh";
	else if (strncmp(locale, "zh_TW", 5) == 0)
		return "tw";
	else if (strncmp(locale, "ja", 2) == 0)
		return "ja";

	return "other";
}

/*
 * Text-to-speach.
 */
void speak_text(const char *msg)
{
	UNUSED_PARAMETER(msg);
}
