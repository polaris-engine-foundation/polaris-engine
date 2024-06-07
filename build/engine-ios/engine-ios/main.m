/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*- */

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

@import UIKit;

#import "AppDelegate.h"

int main(int argc, char * argv[]) {
    NSString * appDelegateClassName;
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        appDelegateClassName = NSStringFromClass([AppDelegate class]);
    }
    setlocale(LC_NUMERIC, "C");
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}
