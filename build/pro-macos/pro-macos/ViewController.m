// -*- coding: utf-8; indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

#import "ViewController.h"
#import "ViewController.h"
#import "GameView.h"
#import "GameRenderer.h"

#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

// Polaris Engine Base
#import "polarisengine.h"
#import "pro.h"
#import "package.h"

// Standard C
#import <wchar.h>

// HAL
#import "aunit.h"

static ViewController *theViewController;

@interface ViewController ()
// Status
@property BOOL isInitialized;
@property BOOL isEnglish;
@property BOOL isRunning;
@property BOOL isContinuePressed;
@property BOOL isNextPressed;
@property BOOL isStopPressed;
@property BOOL isOpenScriptPressed;
@property BOOL isExecLineChanged;
@property int changedExecLine;
@property BOOL isVarsUpdated;

// IBOutlet
@property (weak) IBOutlet NSView *projectPanel;
@property (weak) IBOutlet NSButton *buttonJapaneseLight;
@property (weak) IBOutlet NSButton *buttonJapaneseDark;
@property (weak) IBOutlet NSButton *buttonJapaneseNovel;
@property (weak) IBOutlet NSButton *buttonJapaneseTategaki;
@property (weak) IBOutlet NSButton *buttonEnglish;
@property (weak) IBOutlet NSButton *buttonEnglishNovel;
@property (weak) IBOutlet GameView *renderView;
@property (weak) IBOutlet NSView *editorPanel;
@property (weak) IBOutlet NSButton *buttonContinue;
@property (weak) IBOutlet NSButton *buttonNext;
@property (weak) IBOutlet NSButton *buttonStop;
@property (weak) IBOutlet NSButton *buttonMove;
@property (weak) IBOutlet NSTextField *textFieldScriptName;
@property (weak) IBOutlet NSButton *buttonOpenScript;
@property (unsafe_unretained) IBOutlet NSTextView *textViewScript;
@property (unsafe_unretained) IBOutlet NSTextField *textFieldVariables;
@property (weak) IBOutlet NSButton *buttonUpdateVariables;
@end

@implementation ViewController
{
    // The GameRender (common for AppKit and UIKit)
    GameRenderer *_renderer;
    
    // The screen information
    float _screenScale;
    NSSize _screenSize;
    NSPoint _screenOffset;
    
    // The view frame before entering a full screen mode.
    NSRect _savedViewFrame;
    
    // The temporary window frame size on resizing.
    NSSize _resizeFrame;
    
    // The full screen status.
    BOOL _isFullScreen;
    
    // The video player objects and status.
    AVPlayer *_avPlayer;
    AVPlayerLayer *_avPlayerLayer;
    BOOL _isVideoPlaying;
    
    // Edit
    BOOL _isFirstChange;
    
    NSTimer *_formatTimer;
    BOOL _needFormat;
}

//
// View Initialization
//

- (void)viewDidLoad {
    [super viewDidLoad];
}

- (void)viewDidAppear {
    [super viewDidAppear];
    
    // Determine the language we use.
    _isEnglish = ![[[NSLocale preferredLanguages] objectAtIndex:0] hasPrefix:@"ja-"];
    
    // Save the view controller to the global variable.
    theViewController = self;

    // Set button images.
    [self.buttonJapaneseLight setImage:[NSImage imageNamed:@"japanese-light"]];
    [self.buttonJapaneseDark setImage:[NSImage imageNamed:@"japanese-dark"]];
    [self.buttonJapaneseNovel setImage:[NSImage imageNamed:@"japanese-novel"]];
    [self.buttonJapaneseTategaki setImage:[NSImage imageNamed:@"japanese-tategaki"]];
    [self.buttonEnglish setImage:[NSImage imageNamed:@"english"]];
    [self.buttonEnglishNovel setImage:[NSImage imageNamed:@"english-novel"]];

    // Set the editor panel non-editable.
    [theViewController.buttonContinue setEnabled:NO];
    [theViewController.buttonNext setEnabled:NO];
    [theViewController.buttonStop setEnabled:NO];
    [theViewController.buttonMove setEnabled:NO];
    [theViewController.buttonOpenScript setEnabled:NO];
    [theViewController.textViewScript setEditable:NO];
    [theViewController.textFieldVariables setEditable:NO];
    [theViewController.buttonUpdateVariables setEnabled:NO];
}

// Called when a project is created or opened.
- (void)setupView {
    assert(!_isInitialized);

    // Make the project panel hidden.
    [_projectPanel setHidden:YES];

    // Set the render panel size.
    [_renderView setFrame:NSMakeRect(0, 0, _renderView.bounds.size.width, _editorPanel.frame.size.height)];

    // Initialize the engine.
    init_locale_code();
    if(!init_file())
        exit(1);
    if(!init_conf())
        exit(1);
    if(!init_aunit())
        exit(1);
    if(!on_event_init())
        exit(1);

    // Create an MTKView.
    self.renderView.enableSetNeedsDisplay = YES;
    self.renderView.device = MTLCreateSystemDefaultDevice();
    self.renderView.clearColor = MTLClearColorMake(0.0, 0, 0, 1.0);
    _renderer = [[GameRenderer alloc] initWithMetalKitView:self.renderView andController:self];
    if(!_renderer) {
        NSLog(@"Renderer initialization failed");
        return;
    }
    [_renderer mtkView:self.renderView drawableSizeWillChange:self.renderView.drawableSize];
    self.renderView.delegate = _renderer;
    [self updateViewport:self.renderView.frame.size];
    
    // Setup a rendering timer.
    [NSTimer scheduledTimerWithTimeInterval:1.0/60.0
                                     target:self
                                   selector:@selector(renderingTimerFired:)
                                   userInfo:nil
                                    repeats:YES];
    
    // Accept keyboard and mouse inputs.
    [self.view.window makeFirstResponder:self];
    [self.view.window setAcceptsMouseMovedEvents:YES];
    [self.view.window makeKeyAndOrderFront:self];

    // Set the window title.
    update_window_title();

    // Set the script view delegate.
    _textViewScript.delegate = (id<NSTextViewDelegate>)_textViewScript;

    // Update the viewport size.
    [self updateViewport:self.renderView.frame.size];

    // Set the initialized state.
    self.isInitialized = YES;

    // Set the editor panel editable.
    on_change_running_state(false, false);
}

- (void)renderingTimerFired:(NSTimer *)timer {
    [self.renderView setNeedsDisplay:TRUE];
}

//
// NSWindowDelegate
//

- (void)windowWillClose:(NSNotification *)notification {
    // Save.
    if (_isInitialized) {
        save_global_data();
        save_seen();
    }

    // Exit the event loop.
    [NSApp stop:nil];

    // Magic: Post an empty event and make sure to exit the main loop.
    [NSApp postEvent:[NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                        location:NSMakePoint(0, 0)
                                   modifierFlags:0
                                       timestamp:0
                                    windowNumber:0
                                         context:nil
                                         subtype:0
                                           data1:0
                                           data2:0]
             atStart:YES];
}

// フルスクリーンになるとき呼び出される
- (void)windowWillEnterFullScreen:(NSNotification *)notification {
    _isFullScreen = YES;
}

// フルスクリーンから戻るときに呼び出される
- (void)windowWillExitFullScreen:(NSNotification *)notification {
    _isFullScreen = NO;
}

- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize {
    if (frameSize.width < 1280)
        frameSize.width = 1280;
    if (frameSize.height < 720)
        frameSize.height = 720;
    return frameSize;
}

- (void)windowDidResize:(NSNotification *)notification {
    [self updateViewport:self.renderView.frame.size];

    // 動画プレーヤレイヤのサイズを更新する
    if (_avPlayerLayer != nil)
        [_avPlayerLayer setFrame:NSMakeRect(_screenOffset.x, _screenOffset.y, _screenSize.width, _screenSize.height)];
}

