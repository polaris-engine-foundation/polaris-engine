// -*- coding: utf-8; indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

@import AppKit;
@import AVFoundation;

#import "GameViewControllerProtocol.h"

@interface ViewController : NSViewController <NSApplicationDelegate, NSWindowDelegate, NSTextViewDelegate, GameViewControllerProtocol>

@property BOOL isControlPressed;
@property BOOL isShiftPressed;
@property BOOL isCommandPressed;

- (void)onScriptEnter;
- (void)onScriptChange;

@end
