/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * MIDI playback
 */

#ifndef XENGINE_D3D_H
#define XENGINE_D3D_H

#include "../polarisengine.h"

#include <windows.h>

void init_midi(void);
void cleanup_midi(void);
bool play_midi(const char *dir, const char *fname);
void stop_midi(void);
void sync_midi(void);

#endif