- (void)updateViewport:(NSSize)newViewSize {
    if (newViewSize.width == 0 || newViewSize.height == 0) {
        _screenScale = 1.0f;
        _screenSize = NSMakeSize(conf_window_width, conf_window_height);
        _screenOffset.x = 0;
        _screenOffset.y = 0;
        return;
    }
    
    // ゲーム画面のアスペクト比を求める
    float aspect = (float)conf_window_height / (float)conf_window_width;
    
    // 1. 横幅優先で高さを仮決めする
    _screenSize.width = newViewSize.width;
    _screenSize.height = _screenSize.width * aspect;
    _screenScale = (float)conf_window_width / _screenSize.width;
    
    // 2. 高さが足りなければ、縦幅優先で横幅を決める
    if(_screenSize.height > newViewSize.height) {
        _screenSize.height = newViewSize.height;
        _screenSize.width = _screenSize.height / aspect;
        _screenScale = (float)conf_window_height / _screenSize.height;
    }
    
    // スケールファクタを乗算する
    _screenSize.width *= self.renderView.layer.contentsScale;
    _screenSize.height *= self.renderView.layer.contentsScale;
    newViewSize.width *= self.renderView.layer.contentsScale;
    newViewSize.height *= self.renderView.layer.contentsScale;

    // マージンを計算する
    _screenOffset.x = (newViewSize.width - _screenSize.width) / 2.0f;
    _screenOffset.y = (newViewSize.height - _screenSize.height) / 2.0f;
}

- (void)mouseMoved:(NSEvent *)event {
    NSPoint point = [self windowPointToScreenPoint:[event locationInWindow]];
    on_event_mouse_move(point.x, point.y);
}

- (void)mouseDragged:(NSEvent *)event {
    NSPoint point = [self windowPointToScreenPoint:[event locationInWindow]];
    on_event_mouse_move(point.x, point.y);
}

// キーボード修飾変化イベント
- (void)flagsChanged:(NSEvent *)event {
    BOOL newControllPressed = ([event modifierFlags] & NSEventModifierFlagControl) ==
        NSEventModifierFlagControl;
    self.isShiftPressed = ([event modifierFlags] & NSEventModifierFlagShift) ==
        NSEventModifierFlagShift;
    self.isCommandPressed = ([event modifierFlags] & NSEventModifierFlagCommand) ==
        NSEventModifierFlagCommand;

    // Controlキーの状態が変化した場合は通知する
    if (!self.isControlPressed && newControllPressed) {
        self.isControlPressed = YES;
        on_event_key_press(KEY_CONTROL);
    } else if (self.isControlPressed && !newControllPressed) {
        self.isControlPressed = NO;
        on_event_key_release(KEY_CONTROL);
    }
}

// キー押下イベント
- (void)keyDown:(NSEvent *)theEvent {
    if ([theEvent isARepeat])
        return;
    
    int kc = [self convertKeyCode:[theEvent keyCode]];
    if (kc != -1)
        on_event_key_press(kc);
}

// キー解放イベント
- (void)keyUp:(NSEvent *)theEvent {
    int kc = [self convertKeyCode:[theEvent keyCode]];
    if (kc != -1)
        on_event_key_release(kc);
}

// キーコードを変換する
- (int)convertKeyCode:(int)keyCode {
    switch(keyCode) {
        case 49: return KEY_SPACE;
        case 36: return KEY_RETURN;
        case 123: return KEY_LEFT;
        case 124: return KEY_RIGHT;
        case 125: return KEY_DOWN;
        case 126: return KEY_UP;
        case 53: return KEY_ESCAPE;
    }
    return -1;
}

//
// Project
//

- (BOOL)createProject:(NSString *)templateName {
    // Open folder.
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setCanChooseDirectories:YES];
    [panel setCanCreateDirectories:YES];
    if ([panel runModal] != NSModalResponseOK)
        return NO;

    // Create a project file.
    NSString *path = [[panel directoryURL] path];
    [[NSFileManager defaultManager] changeCurrentDirectoryPath:path];
    FILE *fp = fopen([[[[panel URL] path] stringByAppendingString:@"/game.polarisengine"] UTF8String], "w");
    if (fp == NULL) {
        NSAlert *alert;
        alert = [[NSAlert alloc] init];
        [alert setMessageText:_isEnglish ?
            @"Failed to write to the folder. Choose another one." :
            @"フォルダへの書き込みに失敗しました。別のフォルダを指定してください。"];
        [alert addButtonWithTitle:_isEnglish ? @"Yes" : @"はい"];
        [alert setAlertStyle:NSAlertStyleInformational];
        [alert runModal];
        return NO;
    }
    fclose(fp);

    if (![self copyResourceTemplate:templateName])
        return NO;

    return YES;
}

