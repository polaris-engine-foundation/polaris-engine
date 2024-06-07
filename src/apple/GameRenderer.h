// -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*-

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

//
// GameRenderer: The HAL for Apple macOS/iOS/iPadOS with Metal
//

@import MetalKit;

#import "GameViewControllerProtocol.h"

@interface GameRenderer : NSObject <MTKViewDelegate>
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView andController:(nonnull id<GameViewControllerProtocol>)controller;
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size;

@end
