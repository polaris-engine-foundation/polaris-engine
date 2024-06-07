/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * DirectShow video playback
 */

#ifndef XENGINE_DSVIDEO_H
#define XENGINE_DSVIDEO_H

#include "../xengine.h"

#include <windows.h>

#define WM_GRAPHNOTIFY	(WM_APP + 13)

BOOL DShowPlayVideo(HWND hWnd, const char *pszFileName, int nOfsX, int nOfsY, int nWidth, int nHeight);
VOID DShowStopVideo(void);
BOOL DShowProcessEvent(void);

#endif
