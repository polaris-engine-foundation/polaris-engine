/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * DirectSound Output (DirectSound 5.0)
 */

#ifndef POLARIS_ENGINE_DSOUND_H
#define POLARIS_ENGINE_DSOUND_H

#include <windows.h>

BOOL DSInitialize(HWND hWnd);
VOID DSCleanup();

#endif
