// -*- coding: utf-8; indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

#import "WindowController.h"

@interface WindowController ()
{
}
@end

@implementation WindowController

- (void)windowDidLoad {
    [super windowDidLoad];
    
    self.window.delegate = (id<NSWindowDelegate>)self.contentViewController;
}

@end
