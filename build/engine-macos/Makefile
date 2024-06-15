include ../common.mk

FILES_APPLE = \
	engine-macos/main.m \
	engine-macos/AppDelegate.h \
	engine-macos/AppDelegate.m \
	engine-macos/ViewController.h \
	engine-macos/ViewController.m \
	engine-macos/GameView.h \
	engine-macos/GameView.m \
	../../src/apple/aunit.h \
	../../src/apple/aunit.c \
	../../src/apple/GameRenderer.h \
	../../src/apple/GameShaderTypes.h \
	../../src/apple/GameViewControllerProtocol.h \
	../../src/apple/GameRenderer.m \
	../../src/apple/GameShaders.metal

#
# For prebuilt binary
#
game.dmg: libroot $(SRCS_MAIN) $(HDRS_MAIN) $(FILES_APPLE)
	@echo "Building Game.app"
	@rm -rf build
	@echo 'building...'
	@xcodebuild \
		-quiet \
		-scheme engine-macos \
		-project engine-macos.xcodeproj \
		-configuration Release \
		-archivePath `pwd`/build/Release/engine-macos.xcarchive \
		-allowProvisioningUpdates \
		DEVELOPMENT_TEAM="DT727R68TR" \
		CODE_SIGN_STYLE="manual" \
		CODE_SIGN_IDENTITY="Developer ID Application" \
		CODE_SIGN_INJECT_BASE_ENTITLEMENTS="NO" \
		PROVISIONING_PROFILE_SPECIFIER="05189468-cfae-4be4-87d2-ee463555d66b" \
		archive
	@echo 'archiving...'
	@xcodebuild \
		-quiet \
		-exportArchive \
		-archivePath `pwd`/build/Release/engine-macos.xcarchive \
		-allowProvisioningUpdates \
		DEVELOPMENT_TEAM="DT727R68TR" \
		CODE_SIGN_STYLE="manual" \
		CODE_SIGN_IDENTITY="Developer ID Application" \
		CODE_SIGN_INJECT_BASE_ENTITLEMENTS="NO" \
		PROVISIONING_PROFILE_SPECIFIER="05189468-cfae-4be4-87d2-ee463555d66b" \
		-exportOptionsPlist export-options.plist
	@echo 'exporting...'
	@until \
		xcodebuild \
			-quiet \
			-exportNotarizedApp \
			-archivePath `pwd`/build/Release/engine-macos.xcarchive \
			-exportPath `pwd`/build/Release > /dev/null 2>&1; \
	do \
		echo "Waiting 10 seconds for notarization..."; \
		sleep 10; \
	done
	@echo "Successfully notarized."
	@rm -f game.dmg
	@mkdir tmp
	@cp -Rv build/Release/Game.app tmp/
	@hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname Game game.dmg
	@codesign --sign 'Developer ID Application' game.dmg
	@rm -rf tmp

