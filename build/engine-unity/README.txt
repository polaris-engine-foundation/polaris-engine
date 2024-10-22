ビルド手順および注意事項
========================

## 注意事項
* `Polaris Engine` の開発者はゲーム機のSDKを持っていないです
* なので想像だけで開発されています
* PCとMac以外での動作は一切確認されていません

## ゲーム機版のビルド手順
* まずSDKのコンパイラを使えるターミナルを開いてください
* ゲーム機ごとの作業
  * PlayStation 4/5の場合、
    * `dll-src/ps45.mk` の先頭2行、`CC=`と`AR=`について、実際のコマンド名に書き換えてください
  * Xbox Series X|Sの場合、
    * `dll-src/xbox.mk` の先頭2行、`CC=`と`AR=`について、実際のコマンド名に書き換えてください
  * Switchの場合、
    * `dll-src/switch.mk` の先頭2行、`CC=`と`AR=`について、実際のコマンド名に書き換えてください
* `make`を実行してください
* そのあとでUnityで開いてください

## Unityで開いた後の作業
* `Player Settings` で `Allow unsafe code` にチェックを入れてください
* `MainScene` をダブルクリックしてください
* `Script` を `PolarisEngineScript.cs` と紐づけしてください
* `BGM`, `SE`, `Voice`, `SYSSE` を `PolarisEngineAudio.cs`と紐づけしてください
* 実行してください
