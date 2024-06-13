/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

#ifndef XENGINE_NDKMAIN_H
#define XENGINE_NDKMAIN_H

#include "types.h"
#include <jni.h>

struct rfile {
	jbyteArray array;
	char *buf;
	uint64_t size;
	uint64_t pos;
};

struct wfile {
	jobject os;
};

extern JNIEnv *jni_env;
extern jobject main_activity;

void post_delayed_remove_rfile_ref(struct rfile *rf);

#endif
