CC=cc
AR=ar

SRCS = \
	halwrap.c \
	anime.c \
	conf.c \
	ciel.c \
	event.c \
	glyph.c \
	gui.c \
	history.c \
	image.c \
	log.c \
	main.c \
	mixer.c \
	readimage.c \
	readpng.c \
	readjpeg.c \
	readwebp.c \
	save.c \
	scbuf.c \
	script.c \
	seen.c \
	stage.c \
	uimsg.c \
	vars.c \
	wave.c \
	wms_core.c \
	wms_lexer.yy.c \
	wms_parser.tab.c \
	wms_impl.c \
	cmd_anime.c \
	cmd_bg.c \
	cmd_bgm.c \
	cmd_ch.c \
	cmd_cha.c \
	cmd_chapter.c \
	cmd_chs.c \
	cmd_click.c \
	cmd_gosub.c \
	cmd_goto.c \
	cmd_gui.c \
	cmd_if.c \
	cmd_layer.c \
	cmd_load.c \
	cmd_message.c \
	cmd_pencil.c \
	cmd_return.c \
	cmd_se.c \
	cmd_set.c \
	cmd_setconfig.c \
	cmd_setsave.c \
	cmd_shake.c \
	cmd_skip.c \
	cmd_xbox.c \
	cmd_video.c \
	cmd_vol.c \
	cmd_wait.c \
	cmd_wms.c

all:
	rm -rf *.o libroot-xbox ../Assets/libpolarisengine.dll
	./build-libs.sh \
		"xbox" \
		"$CC" \
		"$AR"
	"$CC" \
		-shared \
		-o ../Assets/libpolarisengine.dll \
		-O2 \
		-fPIC \
		-DUSE_UNITY \
		-DUSE_DLL \
		-DNO_CDECL \
		-I./libroot-xbox/include \
		-I./libroot-xbox/include/png \
		-I./libroot-xbox/include/freetype \
		$(SRCS) \
		libroot-xbox/lib/libwebp.a \
		libroot-xbox/lib/libfreetype.a \
		libroot-xbox/lib/libbrotlidec.a \
		libroot-xbox/lib/libbrotlicommon.a \
		libroot-xbox/lib/libpng.a \
		libroot-xbox/lib/libjpeg.a \
		libroot-xbox/lib/libvorbis.a \
		libroot-xbox/lib/libogg.a \
		libroot-xbox/lib/libbz2.a \
		libroot-xbox/lib/libz.a
