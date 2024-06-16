#!/bin/sh

# Stop when failed.
set -eu

export LANG=en_US.UTF-8
export LANGUAGE=english

#
# Show a welcome message.
#
echo "Hello, this is the release script."

#
# Check if we run on a macOS host.
#
if [ -z "`uname -a | grep Darwin`" ]; then
    echo "Error: please run on macOS.";
    exit 1;
fi

#
# Check for GNU coreutils.
#
SED='sed'
if [ ! -z "`which gsed`" ]; then
    SED='gsed';
fi
HEAD='head'
if [ ! -z "`which ghead`" ]; then
    HEAD='ghead';
fi

#
# Guess the release version number.
#
VERSION=`grep -a1 '<!-- BEGIN-LATEST-JP -->' ../ChangeLog | tail -n1`
VERSION=`echo $VERSION | cut -d ' ' -f 3`

#
# Get the release notes.
#
NOTE_JP=`cat ../ChangeLog | awk '/BEGIN-LATEST-JP/,/END-LATEST-JP/' | tail -n +2 | $HEAD -n -1`

#
# Do an interactive confirmation.
#
echo ""
echo "Are you sure you want to release version $VERSION?"
echo ""
echo "[Japanese Note]"
echo "$NOTE_JP"
echo ""
echo "(press enter to proceed)"
read str

#
# Build "game.exe".
#
echo ""
echo "Building game.exe"
say "Windows用のエンジンをビルドしています" &
cd engine-windows
make clean
make -j8
cd ..

#
# Build the Wasm files.
#
echo ""
echo "Building Wasm files."
say "Web用のエンジンをビルドしています" &
cd engine-wasm
make clean
make
cd ..

#
# Build the macOS app and source tree.
#
echo ""
echo "Building macOS source tree."
cd engine-macos
say "Mac用のエンジンをビルドしています" &
make
make src > /dev/null
cd ..

#
# Build the iOS source tree.
#
echo ""
echo "Building iOS source tree."
cd engine-ios
make src > /dev/null
cd ..

#
# Build the Android source tree.
#
echo ""
echo "Building Android source tree."
cd engine-android
make debug > /dev/null
make src > /dev/null
cd ..

#
# Build the Unity source tree.
#
echo ""
echo "Building Unity source tree."
say "Unity用のソースコードをビルドしています" &
cd engine-unity
make clean
make
cd ..

#
# Build "polaris-engine.exe".
#
echo ""
echo "Building polaris-engine.exe"
say "Windows用の開発ツールをビルドしています" &
cd pro-windows
make clean
make -j8 VERSION="$VERSION"
cd ..

#
# Build "web-test.exe"
#
if [ ! -e ../tools/web-test/web-test.exe ]; then
    echo "";
    echo "Building web-test.exe";
    say "Windows用のWebテストツールをビルドしています";
    cd ../tools/web-test;
    make;
    cd ../../build;
fi

#
# Make an installer for Windows.
#
echo ""
echo "Creating an installer for Windows."
say "Windows用のインストーラをビルドしています" &

# prepare
cp ../tools/library/orbis.txt ../games/japanese-light/txt/library/
cp ../tools/library/orbis.txt ../games/japanese-dark/txt/library/
cp ../tools/library/orbis.txt ../games/japanese-tategaki/txt/library/

# /
cp -v pro-windows/polaris-engine.exe installer-windows/polaris-engine.exe

# /games
rm -rf installer-windows/games
find ../games -name '.DS_Store' | xargs rm
mkdir installer-windows/games
cp -R ../games/japanese-light installer-windows/games/
cp -R ../games/japanese-dark installer-windows/games/
cp -R ../games/japanese-novel installer-windows/games/
cp -R ../games/japanese-tategaki installer-windows/games/
cp -R ../games/english installer-windows/games/
cp -R ../games/english-novel installer-windows/games/

# /tools
rm -rf installer-windows/tools
mkdir -p installer-windows/tools
cp engine-windows/game.exe installer-windows/tools/
cp engine-macos/game.dmg installer-windows/tools/
cp -R engine-macos/macos-src installer-windows/tools/macos-src
cp -R engine-ios/ios-src installer-windows/tools/ios-src
cp -R engine-android/android-src installer-windows/tools/android-src
cp -R engine-unity/unity-src installer-windows/tools/
mkdir -p installer-windows/tools/lib-src
cp libsrc/brotli-1.1.0.tar.gz installer-windows/tools/lib-src/
cp libsrc/freetype-2.13.2.tar.gz installer-windows/tools/lib-src/
cp libsrc/libogg-1.3.3.tar.gz installer-windows/tools/lib-src/
cp libsrc/libpng-1.6.43.tar.gz installer-windows/tools/lib-src/
cp libsrc/libwebp-1.3.2.tar.gz installer-windows/tools/lib-src/
cp libsrc/bzip2-1.0.6.tar.gz installer-windows/tools/lib-src/
cp libsrc/jpegsrc.v9e.tar.gz installer-windows/tools/lib-src/
cp libsrc/libvorbis-1.3.6.tar.gz installer-windows/tools/lib-src/
cp libsrc/zlib-1.2.11.tar.gz installer-windows/tools/lib-src/
mkdir -p installer-windows/tools/web
cp engine-wasm/html/index.html installer-windows/tools/web/index.html
cp engine-wasm/html/index.js installer-windows/tools/web/index.js
cp engine-wasm/html/index.wasm installer-windows/tools/web/index.wasm
cp ../tools/web-test/web-test.exe installer-windows/tools/web-test.exe

# Make an installer
cd installer-windows
make
cd ..

#
# Build "polaris-engine.app".
#
echo ""
echo "Building polaris-engine.app (polaris-engine.dmg)"
say "Mac用の開発ツールをビルドしています" &
cd pro-macos
rm -f polaris-engine.dmg
make
cd ..

rm ../games/japanese-light/txt/library/orbis.txt
rm ../games/japanese-dark/txt/library/orbis.txt
rm ../games/japanese-tategaki/txt/library/orbis.txt

#
# Make a release on GitHub.
#
echo ""
echo "Making a release on GitHub."
say "GitHubでリリースを作成しますか？"
read str
say "GitHubでリリースを作成中です"
git push github master
git tag -a "v$VERSION" -m "release"
git push github "v$VERSION"
mv installer-windows/polaris-engine-installer.exe polaris-engine-installer-windows.exe
mv pro-macos/polaris-engine.dmg polaris-engine-installer-macos.dmg
yes "" | gh release create "v$VERSION" --title "v$VERSION" --notes "$NOTE_EN" polaris-engine-installer-windows.exe polaris-engine-macos.dmg
rm polaris-engine-installer-windows.exe polaris-engine-macos.dmg

#
# Finish.
#
echo ""
echo "Finished. $VERSION was released!"
say "リリースが完了しました"
