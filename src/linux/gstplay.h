/* -*- c-basic-offset: 2; indent-tabs-mode: nil; -*- */

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * GStreamer video playback
 */

#ifndef XENGINE_GSTPLAY_H
#define XENGINE_GSTPLAY_H

#include <X11/Xlib.h>

void
gstplay_init (int argc, char *argv[]);

void
gstplay_play (const char *fname, Window window);

void
gstplay_stop (void);

int
gstplay_is_playing (void);

void
gstplay_loop_iteration (void);

#endif
