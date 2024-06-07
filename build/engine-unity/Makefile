#
# Makefile for the Unity export template
#  - Note that the author does not have any copy of commercial SDK
#  - This Makefile was written with his imagination
#

include ../common.mk

OBJS = $(SRCS_UNITY:../../src/%.c=%.o)

#
# Show help
#
help:
	@echo 'Usage:'
	@echo '  make src-main   ... make a main source tree of Unity export'
	@echo '  make src-switch ... make a DLL source tree for Switch'
	@echo '  make src-ps45   ... make a DLL source tree for PS4/5'
	@echo '  make src-xboxxs ... make a DLL source tree for Xbox X|S'
	@echo '  make dll-win64  ... make a DLL for Windows x64'
	@echo '  make dll-macos  ... make a DLL for macOS arm64'

#
# Make a source tree
#
src-main:
	@rm -rf unity-src
	@mkdir unity-src
	@cp -v -R ProjectSettings unity-src/
	@cp -v -R Packages unity-src/
	@cp -v -R Library unity-src/
	@mkdir unity-src/Assets
	@cp -v Assets/MainScene.unity unity-src/Assets/
	@cp -v Assets/XEngineScript.cs unity-src/Assets/
	@cp -v Assets/XEngineAudio.cs unity-src/Assets/
	@mkdir unity-src/Assets/Resources
	@cp -v Assets/Resources/NormalShader.shader unity-src/Assets/Resources/

#
# Switch DLL Source Tree
#  - Note that the author does not have a copy of SDK for Xbox and this was done by his imagination
#
src-switch:
	rm -rf switch-src
	mkdir switch-src
	cp build-libs.sh switch-src/
	cp switch.mk switch-src/Makefile
	cp $(SRCS_UNITY) switch-src/

#
# PS4/5 DLL source tree
#  - Note that the author does not have a copy of SDK for Xbox and this was done by his imagination
#
src-ps45:
	rm -rf ps45-src
	mkdir ps45-src
	cp build-libs.sh ps45-src/
	cp ps45.mk ps45-src/Makefile
	cp $(SRCS_UNITY) ps45-src/

#
# Xbox Series X|S DLL source tree
#  - Note that the author does not have a copy of SDK for Xbox and this was done by his imagination
#
src-xbox:
	rm -rf xbox-src
	mkdir xbox-src
	cp build-libs.sh xbox-src/
	cp xbox.mk xbox-src/Makefile
	cp $(SRCS_UNITY) xbox-src/

#
# Windows 64bit DLL
#
dll-win64: $(SRCS_UNITY)
	rm -rf *.o libroot-x86_64 libxengine-win64.dll
	../engine-windows-64/llvm/bin/x86_64-w64-mingw32-gcc \
		-shared \
		-fPIC \
		-o libxengine-win64.dll \
		-O2 \
		-DXENGINE_TARGET_UNITY \
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
# macOS DLL
#
dll-mac: ../engine-macos/libroot $(SRCS_UNITY)
	/usr/bin/clang \
		-O2 \
		-arch arm64 \
		-arch x86_64 \
		-dynamiclib \
		-undefined dynamic_lookup \
		-o libxengine-macos.dylib \
		-DXENGINE_TARGET_UNITY \
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
	rm -rf libxengine-macos.dylib.dSYM

#
# Cleanup
#
clean:
	@rm -rf libxengine-switch.so libxengine-ps45.so libxengine-xbox.dll libxengine-win64.dll libxengine-macos.dylib libroot* *.o
