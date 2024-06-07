ビルド手順および注意事項
========================

## 注意事項
* x-engineの開発者はゲーム機のSDKを持っていないです
* なので想像だけで開発されています

## Windows版のビルド手順
* そのままUnityで開いてください
* `Script`を`XEngineScript.cs`と紐づけしてください
* `BGM`, `SE`, `Voice`, `SysSE`を`XEngineAudio.cs`と紐づけしてください
* これは動作確認されています

## Mac版のビルド手順
* そのままUnityで開いてください
* `Script`を`XEngineScript.cs`と紐づけしてください
* `BGM`, `SE`, `Voice`, `SysSE`を`XEngineAudio.cs`と紐づけしてください
* これはあまり動作確認されていません

## ゲーム機版のビルド手順
* まずSDKのコンパイラを使えるターミナルを開いてください
* `dll-src`フォルダに入ってください
* `Makefile`の先頭2行、CC=とAR=について、実際のコマンド名に書き換えてください
* `make`を実行してください
* そのあとでUnityで開いてください
* `Script`を`XEngineScript.cs`と紐づけしてください
* `BGM`, `SE`, `Voice`, `SysSE`を`XEngineAudio.cs`と紐づけしてください
* これは一切動作確認されていません