- (BOOL)copyResourceTemplate:(NSString *)from {
    NSArray *subfolderArray = @[@"/anime", @"/bg", @"/bgm", @"/cg", @"/ch", @"/conf", @"/cv", @"/font", @"/gui", @"/mov", @"/rule", @"/se", @"/txt", @"/wms", @"/.vscode"];
    for (NSString *sub in subfolderArray) {
        NSString *src = [NSString stringWithFormat:@"%@/Contents/Resources/%@%@", [[NSBundle mainBundle] bundlePath], from, sub];
        NSString *dst = [[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:sub];
        if (![[NSFileManager defaultManager] copyItemAtPath:src toPath:dst error:nil]) {
            NSAlert *alert = [[NSAlert alloc] init];
            [alert setMessageText:_isEnglish ?
                @"Failed to copy files. The folder you selected might not not empty. Create new one and choose it again." :
                @"ファイルのコピーに失敗しました。フォルダが空ではありません。フォルダを新規作成し、選択しなおしてください。"];
            [alert addButtonWithTitle:@"OK"];
            [alert setAlertStyle:NSAlertStyleCritical];
            [alert runModal];
            return NO;
        }
    }
    return YES;
}

- (BOOL)openProject {
    while (YES) {
        NSOpenPanel *panel = [NSOpenPanel openPanel];
        [panel setCanChooseDirectories:YES];
        if ([panel runModal] != NSModalResponseOK)
            return NO;

        NSString *path = [[panel URL] path];
        if ([[path lastPathComponent] hasSuffix:@"polarisengine"]) {
            NSAlert *alert = [[NSAlert alloc] init];
            [alert setMessageText:_isEnglish ?
             @"Please select a game folder, not a project file." :
             @"プロジェクトファイルではなくゲームフォルダを選択してください。"];
            [alert addButtonWithTitle:@"OK"];
            [alert setAlertStyle:NSAlertStyleCritical];
            [alert runModal];
            continue;
        }

        NSFileManager *fileManager = [NSFileManager defaultManager];
        if (![fileManager isReadableFileAtPath:[path stringByAppendingString:@"/conf/config.txt"]]) {
            NSAlert *alert = [[NSAlert alloc] init];
            [alert setMessageText:_isEnglish ?
                @"The folder you selected doesn't contain a game. Please select a game folder." :
                @"ゲームフォルダではありません。ゲームフォルダを選択してください。"];
            [alert addButtonWithTitle:@"OK"];
            [alert setAlertStyle:NSAlertStyleCritical];
            [alert runModal];
            continue;
        }

        [fileManager changeCurrentDirectoryPath:path];
        break;
    }

    return YES;
}

//
// GameViewControllerProtocol
//

- (float)screenScale {
    return _screenScale;
}

- (NSPoint)screenOffset {
    return _screenOffset;
}

- (NSSize)screenSize {
    return _screenSize;
}

- (CGPoint)windowPointToScreenPoint:(CGPoint)windowPoint {
    float retinaScale = _renderView.layer.contentsScale;

    int x = (int)(windowPoint.x - (_screenOffset.x / retinaScale)) * _screenScale;
    int y = (int)(windowPoint.y - (_screenOffset.y / retinaScale)) * _screenScale;

    return CGPointMake(x, conf_window_height - y);
}

- (BOOL)isVideoPlaying {
    return _isVideoPlaying;
}

- (void)playVideoWithPath:(NSString *)path skippable:(BOOL)isSkippable {
    // プレーヤーを作成する
    NSURL *url = [NSURL URLWithString:[@"file://" stringByAppendingString:path]];
    AVPlayerItem *playerItem = [[AVPlayerItem alloc] initWithURL:url];
    _avPlayer = [[AVPlayer alloc] initWithPlayerItem:playerItem];

    // プレーヤーのレイヤーを作成する
    [self.view setWantsLayer:YES];
    _avPlayerLayer = [AVPlayerLayer playerLayerWithPlayer:_avPlayer];
    [_avPlayerLayer setFrame:_renderView.bounds];
    [_renderView.layer addSublayer:_avPlayerLayer];

    // 再生終了の通知を送るようにする
    [NSNotificationCenter.defaultCenter addObserver:self
                                           selector:@selector(onPlayEnd:)
                                               name:AVPlayerItemDidPlayToEndTimeNotification
                                             object:playerItem];

    // 再生を開始する
    [_avPlayer play];

    _isVideoPlaying = YES;
}

- (void)onPlayEnd:(NSNotification *)notification {
    [_avPlayer replaceCurrentItemWithPlayerItem:nil];
    _isVideoPlaying = NO;
}

- (void)stopVideo {
    if (_avPlayer != nil) {
        [_avPlayer replaceCurrentItemWithPlayerItem:nil];
        _isVideoPlaying = NO;
        _avPlayer = nil;
        _avPlayerLayer = nil;
    }
}

- (void)setWindowTitle:(NSString *)title {
    [self.view.window setTitle:title];
}

- (BOOL)isFullScreen {
    return _isFullScreen;
}

- (void)enterFullScreen {
    if (!_isFullScreen) {
        [self.view.window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
        [self.view.window toggleFullScreen:self];
        _isFullScreen = YES;
    }
}

- (void)leaveFullScreen {
    if (_isFullScreen) {
        [self.view.window toggleFullScreen:self];
        _isFullScreen = NO;
    }
}

//
// IB Actions
//

- (IBAction)onAbout:(id)sender {
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:@"Polaris Engine\nCopyright (C) 2001-2024, The Authors. All rights reserved."];
    [alert setAlertStyle:NSAlertStyleInformational];
    [alert runModal];
}

- (IBAction)onCreateJapaneseLight:(id)sender {
    if ([self createProject:@"japanese-light"])
        [self setupView];
}

- (IBAction)onCreateJapaneseDark:(id)sender {
    if ([self createProject:@"japanese-dark"])
        [self setupView];
}

- (IBAction)onCreateJapaneseNovel:(id)sender {
    if ([self createProject:@"japanese-novel"])
        [self setupView];
}

- (IBAction)onCreateJapaneseTategaki:(id)sender {
    if ([self createProject:@"japanese-tategaki"])
        [self setupView];
}

- (IBAction)onCreateEnglish:(id)sender {
    if ([self createProject:@"english"])
        [self setupView];
}

- (IBAction)onCreateEnglishNovel:(id)sender {
    if ([self createProject:@"english-novel"])
        [self setupView];
}

- (IBAction)onOpenGame:(id)sender {
    if ([self openProject])
        [self setupView];
}

- (IBAction) onContinueButton:(id)sender {
    self.isContinuePressed = true;
}

- (IBAction) onNextButton:(id)sender {
    self.isNextPressed = true;
}

- (IBAction) onStopButton:(id)sender {
    self.isStopPressed = true;
}

- (IBAction) onMoveButton:(id)sender {
    [self updateScriptModelFromText];
    [self setTextColorForAllLinesDelayed];
    self.isExecLineChanged = YES;
    self.changedExecLine = [self scriptCursorLine];
}

- (IBAction)onOpenScriptButton:(id)sender {
    NSString *basePath = [[NSFileManager defaultManager] currentDirectoryPath];
    NSString *txtPath = [NSString stringWithFormat:@"%@/%@", basePath, @"txt"];
    NSOpenPanel *panel= [NSOpenPanel openPanel];
    [panel setAllowedFileTypes:[NSArray arrayWithObjects:@"s2sc", @"txt", @"stxt", @"ks", @"'TEXT'", nil]];
    [panel setDirectoryURL:[[NSURL alloc] initFileURLWithPath:txtPath]];
    if ([panel runModal] == NSModalResponseOK) {
        NSString *choose = [[panel URL] path];
        if ([choose hasPrefix:txtPath] && [choose length] > [txtPath length] + 1) {
                self.textFieldScriptName.stringValue = [choose substringFromIndex:[txtPath length] + 1];
                self.isOpenScriptPressed = true;
        }
    }
}

- (IBAction)onUpdateVariablesButton:(id)sender {
    // テキストフィールドの内容を取得する
    NSString *text = self.textFieldVariables.stringValue;

    // パースする
    const char *p = [text UTF8String];
    const char *next_line;
    while (*p) {
        /* 空行を読み飛ばす */
        if (*p == '\n') {
            p++;
            continue;
        }

        // 次の行の開始文字を探す
        next_line = p;
        while (*next_line) {
            if (*next_line == '\n') {
                next_line++;
                break;
            }
            next_line++;
        }

        // パースする
        int index, val;
        if (sscanf(p, "$%d=%d\n", &index, &val) != 2)
            index = val = -1;
        if (index >= LOCAL_VAR_SIZE + GLOBAL_VAR_SIZE)
            index = -1;

        // 変数を設定する
        if (index != -1)
            set_variable(index, val);

        // 次の行へポインタを進める
        p = next_line;
    }

    // 変更された後の変数でテキストフィールドを更新する
    [self updateVars];
}

- (IBAction)onQuit:(id)sender {
    [NSApp stop:nil];
}

- (IBAction)onMenuOpenGameFolder:(id)sender {
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSURL *url = [NSURL fileURLWithPath:[fileManager currentDirectoryPath]];
    [[NSWorkspace sharedWorkspace] openURL:url];
}

- (IBAction)onMenuOpenScript:(id)sender {
    // .appバンドルのパスを取得する
    NSString *bundlePath = [[NSBundle mainBundle] bundlePath];

    // .appバンドルの1つ上のディレクトリのパスを取得する
    NSString *basePath = [bundlePath stringByDeletingLastPathComponent];

    // txtディレクトリのパスを取得する
    NSString *txtPath = [NSString stringWithFormat:@"%@/%@", basePath, @"txt"];

    // 開くダイアログを作る
    NSOpenPanel *panel= [NSOpenPanel openPanel];
    [panel setAllowedFileTypes:
               [NSArray arrayWithObjects:@"s2sc", @"txt", @"stxt", @"ks", @"'TEXT'", nil]];
    [panel setDirectoryURL:[[NSURL alloc] initFileURLWithPath:txtPath]];
    if ([panel runModal] == NSModalResponseOK) {
        NSString *file = [[panel URL] lastPathComponent];
        if ([file hasPrefix:txtPath]) {
                self.textFieldScriptName.stringValue = file;
                self.isOpenScriptPressed = true;
        }
    }
}

- (IBAction)onMenuReloadScript:(id)sender {
    _isOpenScriptPressed = YES;
    _isExecLineChanged = YES;
    _changedExecLine = get_expanded_line_num();
}

- (IBAction)onMenuSave:(id)sender {
    // スクリプトファイル名を取得する
    const char *scr = get_script_file_name();
    if(strcmp(scr, "DEBUG") == 0)
        return;

    // 確認のダイアログを開く
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:self.isEnglish ?
           @"Are you sure you want to overwrite the script file?" :
           @"スクリプトファイルを上書き保存します。\nよろしいですか？"];
    [alert addButtonWithTitle:!self.isEnglish ? @"はい" : @"Yes"];
    [alert addButtonWithTitle:!self.isEnglish ? @"いいえ" : @"No"];
    [alert setAlertStyle:NSAlertStyleWarning];
    if([alert runModal] != NSAlertFirstButtonReturn)
        return;

    [self updateScriptModelFromText];
    save_script();
}

- (IBAction)onMenuContinue:(id)sender {
    [self onContinueButton:sender];
}

- (IBAction)onMenuNext:(id)sender {
    [self onNextButton:sender];
}

- (IBAction)onMenuStop:(id)sender {
    [self onStopButton:sender];
}

