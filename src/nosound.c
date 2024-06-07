/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * Dummy ALSA Sound Module (for when sound is not available)
 */

#include "xengine.h"

bool init_asound(void)
{
	return false;
}

void cleanup_asound(void)
{
}

/*
 * Main HAL
 */

bool play_sound(int stream, struct wave *w)
{
	return true;
}

bool stop_sound(int stream)
{
	return true;
}

bool set_sound_volume(int stream, float vol)
{
	return true;
}

bool is_sound_finished(int stream)
{
	return true;
}
