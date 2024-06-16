/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * Text to speech.
 */

#ifndef POLARIS_ENGINE_TTS_SAPI_H
#define POLARIS_ENGINE_TTS_SAPI_H

#ifdef __cplusplus
extern "C" {
#endif

void InitSAPI(void);
void SpeakSAPI(const wchar_t *text);

#ifdef __cplusplus
};
#endif

#endif