src: libroot
	@rm -rf macos-src
	@mkdir macos-src
	@cp -R engine-macos engine-macos.xcodeproj macos-src/
	@sed -e 's|../../src/apple/|engine-macos/apple/|g' < macos-src/engine-macos.xcodeproj/project.pbxproj > tmp && mv tmp macos-src/engine-macos.xcodeproj/project.pbxproj
	@sed -e 's|../../src/|engine-macos/|g' < macos-src/engine-macos.xcodeproj/project.pbxproj > tmp && mv tmp macos-src/engine-macos.xcodeproj/project.pbxproj
	@mkdir -p macos-src/engine-macos/apple
	@cp ../../src/apple/GameRenderer.h macos-src/engine-macos/apple/
	@cp ../../src/apple/GameRenderer.m macos-src/engine-macos/apple/
	@cp ../../src/apple/GameShaders.metal macos-src/engine-macos/apple/
	@cp ../../src/apple/GameShaderTypes.h macos-src/engine-macos/apple/
	@cp ../../src/apple/GameViewControllerProtocol.h macos-src/engine-macos/apple/
	@cp ../../src/apple/aunit.c macos-src/engine-macos/apple/
	@cp ../../src/apple/aunit.h macos-src/engine-macos/apple/
	@cp ../../src/anime.c macos-src/engine-macos/
	@cp ../../src/anime.h macos-src/engine-macos/
	@cp ../../src/ciel.c macos-src/engine-macos/
	@cp ../../src/ciel.h macos-src/engine-macos/
	@cp ../../src/conf.c macos-src/engine-macos/
	@cp ../../src/conf.h macos-src/engine-macos/
	@cp ../../src/event.c macos-src/engine-macos/
	@cp ../../src/event.h macos-src/engine-macos/
	@cp ../../src/file.h macos-src/engine-macos/
	@cp ../../src/file.c macos-src/engine-macos/
	@cp ../../src/glyph.c macos-src/engine-macos/
	@cp ../../src/glyph.h macos-src/engine-macos/
	@cp ../../src/gui.c macos-src/engine-macos/
	@cp ../../src/gui.h macos-src/engine-macos/
	@cp ../../src/history.c macos-src/engine-macos/
	@cp ../../src/history.h macos-src/engine-macos/
	@cp ../../src/image.c macos-src/engine-macos/
	@cp ../../src/image.h macos-src/engine-macos/
	@cp ../../src/key.h macos-src/engine-macos/
	@cp ../../src/log.c macos-src/engine-macos/
	@cp ../../src/log.h macos-src/engine-macos/
	@cp ../../src/main.c macos-src/engine-macos/
	@cp ../../src/main.h macos-src/engine-macos/
	@cp ../../src/mixer.c macos-src/engine-macos/
	@cp ../../src/mixer.h macos-src/engine-macos/
	@cp ../../src/motion.h macos-src/engine-macos/
	@cp ../../src/hal.h macos-src/engine-macos/
	@cp ../../src/readimage.c macos-src/engine-macos/
	@cp ../../src/readpng.c macos-src/engine-macos/
	@cp ../../src/readjpeg.c macos-src/engine-macos/
	@cp ../../src/readwebp.c macos-src/engine-macos/
	@cp ../../src/save.c macos-src/engine-macos/
	@cp ../../src/save.h macos-src/engine-macos/
	@cp ../../src/scbuf.c macos-src/engine-macos/
	@cp ../../src/scbuf.h macos-src/engine-macos/
	@cp ../../src/script.c macos-src/engine-macos/
	@cp ../../src/script.h macos-src/engine-macos/
	@cp ../../src/seen.c macos-src/engine-macos/
	@cp ../../src/seen.h macos-src/engine-macos/
	@cp ../../src/stage.c macos-src/engine-macos/
	@cp ../../src/stage.h macos-src/engine-macos/
	@cp ../../src/uimsg.h macos-src/engine-macos/
	@cp ../../src/uimsg.c macos-src/engine-macos/
	@cp ../../src/polarisengine.h macos-src/engine-macos/
	@cp ../../src/types.h macos-src/engine-macos/
	@cp ../../src/vars.c macos-src/engine-macos/
	@cp ../../src/vars.h macos-src/engine-macos/
	@cp ../../src/wave.c macos-src/engine-macos/
	@cp ../../src/wave.h macos-src/engine-macos/
	@cp ../../src/wms_core.c macos-src/engine-macos/
	@cp ../../src/wms_core.h macos-src/engine-macos/
	@cp ../../src/wms.h macos-src/engine-macos/
	@cp ../../src/wms_impl.c macos-src/engine-macos/
	@cp ../../src/wms_lexer.yy.c macos-src/engine-macos/
	@cp ../../src/wms_parser.tab.c macos-src/engine-macos/
	@cp ../../src/wms_parser.tab.h macos-src/engine-macos/
	@cp ../../src/cmd_anime.c macos-src/engine-macos/
	@cp ../../src/cmd_bg.c macos-src/engine-macos/
	@cp ../../src/cmd_bgm.c macos-src/engine-macos/
	@cp ../../src/cmd_cha.c macos-src/engine-macos/
	@cp ../../src/cmd_chapter.c macos-src/engine-macos/
	@cp ../../src/cmd_ch.c macos-src/engine-macos/
	@cp ../../src/cmd_chs.c macos-src/engine-macos/
	@cp ../../src/cmd_click.c macos-src/engine-macos/
	@cp ../../src/cmd_gosub.c macos-src/engine-macos/
	@cp ../../src/cmd_goto.c macos-src/engine-macos/
	@cp ../../src/cmd_gui.c macos-src/engine-macos/
	@cp ../../src/cmd_if.c macos-src/engine-macos/
	@cp ../../src/cmd_layer.c macos-src/engine-macos/
	@cp ../../src/cmd_load.c macos-src/engine-macos/
	@cp ../../src/cmd_message.c macos-src/engine-macos/
	@cp ../../src/cmd_pencil.c macos-src/engine-macos/
	@cp ../../src/cmd_return.c macos-src/engine-macos/
	@cp ../../src/cmd_se.c macos-src/engine-macos/
	@cp ../../src/cmd_set.c macos-src/engine-macos/
	@cp ../../src/cmd_setconfig.c macos-src/engine-macos/
	@cp ../../src/cmd_setsave.c macos-src/engine-macos/
	@cp ../../src/cmd_shake.c macos-src/engine-macos/
	@cp ../../src/cmd_skip.c macos-src/engine-macos/
	@cp ../../src/cmd_switch.c macos-src/engine-macos/
	@cp ../../src/cmd_video.c macos-src/engine-macos/
	@cp ../../src/cmd_vol.c macos-src/engine-macos/
	@cp ../../src/cmd_wait.c macos-src/engine-macos/
	@cp ../../src/cmd_wms.c macos-src/engine-macos/
	@mkdir macos-src/Resources/
	@cp -Ra libroot macos-src/

libroot:
	./build-libs.sh

clean:
	@rm -rf build game-mac.dmg libroot-mac.tar.gz libroot
