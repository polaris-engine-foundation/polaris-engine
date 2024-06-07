CC="YOUR_SDK_CC"
AR="YOUR_SDK_AR"

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
	cmd_switch.c \
	cmd_video.c \
	cmd_vol.c \
	cmd_wait.c \
	cmd_wms.c

all:
	rm -rf *.o libroot-switch ../Assets/libxengine.nso
	./build-libs.sh \
		"switch" \
		"$CC" \
		"$AR"
	"$CC" \
		-shared \
		-o ../Assets/libxengine.nso \
		-O2 \
		-fPIC \
		-DXENGINE_TARGET_UNITY \
		-DNO_CDECL \
		-I./libroot-switch/include \
		-I./libroot-switch/include/png \
		-I./libroot-switch/include/freetype \
		$(SRCS) \
		libroot-switch/lib/libwebp.a \
		libroot-switch/lib/libfreetype.a \
		libroot-switch/lib/libbrotlidec.a \
		libroot-switch/lib/libbrotlicommon.a \
		libroot-switch/lib/libpng.a \
		libroot-switch/lib/libjpeg.a \
		libroot-switch/lib/libvorbis.a \
		libroot-switch/lib/libogg.a \
		libroot-switch/lib/libbz2.a \
		libroot-switch/lib/libz.a
