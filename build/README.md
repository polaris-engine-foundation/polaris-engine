HOW TO BUILD
============
 
This document provides instructions to build various `Polaris Engine` apps from the source code.

## Getting Started

Firstly, you have to get the `Polaris Engine` repository using `Git`.

* From the terminal, run the following command:
```
git clone https://github.com/polaris-engine-foundation/polaris-engine.git
cd polaris-engine
make setup
```

# Game Runtime (the main engine)

## Windows Game

This method will build a Windows app on Ubuntu or macOS.

* Prerequisite
  * Use Ubuntu or macOS

* Build (x86, 32-bit, recommended)
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make engine-windows
  ```

* Alternative build (x86_64, 64-bit)
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make engine-windows-64
  ```

* Alternative build (Arm64)
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make engine-windows-arm64
  ```

## Mac Game

This method will utilize `Xcode` and terminal to build macOS main engine binary.

* Use macOS 14
* Install Xcode 15
* Run `Polaris Engine` and export a macOS source code tree
* From Xcode, open the exported project
* Build the app

## Web Game

This method will build the Wasm version of `Polaris Engine`.

* Build instructions
  * From the terminal, navigate to the source code directory
  * Run the following command to create the `build/engine-wasm/html/index.*` files:
  ```
  make engine-wasm
  ```
  * Note that you need Emscripten compiler

* Test instructions
  * Copy your `data01.arc` to `build/engine-wasm/html/`
  * Do `make run` in the `build/engine-wasm/` directory
  * Open `http://localhost:8000/html/` by a browser

## Linux Game

This method will build a Linux app.

* Prerequisite
  * Use Ubuntu or Debian

* Build
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make engine-linux
  ```

## iOS Game

This method will utilize `Xcode` and terminal to build an iOS app.

* Use macOS 14
* Install Xcode 15
* Run `Polaris Engine` and export an iOS source code tree
* From Xcode, open the exported project
* Set the build target to your device (or a simulator)
* Build the app

## Android Game

This method requires `Android Studio` to build the Android app.

* Install `Android Studio`
* Run `Polaris Engine` and export an Android source code tree
* Open the exported project from `Android Studio`
* Build the project

# Development Tool

## Windows Dev Tool

This method will build an app of `Polaris Engine` for Windows on Ubuntu or macOS.

* Prerequisite
  * Use Ubuntu or macOS

* Build
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make pro-windows
  ```

## Mac Dev Tool

This method will utilize `Xcode` to build a macOS app.

* Use macOS 14
* Install Xcode 15
* From Xcode, open `build/pro-macos/pro-macos.xcodeproj`
* Build

## iOS Dev Tool

This method will utilize `Xcode` to build an iOS Pro app.

* Steps
  * Use macOS 14
  * Install Xcode 15
  * From Xcode, open `build/pro-ios/pro-ios.xcodeproj`
  * Build

## Linux Dev Tool

This method will build a Linux version of Polaris Engine using Qt6.

* Prerequisite
  * Use Ubuntu or Debian

* Build
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make pro-linux
  ```

## Web Dev Tool

This method will build a Web version of `Polaris Engine`.

* Use Ubuntu or macOS
* From the terminal, navigate to the source code directory and run the following command:
```
make pro-wasm
```
* Upload the `build/pro-wasm/html/*` files to your Web server
* Access the uploaded `index.html` via `https` (Note: `http` is not allowed due to file access APIs)

# Misc

## GCC Static Analysis

To use static analysis of `gcc`, type the following commands:
```
cd build/engine-x11
make analyze
```

## LLVM Static Analysis

To use static analysis of LLVM/Clang, you can run the following commands:
```
cd build/engine-x11
make analyze
```

## Memory Leak Profiling

We use `valgrind` to detect memory leaks and keep memory-related bugs at zero.

To use memory leak checks on Linux, type the following commands:
```
cd build/engine-x11
make valgrind
```

## NetBSD sound settings

* To setup `ALSA/OSS`, create `/etc/asound.conf` and copy the following snippet to the file:
  ```
  pcm.!default {
    type oss
    device /dev/audio
  }
  ctl.!default {
    type oss
    device /dev/mixer
  }
  ```

# Release

A release is fully automated on GitHub actions CI/CD.
However, you can make release binaries on macOS manually.
To do so, type:
```
make release
```
