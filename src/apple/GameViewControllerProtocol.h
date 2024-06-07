// -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*-

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

#import <Foundation/Foundation.h>

#include "types.h"

@protocol GameViewControllerProtocol <NSObject>

// Screen scaling
- (float)screenScale;
- (CGPoint)screenOffset;
- (CGSize)screenSize;
- (CGPoint)windowPointToScreenPoint:(CGPoint)windowPoint;

// Video playback implementation
- (BOOL)isVideoPlaying;
- (void)playVideoWithPath:(NSString *)path skippable:(BOOL)isSkippable;
- (void)stopVideo;

// Set a window title
- (void)setWindowTitle:(NSString *)name;

// Full screen implementation
- (BOOL)isFullScreen;
- (void)enterFullScreen;
- (void)leaveFullScreen;

@end