- (IBAction)onMenuNextError:(id)sender {
    // 行数を取得する
    int lines = get_line_count();

    // カーソル行を取得する
    NSRange sel = [self.textViewScript selectedRange];
    NSString *viewContent = [self.textViewScript string];
    NSRange lineRange = [viewContent lineRangeForRange:NSMakeRange(sel.location, 0)];
    int start = (int)lineRange.location;

    // カーソル行より下を検索する
    for (int i = start + 1; i < lines; i++) {
        const char *text = get_line_string_at_line_num(i);
        if(text[0] == '!') {
            // みつかったので選択する
            [self scrollToLine:i];
            //[self.textViewScript scrollRowToVisible:i];
            return;
        }
    }

    // 先頭行から、選択されている行までを検索する
    if (start != 0) {
        for (int i = 0; i <= start; i++) {
            const char *text = get_line_string_at_line_num(i);
            if(text[0] == '!') {
                // みつかったので選択する
                [self scrollToLine:i];
                //[self.textViewScript scrollRowToVisible:i];
                return;
            }
        }
    }

    log_info(self.isEnglish ? "No error." : "エラーはありません。");
}

- (void)insertText:(NSString *)command {
    NSString *text = _textViewScript.string;
    NSArray *lines = [text componentsSeparatedByString:@"\n"];
    int cur = (int)_textViewScript.selectedRange.location;
    int lineCount = 0;
    int lineTop = 0;
    for (NSString *line in lines) {
        int lineLen = (int)line.length;
        if (cur >= lineTop && cur <= lineTop + lineLen)
            break;
        lineCount++;
        lineTop += lineLen + 1;
    }

    [_textViewScript insertText:command replacementRange:NSMakeRange(lineTop, 0)];
}

- (IBAction)onMenuMessage:(id)sender {
    if (self.isEnglish)
        [self insertText:@"Edit this message and press return."];
    else
        [self insertText:@"この行のメッセージを編集して改行してください。"];
}

- (IBAction)onMenuLine:(id)sender {
    if (self.isEnglish)
        [self insertText:@"*Name*Edit this line and press return."];
    else
        [self insertText:@"名前「このセリフを編集して改行してください。」"];
}

- (IBAction)onMenuLineWithVoice:(id)sender {
    if (self.isEnglish)
        [self insertText:@"*Name*001.ogg*Edit this line and press return."];
    else
        [self insertText:@"*名前*001.ogg*このセリフを編集して改行してください。"];
}

- (IBAction)onMenuBackground:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/bg"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@bg file=%@ duration=1.0", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@背景 ファイル=%@ 秒=1.0", file]];
}

- (IBAction)onMenuBackgroundOnly:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/bg"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@chsx bg=%@ duration=1.0", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@場面転換X 背景=%@ 秒=1.0", file]];
}

- (IBAction)onMenuShowLeftCharacter:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/ch"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@ch position=left file=%@ duration=1.0", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@キャラ 位置=左 ファイル=%@ 秒=1.0", file]];
}

- (IBAction)onMenuHideLeftCharacter:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@ch position=left file=none duration=1.0"];
    else
        [self insertText:@"@キャラ 位置=左 ファイル=なし 秒=1.0"];
}

- (IBAction)onMenuShowLeftCenterCharacter:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/ch"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@ch position=left-center file=%@ duration=1.0", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@キャラ 位置=左中央 ファイル=%@ 秒=1.0", file]];
}

- (IBAction)onMenuHideLeftCenterCharacter:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@ch position=left-center file=none duration=1.0"];
    else
        [self insertText:@"@キャラ 位置=左中央 ファイル=なし 秒=1.0"];
}

- (IBAction)onMenuShowCenterCharacter:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/ch"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@ch position=center file=%@ duration=1.0", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@キャラ 位置=中央 ファイル=%@ 秒=1.0", file]];
}

- (IBAction)onMenuHideCenterCharacter:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@ch position=center file=none duration=1.0"];
    else
        [self insertText:@"@キャラ 位置=中央 ファイル=なし 秒=1.0"];
}

- (IBAction)onMenuShowRightwCenterCharacter:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/ch"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@ch position=right-center file=%@ duration=1.0", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@キャラ 位置=右中央 ファイル=%@ 秒=1.0", file]];
}

- (IBAction)onMenuHideRightCenterCharacter:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@ch position=right-center file=none duration=1.0"];
    else
        [self insertText:@"@キャラ 位置=右中央 ファイル=なし 秒=1.0"];
}

- (IBAction)onMenuShowRightCharacter:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/ch"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@ch position=right file=%@ duration=1.0", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@キャラ 位置=右 ファイル=%@ 秒=1.0", file]];
}

- (IBAction)onMenuHideRightCharacter:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@ch position=right file=none duration=1.0"];
    else
        [self insertText:@"@キャラ 位置=右 ファイル=なし 秒=1.0"];
}

- (IBAction)onMenuChsx:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@chsx left=stay center=stay right=stay duration=1.0"];
    else
        [self insertText:@"@場面転換X left=stay center=stay right=stay duration=1.0"];
}

- (IBAction)onMenuBgmPlay:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/bgm"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@bgm file=%@", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@音楽 ファイル=%@", file]];
}

- (IBAction)onMenuBgmStop:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@bgm file=stop"];
    else
        [self insertText:@"@音楽 ファイル=停止"];
}

- (IBAction)onMenuBgmVolume:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@vol track=bgm volume=1.0"];
    else
        [self insertText:@"@音量 トラック=bgm 音量=1.0"];
}

- (IBAction)onMenuSePlay:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/se"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@se file=%@", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@効果音 ファイル=%@", file]];
}

- (IBAction)onMenuSeStop:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@se file=stop"];
    else
        [self insertText:@"@効果音 ファイル=停止"];
}

- (IBAction)onMenuSeVolume:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@vol track=se volume=1.0"];
    else
        [self insertText:@"@音量 トラック=se 音量=1.0"];
}

- (IBAction)onMenuVoicePlay:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/cv"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@se file=%@ voice", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@効果音 ファイル=%@ voice", file]];
}

- (IBAction)onMenuVoiceStop:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@se file=stop voice"];
    else
        [self insertText:@"@効果音 ファイル=停止 voice"];
}

- (IBAction)onMenuVoiceVolume:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@vol track=voice volume=1.0"];
    else
        [self insertText:@"@音量 トラック=voice 音量=1.0"];
}

- (IBAction)onMenuChoose1:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@choose Label1 \"Option1\""];
    else
        [self insertText:@"@選択肢 ラベル1 \"選択肢１\""];
}

- (IBAction)onMenuChoose2:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@choose Label1 \"Option1\" Label2 \"Option2\""];
    else
        [self insertText:@"@選択肢 ラベル1 \"選択肢１\" ラベル2 \"選択肢２\""];
}

- (IBAction)onMenuChoose3:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@choose Label1 \"Option1\" Label2 \"Option2\" Label3 \"Option3\""];
    else
        [self insertText:@"@選択肢 ラベル1 \"選択肢１\" ラベル2 \"選択肢２\" ラベル3 \"選択肢３\""];
}

- (IBAction)onMenuVideo:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/mov"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@video file=%@", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@動画 ファイル=%@", file]];
}

- (IBAction)onMenuClick:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@click"];
    else
        [self insertText:@"@クリック待ち"];
}

- (IBAction)onMenuWait:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@wait duration=1.0"];
    else
        [self insertText:@"@時間待ち 秒=1.0"];
}

- (IBAction)onMenuGUI:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/gui"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@gui file=%@", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@メニュー ファイル=%@", file]];
}

- (IBAction)onMenuWMS:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/wms"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@wms file=%@", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@スクリプト ファイル=%@", file]];
}

- (IBAction)onMenuLoad:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/txt"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@load file=%@", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@シナリオ ファイル=%@", file]];
}

