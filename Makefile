DESTDIR=/usr/local

build: polaris-engine-runtime polaris-engine

polaris-engine-runtime:
	@# For macOS.
	@if [ ! -z "`uname | grep Darwin`" ]; then \
		echo 'On macOS, run make release instead.'; \
		exit 1; \
	fi;
	@# For Linux:
	@if [ ! -z "`uname | grep Linux`" ]; then \
		cd build/engine-x11 && \
		make -f Makefile.linux -j8 && \
		make -f Makefile.linux install && \
		cd ../..; \
	fi
	@# For FreeBSD:
	@if [ ! -z "`uname | grep FreeBSD`" ]; then \
		cd build/engine-x11 && \
		gmake -f Makefile.freebsd -j8 && \
		gmake -f Makefile.freebsd install && \
		cd ../..; \
	fi
	@# For NetBSD:
	@if [ ! -z "`uname | grep NetBSD`" ]; then \
		cd build/engine-x11 && \
		gmake -f Makefile.netbsd -j8 && \
		gmake -f Makefile.netbsd install && \
		cd ../..; \
	fi
	@# For OpenBSD:
	@if [ ! -z "`uname | grep OpenBSD`" ]; then \
		cd build/engine-x11 && \
		gmake -f Makefile.openbsd -j8 && \
		gmake -f Makefile.openbsd install && \
		cd ../..; \
	fi

polaris-engine:
	@if [ ! -z "`uname | grep Darwin`" ]; then \
		echo 'On macOS, run make release instead.'; \
		exit 1; \
	fi;
	@cd build/pro-qt && \
		./make-deps.sh && \
		rm -rf build && \
		mkdir build && \
		cd build && \
		cmake .. $(CMAKE_FLAGS) && \
		make && \
		cp polaris-engine ../../../ && \
		cd ../../..

install: build
	@install -v -d $(DESTDIR)/bin
	@install -v polaris-engine-runtime $(DESTDIR)/bin/polaris-engine-runtime
	@install -v polaris-engine $(DESTDIR)/bin/polaris-engine

	@install -v -d $(DESTDIR)/share
	@install -v -d $(DESTDIR)/share/polaris-engine

	@install -v -d $(DESTDIR)/share/polaris-engine/export-linux
	@install -v polaris-engine-runtime $(DESTDIR)/share/polaris-engine/export-linux/polaris-engine-runtime

	@install -v -d $(DESTDIR)/share/polaris-engine/export-web
	@if [ -e build/engine-wasm/html/index.html ]; then install -v build/engine-wasm/html/index.html $(DESTDIR)/share/polaris-engine/export-web; fi
	@if [ -e build/engine-wasm/html/index.js ]; then install -v build/engine-wasm/html/index.js $(DESTDIR)/share/polaris-engine/export-web; fi
	@if [ -e build/engine-wasm/html/index.wasm ]; then install -v build/engine-wasm/html/index.wasm $(DESTDIR)/share/polaris-engine/export-web; fi

	@install -v -d $(DESTDIR)/share/polaris-engine/japanese-light
	@cd games/japanese-light && find . -type d -exec install -v -d "$(DESTDIR)/share/polaris-engine/japanese-light/{}" ';' && cd ../..
	@cd games/japanese-light && find . -type f -exec install -v "{}" "$(DESTDIR)/share/polaris-engine/japanese-light/{}" ';' && cd ../..

	@install -v -d $(DESTDIR)/share/polaris-engine/japanese-dark
	@cd games/japanese-dark && find . -type d -exec install -v -d "$(DESTDIR)/share/polaris-engine/japanese-dark/{}" ';' && cd ../..
	@cd games/japanese-dark && find . -type f -exec install -v "{}" "$(DESTDIR)/share/polaris-engine/japanese-dark/{}" ';' && cd ../..

	@install -v -d $(DESTDIR)/share/polaris-engine/japanese-novel
	@cd games/japanese-novel && find . -type d -exec install -v -d "$(DESTDIR)/share/polaris-engine/japanese-novel/{}" ';' && cd ../..
	@cd games/japanese-novel && find . -type f -exec install -v "$(DESTDIR)/share/polaris-engine/japanese-novel/{}" "{}" ';' && cd ../..

	@install -v -d $(DESTDIR)/share/polaris-engine/japanese-tategaki
	@cd games/japanese-tategaki && find . -type d -exec install -v -d "$(DESTDIR)/share/polaris-engine/japanese-tategaki/{}" ';' && cd ../..
	@cd games/japanese-tategaki && find . -type f -exec install -v "{}" "$(DESTDIR)/share/polaris-engine/japanese-tategaki/{}" ';' && cd ../..

	@install -v -d $(DESTDIR)/share/polaris-engine/english
	@cd games/english && find . -type d -exec install -v -d "$(DESTDIR)/share/polaris-engine/english/{}" ';' && cd ../..
	@cd games/english && find . -type f -exec install -v "{}" "$(DESTDIR)/share/polaris-engine/english/{}" ';' && cd ../..

	@install -v -d $(DESTDIR)/share/polaris-engine/english-novel
	@cd games/english-novel && find . -type d -exec install -v -d "$(DESTDIR)/share/polaris-engine/english-novel/{}" ';' && cd ../..
	@cd games/english-novel && find . -type f -exec install -v "{}" "$(DESTDIR)/share/polaris-engine/english-novel/{}" ';' && cd ../..

clean:
	rm -f polaris-engine-runtime polaris-engine

##
## dev internal
##

release:
	@cd build && ./make-release.sh && cd ..
	mv build/installer-windows/polaris-engine-installer.exe polaris-engine-installer-windows.exe
	mv build/pro-macos/polaris-engine.dmg polaris-engine-installer-macos.dmg

