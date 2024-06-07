/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * MIDI Playback
 */

#include "../xengine.h"
#include "midi.h"

#include <windows.h>

static HWND hInvisibleWnd;

static wchar_t szClassName[] = L"VNMidiPlayer";
static wchar_t szLastPlayCommand[1024];
static LONG WINAPI MidiWndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);
static VOID OnMidiPlayerMciNotify(WPARAM wParam, LPARAM lParam);

extern const wchar_t *conv_utf8_to_utf16(const char *utf8_message);

void init_midi(void)
{
	HINSTANCE hInstance;
	WNDCLASS wc;

	hInstance = GetModuleHandle(NULL);

	// Make an invisible window for MCI messages.
	memset(&wc, 0, sizeof(wc));
	wc.lpfnWndProc = MidiWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = szClassName;
	RegisterClass (&wc);
	hInvisibleWnd = CreateWindow(szClassName, szClassName, WS_POPUP, 0, 0, 100, 100, NULL, NULL, hInstance, NULL);
	if (hInvisibleWnd == NULL)
		return;

	sync_midi();
}

void cleanup_midi(void)
{
	stop_midi();
	DestroyWindow(hInvisibleWnd);
	hInvisibleWnd = NULL;
	sync_midi();
}

void sync_midi(void)
{
	MSG msg;
	while (PeekMessage(&msg, hInvisibleWnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LONG WINAPI MidiWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case MM_MCINOTIFY:
	    OnMidiPlayerMciNotify(wParam, lParam);
	    break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void OnMidiPlayerMciNotify(WPARAM wParam, LPARAM lParam)
{
	wchar_t ret[255];

	UNUSED_PARAMETER(lParam);

	if(wParam != MCI_NOTIFY_SUCCESSFUL)
		return;
	if (wcscmp(szLastPlayCommand, L"") == 0)
		return;

	mciSendString(L"close vnmidi", ret, 255, hInvisibleWnd);
	mciSendString(szLastPlayCommand, ret, 255, hInvisibleWnd);
	mciSendString(L"play vnmidi notify", ret, 255, hInvisibleWnd);
}

bool play_midi(const char *dir, const char *fname)
{
	wchar_t ret[256];
	stop_midi();

	if (hInvisibleWnd == NULL)
	{
		init_midi();
	}

	wcscpy(szLastPlayCommand, L"open ");
	wcscat(szLastPlayCommand, conv_utf8_to_utf16(make_valid_path(dir, fname)));
	wcscat(szLastPlayCommand, L" type sequencer alias vnmidi");

	mciSendString(szLastPlayCommand, ret, 255, hInvisibleWnd);
	mciSendString(L"play vnmidi notify", ret, 255, hInvisibleWnd);
	sync_midi();

	return true;
}

void stop_midi(void)
{
	wchar_t ret[256];

	mciSendString(L"stop vnmidi", ret, 255, hInvisibleWnd);
	mciSendString(L"close vnmidi", ret, 255, hInvisibleWnd);
	sync_midi();
}