- (IBAction)onMenuExportForMac:(id)sender {
    if (!create_package("")) {
        log_info("Export error.");
        return;
    }

    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *exportPath = [NSString stringWithFormat:@"%@/export-mac", [fileManager currentDirectoryPath]];
    [fileManager removeItemAtURL:[NSURL fileURLWithPath:exportPath] error:nil];
    if (![fileManager createDirectoryAtURL:[NSURL fileURLWithPath:exportPath] withIntermediateDirectories:NO attributes:nil error:nil]) {
        log_warn("mkdir error.");
        return;
    }

    // ビルド済みアプリ出力の確認のダイアログを開く
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:self.isEnglish ?
           @"Would you like to export a prebuilt app?" :
           @"ビルド済みアプリを出力しますか？"];
    [alert addButtonWithTitle:!self.isEnglish ? @"はい" : @"Yes"];
    [alert addButtonWithTitle:!self.isEnglish ? @"いいえ" : @"No"];
    [alert setAlertStyle:NSAlertStyleWarning];
    if([alert runModal] != NSAlertFirstButtonReturn) {
        //
        // ソースツリーを出力する
        //
        NSArray *appArray = @[@"libroot", @"engine-macos", @"engine-macos.xcodeproj", @"Resources"];
        for (NSString *sub in appArray) {
            if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/Contents/Resources/macos-src/%@", [[NSBundle  mainBundle] bundlePath], sub]
                                      toPath:[NSString stringWithFormat:@"%@/%@", exportPath, sub]
                                       error:nil]) {
                log_warn("Copy error (1).");
                return;
            }
        }
        if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/data01.arc", [fileManager currentDirectoryPath]]
                                  toPath:[NSString stringWithFormat:@"%@/Resources/data01.arc", exportPath]
                                   error:nil]) {
            log_warn("Copy error (2).");
            return;
        }
    } else {
        //
        // ビルド済みアプリを出力する
        //
        if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/Contents/Resources/game.dmg", [[NSBundle  mainBundle] bundlePath]]
                                      toPath:[NSString stringWithFormat:@"%@/game.dmg", exportPath]
                                       error:nil]) {
            log_warn("Copy error (1).");
            return;
        }
        if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/data01.arc", [fileManager currentDirectoryPath]]
                                  toPath:[NSString stringWithFormat:@"%@/data01.arc", exportPath]
                                   error:nil]) {
            log_warn("Copy error (2).");
            return;
        }
    }

    log_info(_isEnglish ? "Successflully exported" : "エクスポートに成功しました。");
    [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:[[fileManager currentDirectoryPath] stringByAppendingString:@"/export-mac"]]];
}

- (IBAction)onMenuExportForPC:(id)sender {
    if (!create_package("")) {
        log_info("Export error.");
        return;
    }

    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *exportPath = [NSString stringWithFormat:@"%@/export-pc", [fileManager currentDirectoryPath]];
    [fileManager removeItemAtURL:[NSURL fileURLWithPath:exportPath] error:nil];
    if (![fileManager createDirectoryAtURL:[NSURL fileURLWithPath:exportPath] withIntermediateDirectories:NO attributes:nil error:nil]) {
        log_warn("mkdir error.");
        return;
    }

    if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/data01.arc", [fileManager currentDirectoryPath]] toPath:[NSString stringWithFormat:@"%@/export-pc/data01.arc", [fileManager currentDirectoryPath]] error:nil]) {
        log_warn("Copy error (1).");
        return;
    }

    if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/Contents/Resources/game.exe", [[NSBundle mainBundle] bundlePath]] toPath:[NSString stringWithFormat:@"%@/export-pc/game.exe", [fileManager currentDirectoryPath]] error:nil]) {
        log_warn("Copy error (2).");
        return;
    }

    log_info(_isEnglish ? "Successflully exported" : "エクスポートに成功しました。");

    [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:[[fileManager currentDirectoryPath] stringByAppendingString:@"/export-pc"]]];
}

- (IBAction)onMenuExportForWeb:(id)sender {
    if (!create_package("")) {
        log_info("Export error.");
        return;
    }

    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *exportPath = [NSString stringWithFormat:@"%@/export-web", [fileManager currentDirectoryPath]];
    [fileManager removeItemAtURL:[NSURL fileURLWithPath:exportPath] error:nil];
    if (![fileManager createDirectoryAtURL:[NSURL fileURLWithPath:exportPath] withIntermediateDirectories:NO attributes:nil error:nil]) {
        log_warn("mkdir error.");
        return;
    }

    if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/data01.arc", [fileManager currentDirectoryPath]]
                            toPath:[NSString stringWithFormat:@"%@/data01.arc", exportPath]
                             error:nil]) {
        log_warn("Copy error (1).");
        return;
    }

    NSArray *appArray = @[@"index.html", @"index.js", @"index.wasm"];
    for (NSString *sub in appArray) {
        if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/Contents/Resources/%@", [[NSBundle  mainBundle] bundlePath], sub]
                                  toPath:[NSString stringWithFormat:@"%@/%@", exportPath, sub]
                                   error:nil]) {
            log_warn("Copy error (2).");
            return;
        }
    }
    
    log_info(_isEnglish ? "Successflully exported" : "エクスポートに成功しました。");
    
    [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:[[fileManager currentDirectoryPath] stringByAppendingString:@"/export-web"]]];
}

- (IBAction)onMenuExportForIOS:(id)sender {
    if (!create_package("")) {
        log_info("Export error.");
        return;
    }

    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *exportPath = [NSString stringWithFormat:@"%@/export-ios", [fileManager currentDirectoryPath]];
    [fileManager removeItemAtURL:[NSURL fileURLWithPath:exportPath] error:nil];
    if (![fileManager createDirectoryAtURL:[NSURL fileURLWithPath:exportPath] withIntermediateDirectories:NO attributes:nil error:nil]) {
        log_warn("mkdir error.");
        return;
    }

    NSArray *appArray = @[@"libroot-device", @"libroot-sim", @"engine-ios", @"engine-ios.xcodeproj", @"Resources", @"build.sh", @"ExportOptions.plist"];
    for (NSString *sub in appArray) {
        if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/Contents/Resources/ios-src/%@", [[NSBundle  mainBundle] bundlePath], sub]
                                  toPath:[NSString stringWithFormat:@"%@/export-ios/%@", [fileManager currentDirectoryPath], sub]
                                   error:nil]) {
            log_warn("Copy error (1).");
            return;
        }
    }

    if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/data01.arc", [fileManager currentDirectoryPath]]
                              toPath:[NSString stringWithFormat:@"%@/Resources/data01.arc", exportPath]
                               error:nil]) {
        log_warn("Copy error (2).");
        return;
    }

    NSArray *contents = [fileManager contentsOfDirectoryAtPath:[NSString stringWithFormat:@"%@/mov", [fileManager currentDirectoryPath]] error:nil];
    for (NSString *item in contents) {
        if ([[item pathExtension] isEqualToString:@"mp4"]) {
            if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/mov/%@", [fileManager currentDirectoryPath], item]
                                      toPath:[NSString stringWithFormat:@"%@/Resources/mov/%@", exportPath, item]
                                       error:nil]) {
                log_warn("Copy error (3).");
                return;
            }
        }
    }

    log_info(_isEnglish ? "Successflully exported" : "エクスポートに成功しました。");

    // IPA作成の確認のダイアログを開く
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:self.isEnglish ?
           @"Would you like to build an IPA file? (You are required to be an Apple Developer Program member.)" :
           @"IPAファイルをビルドしますか？ (Apple Developer Programに加入しておく必要があります)"];
    [alert addButtonWithTitle:!self.isEnglish ? @"はい" : @"Yes"];
    [alert addButtonWithTitle:!self.isEnglish ? @"いいえ" : @"No"];
    [alert setAlertStyle:NSAlertStyleWarning];
    if([alert runModal] != NSAlertFirstButtonReturn) {
        [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:[[fileManager currentDirectoryPath] stringByAppendingString:@"/export-ios"]]];
        return;
    }

    NSString *command = [NSString stringWithFormat:@"osascript -e \'tell application \"Terminal\" to do script \"%@/build.sh\"\'", exportPath];
    system([command UTF8String]);
}

