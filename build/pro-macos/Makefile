include ../common.mk

FILES_MACOS = \
	pro-macos/main.m \
	pro-macos/AppDelegate.h \
	pro-macos/AppDelegate.m \
	pro-macos/ViewController.h \
	pro-macos/ViewController.m \
	pro-macos/GameView.h \
	pro-macos/GameView.m \
	../../src/apple/aunit.h \
	../../src/apple/aunit.c \
	../../src/apple/GameRenderer.h \
	../../src/apple/GameShaderTypes.h \
	../../src/apple/GameViewControllerProtocol.h \
	../../src/apple/GameRenderer.m \
	../../src/apple/GameShaders.metal

main: polaris-engine.dmg

polaris-engine.dmg: libroot $(SRCS_MAIN) $(HDRS_MAIN) $(FILES_MACOS)
	@echo "Building polaris-engine.app"
	@rm -rf build
	@xcodebuild \
		-quiet \
		-scheme pro-macos \
		-project pro-macos.xcodeproj \
		-configuration Release \
		-archivePath `pwd`/build/Release/pro-macos.xcarchive \
		-allowProvisioningUpdates \
		DEVELOPMENT_TEAM="DT727R68TR" \
		CODE_SIGN_STYLE="manual" \
		CODE_SIGN_IDENTITY="Developer ID Application" \
		CODE_SIGN_INJECT_BASE_ENTITLEMENTS="NO" \
		PROVISIONING_PROFILE_SPECIFIER="465f3957-3f75-418d-ab03-cb77360ee9f2" \
		archive
	@xcodebuild \
		-quiet \
		-exportArchive \
		-archivePath `pwd`/build/Release/pro-macos.xcarchive \
		-allowProvisioningUpdates \
		-exportOptionsPlist export-options.plist \
		-exportPath `pwd`/build/Release/pro-macos.xcarchive
	@rm -rf "build/Release/polaris-engine.app"
	@until \
		xcodebuild \
			-quiet \
			-exportNotarizedApp \
			-archivePath `pwd`/build/Release/pro-macos.xcarchive \
			-allowProvisioningUpdates \
			-exportPath `pwd`/build/Release > /dev/null 2>&1; \
	do \
		echo "Waiting 10 seconds for notarization..."; \
		sleep 10; \
	done
	@echo "Successfully notarized."
	@rm -rf tmp *.dmg
	@mkdir tmp
	@cp -Ra "build/Release/polaris-engine.app" "tmp/polaris-engine.app"
	@echo "Creating a dmg file."
	@hdiutil create -fs APFS -format UDBZ -srcfolder tmp -volname "polaris-engine" "polaris-engine.dmg"
	@echo "Signing the dmg file."
	@codesign --sign 'Developer ID Application' polaris-engine.dmg
	@rm -rf tmp

libroot:
	./build-libs.sh

clean:
	@rm -rf build polaris-engine.dmg libroot-mac.tar.gz libroot
