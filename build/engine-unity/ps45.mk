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
	rm -rf *.o libroot-ps45 ../Assets/libxengine.so
	./build-libs.sh \
		"ps45" \
		"$CC" \
		"$AR"
	"$CC" \
		-shared \
		-o ../Assets/libxengine.so \
		-O2 \
		-fPIC \
		-DXENGINE_TARGET_UNITY \
		-DNO_CDECL \
		-I./libroot-ps45/include \
		-I./libroot-ps45/include/png \
		-I./libroot-ps45/include/freetype \
		$(SRCS) \
		libroot-ps45/lib/libwebp.a \
		libroot-ps45/lib/libfreetype.a \
		libroot-ps45/lib/libbrotlidec.a \
		libroot-ps45/lib/libbrotlicommon.a \
		libroot-ps45/lib/libpng.a \
		libroot-ps45/lib/libjpeg.a \
		libroot-ps45/lib/libvorbis.a \
		libroot-ps45/lib/libogg.a \
		libroot-ps45/lib/libbz2.a \
		libroot-ps45/lib/libz.a