- (IBAction)onMenuExportForAndroid:(id)sender {
    if (!create_package("")) {
        log_info("Export error.");
        return;
    }

    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *exportPath = [NSString stringWithFormat:@"%@/export-android", [fileManager currentDirectoryPath]];
    [fileManager removeItemAtURL:[NSURL fileURLWithPath:exportPath] error:nil];
    if (![fileManager createDirectoryAtURL:[NSURL fileURLWithPath:exportPath] withIntermediateDirectories:NO attributes:nil error:nil]) {
        log_warn("mkdir error (1).");
        return;
    }

    NSArray *appArray = @[@"app", @"gradle", @"gradlew", @"settings.gradle", @"build.gradle", @"gradle.properties", @"gradlew.bat", @"build.sh"];
    for (NSString *sub in appArray) {
        if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/Contents/Resources/android-src/%@", [[NSBundle  mainBundle] bundlePath], sub]
                                  toPath:[NSString stringWithFormat:@"%@/%@", exportPath, sub]
                                   error:nil]) {
            log_warn("Copy error (1).");
            return;
        }
    }

    if (![fileManager createDirectoryAtURL:[NSURL fileURLWithPath:[exportPath stringByAppendingString:@"/app/src/main/assets"]] withIntermediateDirectories:YES attributes:nil error:nil]) {
        log_warn("mkdir error (2).");
        return;
    }

    NSArray *subfolderArray = @[@"anime", @"bg", @"bgm", @"cg", @"ch", @"conf", @"cv", @"font", @"gui", @"mov", @"rule", @"se", @"txt", @"wms"];
    for (NSString *sub in subfolderArray) {
        if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/%@", [fileManager currentDirectoryPath], sub]
                                  toPath:[NSString stringWithFormat:@"%@/app/src/main/assets/%@", exportPath, sub]
                                   error:nil]) {
            log_warn("Copy error (2).");
            return;
        }
    }
 
    log_info(_isEnglish ? "Successflully exported" : "エクスポートに成功しました。");

    // IPA作成の確認のダイアログを開く
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:self.isEnglish ?
           @"Would you like to build an APK file?" :
           @"APKファイルをビルドしますか？"];
    [alert addButtonWithTitle:!self.isEnglish ? @"はい" : @"Yes"];
    [alert addButtonWithTitle:!self.isEnglish ? @"いいえ" : @"No"];
    [alert setAlertStyle:NSAlertStyleWarning];
    if([alert runModal] != NSAlertFirstButtonReturn) {
        [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:[[fileManager currentDirectoryPath] stringByAppendingString:@"/export-android"]]];
        return;
    }

    NSString *command = [NSString stringWithFormat:@"osascript -e \'tell application \"Terminal\" to do script \"%@/build.sh\"\'", exportPath];
    system([command UTF8String]);
}

- (IBAction)onMenuExportForUnity:(id)sender {
    if (!create_package("")) {
        log_info("Export error.");
        return;
    }
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *exportPath = [NSString stringWithFormat:@"%@/export-unity", [fileManager currentDirectoryPath]];
    [fileManager removeItemAtURL:[NSURL fileURLWithPath:exportPath] error:nil];
    if (![fileManager createDirectoryAtURL:[NSURL fileURLWithPath:exportPath] withIntermediateDirectories:NO attributes:nil error:nil]) {
        log_warn("mkdir error (1).");
        return;
    }
    
    NSArray *appArray = @[@"Assets", @"Library", @"Packages", @"ProjectSettings", @"dll-src", @"Makefile", @"README.txt", @"libpolarisengine-win64.dll", @"libpolarisengine-macos.dylib"];
    for (NSString *sub in appArray) {
        if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/Contents/Resources/unity-src/%@", [[NSBundle  mainBundle] bundlePath], sub]
                                  toPath:[NSString stringWithFormat:@"%@/%@", exportPath, sub]
                                   error:nil]) {
            log_warn("Copy error (2).");
            return;
        }
    }

    if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/Contents/Resources/unity-src/libpolarisengine-macos.dylib", [[NSBundle  mainBundle] bundlePath]]
                              toPath:[NSString stringWithFormat:@"%@/Assets/libpolarisengine.dylib", exportPath]
                               error:nil]) {
        log_warn("Copy error (3).");
        return;
    }

    NSArray *subfolderArray = @[@"anime", @"bg", @"bgm", @"cg", @"ch", @"conf", @"cv", @"font", @"gui", @"mov", @"rule", @"se", @"txt", @"wms"];
    for (NSString *sub in subfolderArray) {
        if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/%@", [fileManager currentDirectoryPath], sub]
                                  toPath:[NSString stringWithFormat:@"%@/Assets/StreamingAssets/%@", exportPath, sub]
                                   error:nil]) {
            log_warn("Copy error (4).");
            return;
        }
    }

    log_info(_isEnglish ? "Successflully exported" : "エクスポートに成功しました。");
    
    [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:[[fileManager currentDirectoryPath] stringByAppendingString:@"/export-unity"]]];
}

- (IBAction)onMenuExportPackage:(id)sender {
    if (!create_package("")) {
        log_info("Export error.");
        return;
    }

    log_info(_isEnglish ? "Successflully exported" : "エクスポートに成功しました。");
    
    [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:[[NSFileManager defaultManager] currentDirectoryPath]]];
}

- (IBAction)onHelp:(id)sender {
    if (_isEnglish) {
        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://Polaris Engine.com/en/wiki/"]];
    } else {
        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://Polaris Engine.com/wiki/"]];
    }
}

//
// スクリプトのテキストビュー
//

// テキストビューの内容をスクリプトモデルを元に設定する
- (void)updateTextFromScriptModel {
    // 行を連列してスクリプト文字列を作成する
    NSString *text = @"";
    for (int i = 0; i < get_line_count(); i++) {
        const char *cstr = get_line_string_at_line_num(i);
        NSString *line = [[NSString alloc] initWithUTF8String:cstr];
        text = [text stringByAppendingString:line];
        text = [text stringByAppendingString:@"\n"];
    }

    // テキストビューにテキストを設定する
    _isFirstChange = TRUE;
    self.textViewScript.string = text;
}

// テキストビューの内容を元にスクリプトモデルを更新する
- (void)updateScriptModelFromText {
    // パースエラーをリセットして、最初のパースエラーで通知を行う
    dbg_reset_parse_error_count();

    // リッチエディットのテキストの内容でスクリプトの各行をアップデートする
    NSString *text = _textViewScript.string;
    NSArray *lines = [text componentsSeparatedByString:@"\n"];
    int lineNum = 0;
    for (NSString *line in lines) {
        if (lineNum < get_line_count())
            update_script_line(lineNum, [line UTF8String]);
        else
            insert_script_line(lineNum, [line UTF8String]);
        lineNum++;
    }

    // メッセージになった拡張構文を再度パースする
    reparse_script_for_structured_syntax();
    
    // 削除された末尾の行を処理する
    self.isExecLineChanged = FALSE;
    for (int i = get_line_count() - 1; i >= lineNum; i--)
        if (delete_script_line(i))
            self.isExecLineChanged = TRUE;
    if (self.isExecLineChanged)
        [self setTextColorForAllLinesDelayed];

    // コマンドのパースに失敗した場合
    if (dbg_get_parse_error_count() > 0) {
        // 行頭の'!'を反映するためにテキストを再設定する
        [self updateTextFromScriptModel];
        [self setTextColorForAllLinesDelayed];
    }
}

// テキストビューの現在の行の内容を元にスクリプトモデルを更新する
- (void)updateScriptModelFromCurrentLineText {
    [self updateScriptModelFromText];
}

- (void)setTextColorForAllLinesDelayed {
    if (_formatTimer == nil) {
        _formatTimer = [NSTimer scheduledTimerWithTimeInterval:1.0f
                                                        target:self
                                                      selector:@selector(delayedFormatTimerFired:)
                                                      userInfo:nil
                                                       repeats:YES];
    }
    _needFormat = YES;
}

- (void)delayedFormatTimerFired:(NSTimer *)timer {
    if (_needFormat)
        [self setTextColorForAllLinesImmediately];
    _needFormat = NO;
}

