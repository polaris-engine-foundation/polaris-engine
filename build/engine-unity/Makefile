#
# Makefile for the Unity export template
#  - Note that the author does not have any copy of commercial SDK
#  - This Makefile was written with his imagination
#

include ../common.mk

OBJS = $(SRCS_UNITY:../../src/%.c=%.o)

#
# Make a source tree
#
all: libpolarisengine-win64.dll libpolarisengine-macos.dylib
	@rm -rf unity-src
	@mkdir unity-src
	@cp -v -R ProjectSettings unity-src/
	@cp -v -R Packages unity-src/
	@cp -v -R Library unity-src/
	@mkdir unity-src/Assets
	@cp -v Assets/MainScene.unity unity-src/Assets/
	@cp -v Assets/PolarisEngineScript.cs unity-src/Assets/
	@cp -v Assets/PolarisEngineAudio.cs unity-src/Assets/
	@mkdir unity-src/Assets/Resources
	@mkdir unity-src/Assets/Resources/StreamingAssets
	@cp -v Assets/Resources/NormalShader.shader unity-src/Assets/Resources/
	@cp libpolarisengine-win64.dll unity-src/
	@cp libpolarisengine-macos.dylib unity-src/
	@cp README.txt unity-src/
	@mkdir unity-src/dll-src
	@cp build-libs.sh unity-src/dll-src/
	@cp console.mk unity-src/Makefile
	@cp $(SRCS_UNITY) unity-src/dll-src/
	@cp ps45.mk unity-src/dll-src/
	@cp xbox.mk unity-src/dll-src/
	@cp switch.mk unity-src/dll-src/
	@cp -R ../libsrc unity-src/

#
# Windows 64bit DLL for Test Run
#
libpolarisengine-win64.dll: $(SRCS_UNITY)
	rm -rf *.o libroot-x86_64 libpolarisengine-win64.dll
	../engine-windows-64/llvm/bin/x86_64-w64-mingw32-gcc \
		-shared \
		-fPIC \
		-o libpolarisengine-win64.dll \
		-O2 \
		-DPOLARIS_ENGINE_TARGET_UNITY \
		-DNO_CDECL \
		-I../engine-windows-64/libroot/include \
		-I../engine-windows-64/libroot/include/png \
		-I../engine-windows-64/libroot/include/freetype2 \
		$(SRCS_UNITY) \
		../engine-windows-64/libroot/lib/libfreetype.a \
		../engine-windows-64/libroot/lib/libpng16.a \
		../engine-windows-64/libroot/lib/libjpeg.a \
		../engine-windows-64/libroot/lib/libwebp.a \
		../engine-windows-64/libroot/lib/libz.a \
		../engine-windows-64/libroot/lib/libbz2.a \
		../engine-windows-64/libroot/lib/libbrotlidec.a \
		../engine-windows-64/libroot/lib/libbrotlicommon.a \
		../engine-windows-64/libroot/lib/libvorbisfile.a \
		../engine-windows-64/libroot/lib/libvorbis.a \
		../engine-windows-64/libroot/lib/libogg.a

#
# macOS DLL for Test Run
#
libpolarisengine-macos.dylib: ../engine-macos/libroot $(SRCS_UNITY)
	/usr/bin/clang \
		-O2 \
		-arch arm64 \
		-arch x86_64 \
		-dynamiclib \
		-undefined dynamic_lookup \
		-o libpolarisengine-macos.dylib \
		-DPOLARIS_ENGINE_TARGET_UNITY \
		-I../engine-macos/libroot/include \
		-I../engine-macos/libroot/include/freetype2 \
		$(SRCS_UNITY) \
		../engine-macos/libroot/lib/libwebp.a \
		../engine-macos/libroot/lib/libwebpdemux.a \
		../engine-macos/libroot/lib/libfreetype.a \
		../engine-macos/libroot/lib/libbrotlidec.a \
		../engine-macos/libroot/lib/libbrotlicommon.a \
		../engine-macos/libroot/lib/libsharpyuv.a \
		../engine-macos/libroot/lib/libpng.a \
		../engine-macos/libroot/lib/libjpeg.a \
		../engine-macos/libroot/lib/libvorbis.a \
		../engine-macos/libroot/lib/libogg.a \
		../engine-macos/libroot/lib/libbz2.a \
		../engine-macos/libroot/lib/libz.a
	rm -rf libpolarisengine-macos.dylib.dSYM

#
# Cleanup
#
clean:
	@rm -rf libpolarisengine-win64.dll libpolarisengine-macos.dylib libroot* *.o
