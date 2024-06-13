/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * The HAL for Direct3D 9
 */

#ifndef XENGINE_D3D_H
#define XENGINE_D3D_H

#include "../polarisengine.h"

#include <windows.h>

BOOL D3DInitialize(HWND hWnd);
VOID D3DCleanup(void);
BOOL D3DResizeWindow(int nOffsetX, int nOffsetY, float scale);
BOOL D3DLockTexture(int width, int height, pixel_t *pixels,
					pixel_t **locked_pixels, void **texture);
BOOL D3DUnlockTexture(int width, int height, pixel_t *pixels,
					  pixel_t **locked_pixels, void **texture);
VOID D3DDestroyTexture(void *texture);
VOID D3DStartFrame(void);
VOID D3DEndFrame(void);
BOOL D3DRedraw(void);
VOID D3DRenderImage(int dst_left, int dst_top,
					struct image * RESTRICT src_image, int width, int height,
					int src_left, int src_top, int alpha, int bt);
VOID D3DRenderImageDim(int dst_left, int dst_top,
					   struct image * RESTRICT src_image, int width,
					   int height, int src_left, int src_top);
VOID D3DRenderImageRule(struct image * RESTRICT src_image,
						struct image * RESTRICT rule_image,
						int threshold);
VOID D3DRenderImageMelt(struct image * RESTRICT src_image,
						struct image * RESTRICT rule_image,
						int threshold);
VOID D3DRenderClear(int left, int top, int width, int height, pixel_t color);
VOID *D3DGetDevice(void);
VOID D3DSetDeviceLostCallback(void (*pFunc)(void));

BOOL D3DIsSoftRendering(void);

#endif