setup:
	@if [ ! -z "`uname -a | grep Darwin`" ]; then \
		if [ -z  "`which brew`" ]; then \
			echo 'Installing Homebrew...'; \
			/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"; \
		fi; \
		echo 'Installing build tools...'; \
		brew install mingw-w64 gsed cmake coreutils emscripten makensis create-dmg; \
		echo "Building libraries..."; \
		cd build/engine-windows && ./build-libs.sh && cd ../..; \
		cp -Ra build/engine-windows/libroot build/pro-windows/; \
		cd build/engine-windows-64 && ./build-libs.sh && cd ../..; \
		cd build/engine-windows-arm64 && ./build-libs.sh && cd ../..; \
		cd build/engine-macos && ./build-libs.sh && cd ../..; \
		cd build/engine-ios && ./build-libs-device.sh && ./build-libs-sim.sh && cd ../..; \
		cp -Ra build/engine-macos/libroot build/pro-macos/; \
		cp -Ra build/engine-ios/libroot-device build/pro-ios/; \
		cp -Ra build/engine-ios/libroot-sim build/pro-ios/; \
	fi
	@if [ ! -z "`uname -a | grep Debian`" ]; then \
		echo 'Installing dependencies...'; \
		sudo apt-get update; \
		sudo apt-get install build-essential cmake libasound2-dev libx11-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libxpm-dev mesa-common-dev zlib1g-dev libpng-dev libjpeg-dev libwebp-dev libbz2-dev libogg-dev libvorbis-dev libfreetype-dev cmake qt6-base-dev qt6-multimedia-dev libqt6core6 libqt6gui6 libqt6widgets6 libqt6opengl6-dev libqt6openglwidgets6 libqt6multimedia6 libqt6multimediawidgets6 mingw-w64 autoconf automake; \
		echo "Building libraries..."; \
		cd build/engine-windows && ./build-libs.sh && cd ../..; \
		cp -Ra build/engine-windows/libroot build/pro-windows/; \
		cd build/engine-windows-64 && ./build-libs.sh && cd ../..; \
	fi
	@if [ ! -z "`uname -a | grep Ubuntu`" ]; then \
		echo 'Installing dependencies...'; \
		sudo apt-get update; \
		sudo apt-get install build-essential cmake libasound2-dev libx11-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libxpm-dev mesa-common-dev zlib1g-dev libpng-dev libjpeg-dev libwebp-dev libbz2-dev libogg-dev libvorbis-dev libfreetype-dev cmake qt6-base-dev qt6-multimedia-dev libqt6core6 libqt6gui6 libqt6widgets6 libqt6opengl6-dev libqt6openglwidgets6 libqt6multimedia6 libqt6multimediawidgets6 mingw-w64 autoconf automake; \
		cd build/engine-windows && ./build-libs.sh && cd ../..; \
		cp -Ra build/engine-windows/libroot build/pro-windows/; \
		cd build/engine-windows-64 && ./build-libs.sh && cd ../..; \
	fi
	@if [ ! -z "`uname -a | grep WSL2`" ]; then \
		echo 'Installing dependencies for WSL2...'; \
		sudo apt-get update; \
		sudo apt-get install build-essential cmake libasound2-dev libx11-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libxpm-dev mesa-common-dev zlib1g-dev libpng-dev libjpeg-dev libwebp-dev libbz2-dev libogg-dev libvorbis-dev libfreetype-dev cmake qt6-base-dev qt6-multimedia-dev libqt6core6 libqt6gui6 libqt6widgets6 libqt6opengl6-dev libqt6openglwidgets6 libqt6multimedia6 libqt6multimediawidgets6 mingw-w64 autoconf automake; \
		cd build/engine-windows && ./build-libs.sh && cd ../..; \
		cp -Ra build/engine-windows/libroot build/pro-windows/; \
		cd build/engine-windows-64 && ./build-libs.sh && cd ../..; \
	fi
	@if [ ! -z "`uname -a | grep Microsoft`" ]; then \
		echo 'Installing dependencies for WSL1...'; \
		sudo apt-get update; \
		sudo apt-get install build-essential cmake libasound2-dev libx11-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libxpm-dev mesa-common-dev zlib1g-dev libpng-dev libjpeg-dev libwebp-dev libbz2-dev libogg-dev libvorbis-dev libfreetype-dev cmake qt6-base-dev qt6-multimedia-dev libqt6core6 libqt6gui6 libqt6widgets6 libqt6opengl6-dev libqt6openglwidgets6 libqt6multimedia6 libqt6multimediawidgets6 mingw-w64 autoconf automake; \
		echo "Disabling EXE file execution."; \
		cd build/engine-windows && ./build-libs.sh && cd ../..; \
		cp -Ra build/engine-windows/libroot build/pro-windows/; \
		cd build/engine-windows-64 && ./build-libs.sh && cd ../..; \
	fi
	@if [ ! -z "`uname -a | grep FreeBSD`" ]; then \
		echo 'Installing dependencies...'; \
		sudo pkg update; \
		sudo pkg install git gmake gsed alsa-lib alsa-plugins qt6 xorg git cmake mesa-devel freetype2; \
	fi

engine-windows:
	cd build/engine-windows && make && cd ../..

engine-windows-64:
	cd build/engine-windows-64 && make && cd ../..

engine-windows-arm64:
	cd build/engine-windows-arm64 && make && cd ../..

pro-windows:
	cd build/pro-windows && make && cd ../..

engine-macos:
	cd build/engine-macos && make && cd ../..

pro-macos:
	cd build/pro-macos && make && cd ../..

engine-wasm:
	cd build/engine-wasm  && make && cd ../..

pro-wasm:
	cd build/pro-wasm  && make && cd ../..

engine-linux: polaris-engine-runtime

pro-linux: polaris-engine
