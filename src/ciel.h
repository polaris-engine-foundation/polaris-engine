/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * The Ciel Direction System (Designed by Asatsuki and ktabata)
 */

#ifndef XENGINE_CIEL_H
#define XENGINE_CIEL_H

#include "file.h"

void ciel_clear_hook(void);
bool ciel_serialize_hook(struct wfile *wf);
bool ciel_deserialize_hook(struct rfile *rf);

#endif