- (void)setTextColorForAllLinesImmediately {
    // すべてのテキスト装飾を削除する
    NSString *text = self.textViewScript.string;
    NSRange allRange = NSMakeRange(0, [text length]);
    [self.textViewScript.textStorage removeAttribute:NSForegroundColorAttributeName range:allRange];
    [self.textViewScript.textStorage removeAttribute:NSBackgroundColorAttributeName range:allRange];

    // 行ごとに処理する
    NSArray *lineArray = [text componentsSeparatedByString:@"\n"];
    int startPos = 0;
    int execLineNum = get_expanded_line_num();
    for (int i = 0; i < lineArray.count; i++) {
        NSString *lineText = lineArray[i];
        NSUInteger lineLen = [lineText length];
        if (lineLen == 0) {
            startPos++;
            continue;
        }

        NSRange lineRange = NSMakeRange(startPos, [lineArray[i] length]);

        // 実行行であれば背景色を設定する
        if (i == execLineNum) {
            NSColor *bgColor = self.isRunning ?
                [NSColor colorWithRed:0xff/255.0f green:0xc0/255.0f blue:0xc0/255.0f alpha:1.0f] :
                [NSColor colorWithRed:0xc0/255.0f green:0xc0/255.0f blue:0xff/255.0f alpha:1.0f];
            [self.textViewScript.textStorage addAttribute:NSBackgroundColorAttributeName value:bgColor range:lineRange];
        }

        // コメントを処理する
        if ([lineText characterAtIndex:0] == L'#') {
            // 行全体のテキスト色を変更する
            [self.textViewScript.textStorage
                addAttribute:NSForegroundColorAttributeName
                       value:[NSColor colorWithRed:0.5f green:0.5f blue:0.5f alpha:1.0f]
                       range:lineRange];
        }
        // ラベルを処理する
        else if ([lineText characterAtIndex:0] == L':') {
            // 行全体のテキスト色を変更する
            [self.textViewScript.textStorage
                addAttribute:NSForegroundColorAttributeName
                       value:[NSColor colorWithRed:1.0f green:0 blue:0 alpha:1.0f]
                       range:lineRange];
        }
        // エラー行を処理する
        else if ([lineText characterAtIndex:0] == L'!') {
            // 行全体のテキスト色を変更する
            [self.textViewScript.textStorage
                addAttribute:NSForegroundColorAttributeName
                       value:[NSColor colorWithRed:1.0f green:0 blue:0 alpha:1.0f]
                       range:lineRange];
        }
        // コマンド行を処理する
        else if ([lineText characterAtIndex:0] == L'@') {
            // コマンド名部分を抽出する
            NSUInteger commandNameLen = [lineText rangeOfString:@" "].location;
            if (commandNameLen == NSNotFound)
                commandNameLen = [lineText length];

            // コマンド名のテキストに色を付ける
            NSRange commandRange = NSMakeRange(startPos, commandNameLen);
            int commandType = get_command_type_from_name([[lineText substringToIndex:commandNameLen] UTF8String]);
            NSColor *commandColor = commandType != COMMAND_CIEL ?
                                    [NSColor colorWithRed:0 green:0 blue:1.0f alpha:1.0f] :
                                    [NSColor colorWithRed:0.36 green:0.65 blue:0.80f alpha:1.0f];
            [self.textViewScript.textStorage
                addAttribute:NSForegroundColorAttributeName
                       value:commandColor
                       range:commandRange];

            // 引数に色を付ける
            if (commandType != COMMAND_SET && commandType != COMMAND_IF &&
                commandType != COMMAND_UNLESS && commandType != COMMAND_PENCIL &&
                [lineText length] != commandNameLen) {
                // 引数名を灰色にする
                NSUInteger paramStart = startPos + commandNameLen;
                NSUInteger ofs = commandNameLen;
                do {
                    NSString *sub = [lineText substringFromIndex:ofs + 1];
                    if ([sub length] == 0)
                        break;

                    // '='を探す
                    NSUInteger eqPos = [sub rangeOfString:@"="].location;
                    if (eqPos == NSNotFound)
                        break;

                    // '='の手前に' 'があればスキップする
                    NSUInteger spacePos = [sub rangeOfString:@" "].location;
                    if (spacePos != NSNotFound && spacePos < eqPos) {
                        ofs += spacePos + 1;
                        paramStart += spacePos + 1;
                        continue;
                    }

                    // 引数名部分のテキスト色を変更する
                    NSRange paramNameRange = NSMakeRange(paramStart, eqPos + 2);
                    [self.textViewScript.textStorage addAttribute:NSForegroundColorAttributeName
                               value:[NSColor colorWithRed:0xc0/255.0f green:0xf0/255.0f blue:0xc0/255.0f alpha:1.0f]
                               range:paramNameRange];

                    ofs += spacePos + 1;
                    paramStart += spacePos + 1;
                } while (paramStart < lineLen);
            }
        }
        
        startPos += [lineText length] + 1;
    }
}

- (void)onScriptEnter {
    [self updateScriptModelFromText];
    self.changedExecLine = [self scriptCursorLine];
    self.isExecLineChanged = YES;
    self.isNextPressed = YES;
}

- (void)onScriptChange {
    [self setTextColorForAllLinesDelayed];
}

- (int)scriptCursorLine {
    NSString *text = _textViewScript.string;
    NSArray *lines = [text componentsSeparatedByString:@"\n"];
    int cur = (int)_textViewScript.selectedRange.location;
    int lineCount = 0;
    int lineTop = 0;
    for (NSString *line in lines) {
        int lineLen = (int)line.length;
        if (cur >= lineTop && cur <= lineTop + lineLen)
            return lineCount;
        lineCount++;
        lineTop += lineLen + 1;
    }
    return 0;
}

- (void)scrollToExecLine {
    int line = get_expanded_line_num();
    [self scrollToLine:line];
}

- (void)scrollToLine:(int)lineToScroll {
    NSString *text = _textViewScript.string;
    NSArray *lines = [text componentsSeparatedByString:@"\n"];
    int lineCount = 0;
    int lineTop = 0;
    for (NSString *line in lines) {
        int lineLen = (int)line.length;
        if (lineCount == lineToScroll) {
            NSRange caretRange = NSMakeRange(lineTop, 0);
            _textViewScript.selectedRange = caretRange;
            NSLayoutManager *layoutManager = [_textViewScript layoutManager];
            NSRange glyphRange = [layoutManager glyphRangeForCharacterRange:caretRange actualCharacterRange:nil];
            NSRect glyphRect = [layoutManager boundingRectForGlyphRange:glyphRange inTextContainer:[_textViewScript textContainer]];
            [_textViewScript scrollRectToVisible:glyphRect];
            break;
        }
        lineCount++;
        lineTop += lineLen + 1;
    }
}

//
// Variables
//

- (void)updateVars {
    int index, val;
    NSMutableString *text = [NSMutableString new];
    for (index = 0; index < LOCAL_VAR_SIZE + GLOBAL_VAR_SIZE; index++) {
        // 変数が初期値の場合
        val = get_variable(index);
        if(val == 0 && !is_variable_changed(index))
            continue;
        
        // 行を追加する
        [text appendString:[NSString stringWithFormat:@"$%d=%d\n", index, val]];
    }
    
    theViewController.textFieldVariables.stringValue = text;
    self.isVarsUpdated = NO;
}

@end

//
// Main HAL
//

//
// INFOログを出力する
//
bool log_info(const char *s, ...)
{
    @autoreleasepool {
        char buf[1024];
        va_list ap;
    
        va_start(ap, s);
        vsnprintf(buf, sizeof(buf), s, ap);
        va_end(ap);

        // アラートを表示する
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_INFO)]];
        NSString *text = [[NSString alloc] initWithUTF8String:buf];
        if (![text canBeConvertedToEncoding:NSUTF8StringEncoding])
            text = @"(invalid utf-8 string)";
        [alert setInformativeText:text];
        [alert runModal];

        return true;
    }
}

//
// WARNログを出力する
//
bool log_warn(const char *s, ...)
{
    @autoreleasepool {
        char buf[1024];
        va_list ap;

        va_start(ap, s);
        vsnprintf(buf, sizeof(buf), s, ap);
        va_end(ap);

        // アラートを表示する
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_WARN)]];
        NSString *text = [[NSString alloc] initWithUTF8String:buf];
        if (![text canBeConvertedToEncoding:NSUTF8StringEncoding])
            text = @"(invalid utf-8 string)";
        [alert setInformativeText:text];
        [alert runModal];

        return true;
    }
}

