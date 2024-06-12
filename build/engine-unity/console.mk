help:
	@echo 'Usage:'
	@echo '  make ps45   ... make a DLL for PlayStation 4/5'
	@echo '  make xbox   ... make a DLL for Xbox Series X|S'
	@echo '  make switch ... make a DLL for Switch'
	@echo '  make win64  ... switch to a DLL for test run on Windows 64bit'
	@echo '  make macos  ... switch to a DLL for test run on macOS'

ps45:
	cd dll-src && make -f ps45.mk && cd ..

xbox:
	cd dll-src && make -f xbox.mk && cd ..

switch:
	cd dll-src && make -f switch.mk && cd ..
