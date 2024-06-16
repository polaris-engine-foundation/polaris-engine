/* -*- coding: utf-8; indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * Audio Unit Sound Output
 */

#ifndef POLARIS_ENGINE_AUNIT_H
#define POLARIS_ENGINE_AUNIT_H

bool init_aunit(void);
void cleanup_aunit(void);
void pause_sound(void);
void resume_sound(void);

#endif