//
// Errorログを出力する
//
bool log_error(const char *s, ...)
{
    @autoreleasepool {
        char buf[1024];
        va_list ap;

        va_start(ap, s);
        vsnprintf(buf, sizeof(buf), s, ap);
        va_end(ap);

        // アラートを表示する
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_ERROR)]];
        NSString *text = [[NSString alloc] initWithUTF8String:buf];
        if (![text canBeConvertedToEncoding:NSUTF8StringEncoding])
            text = @"(invalid utf-8 string)";
        [alert setInformativeText:text];
        [alert runModal];
        return true;
    }
}

//
// セーブディレクトリを作成する
//
bool make_sav_dir(void)
{
    @autoreleasepool {
        NSString *basePath = [[NSFileManager defaultManager] currentDirectoryPath];
        NSString *savePath = [NSString stringWithFormat:@"%@/%s", basePath, SAVE_DIR];
        NSError *error;
        [[NSFileManager defaultManager] createDirectoryAtPath:savePath
                                  withIntermediateDirectories:NO
                                                   attributes:nil
                                                        error:&error];
        return true;
    }
}

//
// データファイルのディレクトリ名とファイル名を指定して有効なパスを取得する
//
char *make_valid_path(const char *dir, const char *fname)
{
    @autoreleasepool {
        NSString *basePath = [[NSFileManager defaultManager] currentDirectoryPath];
        NSString *filePath;
        if (dir != NULL) {
            if (fname != NULL)
                filePath = [NSString stringWithFormat:@"%@/%s/%s", basePath, dir, fname];
            else
                filePath = [NSString stringWithFormat:@"%@/%s", basePath, dir];
        } else {
            if (fname != NULL)
                filePath = [NSString stringWithFormat:@"%@/%s", basePath, fname];
            else
                filePath = basePath;
        }
        const char *cstr = [filePath UTF8String];
        char *ret = strdup(cstr);
        if (ret == NULL) {
            log_memory();
            return NULL;
        }
        return ret;
    }
}

//
// Pro HAL
//

bool is_continue_pushed(void)
{
    @autoreleasepool {
        bool ret = theViewController.isContinuePressed;
        theViewController.isContinuePressed = false;
        return ret;
    }
}

bool is_next_pushed(void)
{
    @autoreleasepool {
        bool ret = theViewController.isNextPressed;
        theViewController.isNextPressed = false;
        return ret;
    }
}

//
// 停止ボタンが押されたか調べる
//
bool is_stop_pushed(void)
{
    @autoreleasepool {
        bool ret = theViewController.isStopPressed;
        theViewController.isStopPressed = false;
        return ret;
    }
}

//
// 実行するスクリプトファイルが変更されたか調べる
//
bool is_script_opened(void)
{
    @autoreleasepool {
        bool ret = theViewController.isOpenScriptPressed;
        theViewController.isOpenScriptPressed = false;
        return ret;
    }
}

//
// 変更された実行するスクリプトファイル名を取得する
//
const char *get_opened_script(void)
{
    static char script[256];

    @autoreleasepool {
        snprintf(script, sizeof(script), "%s", [theViewController.textFieldScriptName.stringValue UTF8String]);
    }

    return script;
}

//
// 実行する行番号が変更されたか調べる
//
bool is_exec_line_changed(void)
{
    @autoreleasepool {
        bool ret = theViewController.isExecLineChanged;
        theViewController.isExecLineChanged = false;
        return ret;
    }
}

//
// 変更された行番号を取得する
//
int get_changed_exec_line(void)
{
    @autoreleasepool {
        return theViewController.changedExecLine;
    }
}

//
// コマンドの実行中状態を設定する
//
void on_change_running_state(bool running, bool request_stop)
{
    if (!theViewController.isInitialized)
        return;

    @autoreleasepool {
        theViewController.isRunning = running ? YES : NO;
        if(request_stop) {
            // Running and there is a stop request.
            [theViewController.buttonContinue setEnabled:NO];
            [theViewController.buttonNext setEnabled:NO];
            [theViewController.buttonStop setEnabled:NO];
            [theViewController.buttonMove setEnabled:NO];
            [theViewController.buttonOpenScript setEnabled:NO];
            [theViewController.textViewScript setEditable:NO];
            [theViewController.textFieldVariables setEditable:NO];
            [theViewController.buttonUpdateVariables setEnabled:NO];
            for (NSMenuItem *item in [[[[NSApp mainMenu] itemAtIndex:1] submenu] itemArray])
                [item setEnabled:NO];
            for (NSMenuItem *item in [[[[NSApp mainMenu] itemAtIndex:3] submenu] itemArray])
                [item setEnabled:YES];
            [[[[[NSApp mainMenu] itemAtIndex:3] submenu] itemWithTag:112] setEnabled:NO];
            for (NSMenuItem *item in [[[[NSApp mainMenu] itemAtIndex:5] submenu] itemArray])
                [item setEnabled:NO];
            [theViewController setTextColorForAllLinesImmediately];
            return;
        } else if (running) {
            // Running and there is no stop request.
            [theViewController.buttonContinue setEnabled:NO];
            [theViewController.buttonNext setEnabled:NO];
            [theViewController.buttonStop setEnabled:YES];
            [theViewController.buttonMove setEnabled:NO];
            [theViewController.buttonOpenScript setEnabled:NO];
            [theViewController.textViewScript setEditable:YES];
            [theViewController.textFieldVariables setEditable:NO];
            [theViewController.buttonUpdateVariables setEnabled:NO];
            for (NSMenuItem *item in [[[[NSApp mainMenu] itemAtIndex:1] submenu] itemArray])
                [item setEnabled:NO];
            for (NSMenuItem *item in [[[[NSApp mainMenu] itemAtIndex:3] submenu] itemArray])
                [item setEnabled:NO];
            [[[[[NSApp mainMenu] itemAtIndex:3] submenu] itemWithTag:112] setEnabled:YES];
            for (NSMenuItem *item in [[[[NSApp mainMenu] itemAtIndex:5] submenu] itemArray])
                [item setEnabled:NO];
            [theViewController setTextColorForAllLinesImmediately];
            return;
        } else {
            // Stopped.
            [theViewController.buttonContinue setEnabled:YES];
            [theViewController.buttonNext setEnabled:YES];
            [theViewController.buttonStop setEnabled:NO];
            [theViewController.buttonMove setEnabled:YES];
            [theViewController.buttonOpenScript setEnabled:YES];
            [theViewController.textViewScript setEditable:YES];
            [theViewController.textFieldVariables setEditable:YES];
            [theViewController.buttonUpdateVariables setEnabled:YES];
            for (NSMenuItem *item in [[[[NSApp mainMenu] itemAtIndex:1] submenu] itemArray])
                [item setEnabled:YES];
            for (NSMenuItem *item in [[[[NSApp mainMenu] itemAtIndex:3] submenu] itemArray])
                [item setEnabled:YES];
            [[[[[NSApp mainMenu] itemAtIndex:3] submenu] itemWithTag:112] setEnabled:NO];
            for (NSMenuItem *item in [[[[NSApp mainMenu] itemAtIndex:5] submenu] itemArray])
                [item setEnabled:YES];
            [theViewController setTextColorForAllLinesImmediately];
        }
    }
}

void on_load_script(void)
{
    @autoreleasepool {
        NSString *scriptName = [[NSString alloc] initWithUTF8String:get_script_file_name()];
        theViewController.textFieldScriptName.stringValue = scriptName;
        [theViewController updateTextFromScriptModel];
        [theViewController setTextColorForAllLinesDelayed];
    }
}

void on_change_position(void)
{
    @autoreleasepool {
        [theViewController scrollToExecLine];
        [theViewController setTextColorForAllLinesImmediately];
        if (theViewController.isVarsUpdated)
            [theViewController updateVars];
            
    }
}

void on_update_variable(void)
{
    theViewController.isVarsUpdated = YES;
}
