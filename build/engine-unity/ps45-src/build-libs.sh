#!/bin/sh

set -eu

MACHINE=$1
CC=$2
AR=$3

PREFIX=`pwd`/libroot-$MACHINE
rm -rf tmp $PREFIX
mkdir -p tmp $PREFIX
mkdir -p $PREFIX/include $PREFIX/lib

cd tmp

echo 'Building brotli...'
tar xzf ../lib-src/brotli-1.1.0.tar.gz
cd brotli-1.1.0
$CC -I./c/include -fPIC -c c/common/constants.c
$CC -I./c/include -fPIC -c c/common/dictionary.c
$CC -I./c/include -fPIC -c c/common/shared_dictionary.c
$CC -I./c/include -fPIC -c c/common/context.c
$CC -I./c/include -fPIC -c c/common/platform.c
$CC -I./c/include -fPIC -c c/common/transform.c
$CC -I./c/include -fPIC -c c/dec/bit_reader.c
$CC -I./c/include -fPIC -c c/dec/decode.c
$CC -I./c/include -fPIC -c c/dec/huffman.c
$CC -I./c/include -fPIC -c c/dec/state.c
$AR rcs $PREFIX/lib/libbrotlicommon.a constants.o dictionary.o shared_dictionary.o context.o platform.o transform.o
$AR rcs $PREFIX/lib/libbrotlidec.a bit_reader.o decode.o huffman.o state.o
cp -R c/include/brotli $PREFIX/include/
cd ..

echo 'Building bzip2...'
tar xzf ../lib-src/bzip2-1.0.6.tar.gz
cd bzip2-1.0.6
$CC -I./c/include -fPIC -c blocksort.c
$CC -I./c/include -fPIC -c compress.c
$CC -I./c/include -fPIC -c decompress.c
$CC -I./c/include -fPIC -c huffman.c
$CC -I./c/include -fPIC -c randtable.c
$CC -I./c/include -fPIC -Wno-pointer-sign -c unzcrash.c
$CC -I./c/include -fPIC -c bzlib.c
$CC -I./c/include -fPIC -c crctable.c
$AR rcs $PREFIX/lib/libbz2.a blocksort.o compress.o decompress.o huffman.o randtable.o unzcrash.o bzlib.o crctable.o
cp bzlib.h $PREFIX/include/
cd ..

echo 'Building libwebp...'
tar xzf ../lib-src/libwebp-1.3.2.tar.gz
cd libwebp-1.3.2
$CC -I. -I./src -fPIC -c src/dec/alpha_dec.c
$CC -I. -I./src -fPIC -c src/dec/buffer_dec.c
$CC -I. -I./src -fPIC -c src/dec/frame_dec.c
$CC -I. -I./src -fPIC -c src/dec/idec_dec.c
$CC -I. -I./src -fPIC -c src/dec/io_dec.c
$CC -I. -I./src -fPIC -c src/dec/quant_dec.c
$CC -I. -I./src -fPIC -c src/dec/tree_dec.c
$CC -I. -I./src -fPIC -c src/dec/vp8_dec.c
$CC -I. -I./src -fPIC -c src/dec/vp8l_dec.c
$CC -I. -I./src -fPIC -c src/dec/webp_dec.c
$CC -I. -I./src -fPIC -c src/demux/anim_decode.c
$CC -I. -I./src -fPIC -c src/demux/demux.c
$CC -I. -I./src -fPIC -c src/dsp/alpha_processing.c
$CC -I. -I./src -fPIC -c src/dsp/cost.c
$CC -I. -I./src -fPIC -c src/dsp/cost_mips32.c
$CC -I. -I./src -fPIC -c src/dsp/cpu.c
$CC -I. -I./src -fPIC -c src/dsp/dec.c
$CC -I. -I./src -fPIC -c src/dsp/dec_clip_tables.c
$CC -I. -I./src -fPIC -c src/dsp/enc.c
$CC -I. -I./src -fPIC -c src/dsp/filters.c
$CC -I. -I./src -fPIC -c src/dsp/lossless.c
$CC -I. -I./src -fPIC -c src/dsp/lossless_enc.c
$CC -I. -I./src -fPIC -c src/dsp/rescaler.c
$CC -I. -I./src -fPIC -c src/dsp/ssim.c
$CC -I. -I./src -fPIC -c src/dsp/upsampling.c
$CC -I. -I./src -fPIC -c src/dsp/yuv.c
$CC -I. -I./src -fPIC -c src/enc/alpha_enc.c
$CC -I. -I./src -fPIC -c src/enc/analysis_enc.c
$CC -I. -I./src -fPIC -c src/enc/backward_references_cost_enc.c
$CC -I. -I./src -fPIC -c src/enc/backward_references_enc.c
$CC -I. -I./src -fPIC -c src/enc/config_enc.c
$CC -I. -I./src -fPIC -c src/enc/cost_enc.c
$CC -I. -I./src -fPIC -c src/enc/filter_enc.c
$CC -I. -I./src -fPIC -c src/enc/frame_enc.c
$CC -I. -I./src -fPIC -c src/enc/histogram_enc.c
$CC -I. -I./src -fPIC -c src/enc/iterator_enc.c
$CC -I. -I./src -fPIC -c src/enc/near_lossless_enc.c
$CC -I. -I./src -fPIC -c src/enc/picture_csp_enc.c
$CC -I. -I./src -fPIC -c src/enc/picture_enc.c
$CC -I. -I./src -fPIC -c src/enc/picture_psnr_enc.c
$CC -I. -I./src -fPIC -c src/enc/picture_rescale_enc.c
$CC -I. -I./src -fPIC -c src/enc/picture_tools_enc.c
$CC -I. -I./src -fPIC -c src/enc/predictor_enc.c
$CC -I. -I./src -fPIC -c src/enc/quant_enc.c
$CC -I. -I./src -fPIC -c src/enc/syntax_enc.c
$CC -I. -I./src -fPIC -c src/enc/token_enc.c
$CC -I. -I./src -fPIC -c src/enc/tree_enc.c
$CC -I. -I./src -fPIC -c src/enc/vp8l_enc.c
$CC -I. -I./src -fPIC -c src/enc/webp_enc.c
$CC -I. -I./src -fPIC -c src/mux/anim_encode.c
$CC -I. -I./src -fPIC -c src/mux/muxedit.c
$CC -I. -I./src -fPIC -c src/mux/muxinternal.c
$CC -I. -I./src -fPIC -c src/mux/muxread.c
$CC -I. -I./src -fPIC -c src/utils/bit_reader_utils.c
$CC -I. -I./src -fPIC -c src/utils/bit_writer_utils.c
$CC -I. -I./src -fPIC -c src/utils/color_cache_utils.c
$CC -I. -I./src -fPIC -c src/utils/filters_utils.c
$CC -I. -I./src -fPIC -c src/utils/huffman_encode_utils.c
$CC -I. -I./src -fPIC -c src/utils/huffman_utils.c
$CC -I. -I./src -fPIC -c src/utils/quant_levels_dec_utils.c
$CC -I. -I./src -fPIC -c src/utils/quant_levels_utils.c
$CC -I. -I./src -fPIC -c src/utils/random_utils.c
$CC -I. -I./src -fPIC -c src/utils/rescaler_utils.c
$CC -I. -I./src -fPIC -c src/utils/thread_utils.c
$CC -I. -I./src -fPIC -c src/utils/utils.c
$AR rcs $PREFIX/lib/libwebp.a *.o
cp -R src/webp $PREFIX/include/
cd ..

echo 'Building zlib...'
tar xzf ../lib-src/zlib-1.2.11.tar.gz
cd zlib-1.2.11
$CC -I. -I./src -fPIC -std=c89 -Wno-deprecated-non-prototype -c adler32.c
$CC -I. -I./src -fPIC -std=c89 -Wno-deprecated-non-prototype -c infback.c
$CC -I. -I./src -fPIC -std=c89 -Wno-deprecated-non-prototype -c trees.c
$CC -I. -I./src -fPIC -std=c89 -Wno-deprecated-non-prototype -c compress.c
$CC -I. -I./src -fPIC -std=c89 -Wno-deprecated-non-prototype -c inffast.c
$CC -I. -I./src -fPIC -std=c89 -Wno-deprecated-non-prototype -c uncompr.c
$CC -I. -I./src -fPIC -std=c89 -Wno-deprecated-non-prototype -c crc32.c
$CC -I. -I./src -fPIC -std=c89 -Wno-deprecated-non-prototype -c inflate.c
$CC -I. -I./src -fPIC -std=c89 -Wno-deprecated-non-prototype -c deflate.c
$CC -I. -I./src -fPIC -std=c89 -Wno-deprecated-non-prototype -c inftrees.c
$AR rcs $PREFIX/lib/libz.a adler32.o infback.o trees.o compress.o inffast.o uncompr.o crc32.o inflate.o deflate.o inftrees.o
cp zlib.h zconf.h $PREFIX/include/
cd ..

echo 'Building libpng...'
tar xzf ../lib-src/libpng-1.6.43.tar.gz
cd libpng-1.6.43
cp scripts/pnglibconf.h.prebuilt pnglibconf.h
$CC -I. -I./src -I$PREFIX/include -fPIC -c pngget.c
$CC -I. -I./src -I$PREFIX/include -fPIC -c pngread.c
$CC -I. -I./src -I$PREFIX/include -fPIC -c pngrutil.c
$CC -I. -I./src -I$PREFIX/include -fPIC -c pngtrans.c
$CC -I. -I./src -I$PREFIX/include -fPIC -c pngwtran.c
$CC -I. -I./src -I$PREFIX/include -fPIC -c png.c
$CC -I. -I./src -I$PREFIX/include -fPIC -c pngmem.c
$CC -I. -I./src -I$PREFIX/include -fPIC -c pngrio.c
$CC -I. -I./src -I$PREFIX/include -fPIC -c pngset.c
$CC -I. -I./src -I$PREFIX/include -fPIC -c pngwio.c
$CC -I. -I./src -I$PREFIX/include -fPIC -c pngwutil.c
$CC -I. -I./src -I$PREFIX/include -fPIC -c pngerror.c
$CC -I. -I./src -I$PREFIX/include -fPIC -c pngpread.c
$CC -I. -I./src -I$PREFIX/include -fPIC -c pngrtran.c
$CC -I. -I./src -I$PREFIX/include -fPIC -c pngwrite.c
$AR rcs $PREFIX/lib/libpng.a pngget.o pngread.o pngrutil.o pngtrans.o pngwtran.o png.o pngmem.o pngrio.o pngset.o pngwio.o pngwutil.o pngerror.o pngpread.o pngrtran.o pngwrite.o
mkdir $PREFIX/include/png
cp *.h $PREFIX/include/png/
cd ..

echo 'Building jpeg9...'
tar xzf ../lib-src/jpegsrc.v9e.tar.gz
cd jpeg-9e
cp jconfig.txt jconfig.h
$CC -fPIC -c cdjpeg.c
$CC -fPIC -c jaricom.c
$CC -fPIC -c jcapimin.c
$CC -fPIC -c jcapistd.c
$CC -fPIC -c jcarith.c
$CC -fPIC -c jccoefct.c
$CC -fPIC -c jccolor.c
$CC -fPIC -c jcdctmgr.c
$CC -fPIC -c jchuff.c
$CC -fPIC -c jcinit.c
$CC -fPIC -c jcmainct.c
$CC -fPIC -c jcmarker.c
$CC -fPIC -c jcmaster.c
$CC -fPIC -c jcomapi.c
$CC -fPIC -c jcparam.c
$CC -fPIC -c jcprepct.c
$CC -fPIC -c jcsample.c
$CC -fPIC -c jctrans.c
$CC -fPIC -c jdapimin.c
$CC -fPIC -c jdapistd.c
$CC -fPIC -c jdarith.c
$CC -fPIC -c jdatadst.c
$CC -fPIC -c jdatasrc.c
$CC -fPIC -c jdcoefct.c
$CC -fPIC -c jdcolor.c
$CC -fPIC -c jddctmgr.c
$CC -fPIC -c jdhuff.c
$CC -fPIC -c jdinput.c
$CC -fPIC -c jdmainct.c
$CC -fPIC -c jdmarker.c
$CC -fPIC -c jdmaster.c
$CC -fPIC -c jdmerge.c
$CC -fPIC -c jdpostct.c
$CC -fPIC -c jdsample.c
$CC -fPIC -c jdtrans.c
$CC -fPIC -c jerror.c
$CC -fPIC -c jfdctflt.c
$CC -fPIC -c jfdctfst.c
$CC -fPIC -c jfdctint.c
$CC -fPIC -c jidctflt.c
$CC -fPIC -c jidctfst.c
$CC -fPIC -c jidctint.c
$CC -fPIC -c jmemansi.c
$CC -fPIC -c jmemmgr.c
$CC -fPIC -c jmemname.c
$CC -fPIC -c jmemnobs.c
$CC -fPIC -c jquant1.c
$CC -fPIC -c jquant2.c
$CC -fPIC -c jutils.c
$CC -fPIC -c transupp.c
$AR rcs $PREFIX/lib/libjpeg.a cdjpeg.o jaricom.o jcapimin.o jcapistd.o jcarith.o jccoefct.o jccolor.o jcdctmgr.o jchuff.o jcinit.o jcmainct.o jcmarker.o jcmaster.o jcomapi.o jcparam.o jcprepct.o jcsample.o jctrans.o jdapimin.o jdapistd.o jdarith.o jdatadst.o jdatasrc.o jdcoefct.o jdcolor.o jddctmgr.o jdhuff.o jdinput.o jdmainct.o jdmarker.o jdmaster.o jdmerge.o jdpostct.o jdsample.o jdtrans.o jerror.o jfdctflt.o jfdctfst.o jfdctint.o jidctflt.o jidctfst.o jidctint.o jmemansi.o jmemmgr.o jmemname.o jmemnobs.o jquant1.o jquant2.o jutils.o transupp.o
mkdir $PREFIX/include/jpeg
cp *.h $PREFIX/include/jpeg/
cd ..

echo 'Building libogg...'
tar xzf ../lib-src/libogg-1.3.3.tar.gz
cd libogg-1.3.3
sed -e 's/@INCLUDE_INTTYPES_H@/1/g' \
    -e 's/@INCLUDE_STDINT_H@/1/g' \
    -e 's/@INCLUDE_SYS_TYPES_H@/1/g' \
    -e 's/@SIZE16@/int16_t/g' \
    -e 's/@USIZE16@/uint16_t/g' \
    -e 's/@SIZE32@/int32_t/g' \
    -e 's/@USIZE32@/uint32_t/g' \
    -e 's/@SIZE64@/uint64_t/g' \
    < include/ogg/config_types.h.in \
    > include/ogg/config_types.h
$CC -I./include -fPIC -c src/bitwise.c
$CC -I./include -fPIC -c src/framing.c
$AR rcs $PREFIX/lib/libogg.a bitwise.o framing.o
cp -R include/ogg $PREFIX/include/
cd ..

echo 'Building libvorbis...'
tar xzf ../lib-src/libvorbis-1.3.6.tar.gz
cd libvorbis-1.3.6
$CC -I./include -I$PREFIX/include -fPIC -c lib/analysis.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/barkmel.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/bitrate.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/block.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/codebook.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/envelope.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/floor0.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/floor1.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/info.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/lookup.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/lpc.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/lsp.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/mapping0.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/mdct.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/psy.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/registry.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/res0.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/sharedbook.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/smallft.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/synthesis.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/tone.c
$CC -I./include -I$PREFIX/include -I./lib -fPIC -c lib/vorbisenc.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/vorbisfile.c
$CC -I./include -I$PREFIX/include -fPIC -c lib/window.c
$AR rcs $PREFIX/lib/libvorbis.a analysis.o barkmel.o bitrate.o block.o codebook.o envelope.o floor0.o floor1.o info.o lookup.o lpc.o lsp.o mapping0.o mdct.o psy.o registry.o res0.o sharedbook.o smallft.o synthesis.o tone.o vorbisenc.o vorbisfile.o window.o
cp -R include/vorbis $PREFIX/include/
cd ..

echo 'Building freetype2...'
tar xzf ../lib-src/freetype-2.13.2.tar.gz
cd freetype-2.13.2
sed -e 's/FONT_MODULES += type1//' \
    -e 's/FONT_MODULES += cid//' \
    -e 's/FONT_MODULES += pfr//' \
    -e 's/FONT_MODULES += type42//' \
    -e 's/FONT_MODULES += pcf//' \
    -e 's/FONT_MODULES += bdf//' \
    -e 's/FONT_MODULES += pshinter//' \
    -e 's/FONT_MODULES += raster//' \
    -e 's/FONT_MODULES += psaux//' \
    -e 's/FONT_MODULES += psnames//' \
    < modules.cfg > modules.cfg.new
mv modules.cfg.new modules.cfg
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftadvanc.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftbase.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftbbox.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftbdf.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftbitmap.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftcalc.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftcid.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftcolor.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftdbgmem.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftdebug.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/fterrors.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftfntfmt.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftfstype.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftgasp.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftgloadr.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftglyph.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftgxval.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/fthash.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftinit.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftlcdfil.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftmac.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftmm.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftobjs.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftotval.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftoutln.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftpatent.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftpfr.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftpsprop.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftrfork.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftsnames.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftstream.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftstroke.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftsynth.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftsystem.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/fttrigon.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/fttype1.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftutil.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/ftwinfnt.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/base/md5.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/bdf/bdf.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/bdf/bdfdrivr.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/bdf/bdflib.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/bzip2/ftbzip2.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cache/ftcache.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cache/ftcbasic.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cache/ftccache.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cache/ftccmap.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cache/ftcglyph.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cache/ftcimage.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cache/ftcmanag.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cache/ftcmru.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cache/ftcsbits.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cff/cff.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cff/cffcmap.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cff/cffdrivr.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cff/cffgload.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cff/cffload.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cff/cffobjs.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cff/cffparse.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cid/cidgload.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cid/cidload.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cid/cidobjs.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cid/cidparse.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cid/cidriver.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/cid/type1cid.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/otvalid/otvalid.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/otvalid/otvbase.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/otvalid/otvcommn.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/otvalid/otvgdef.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/otvalid/otvgpos.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/otvalid/otvgsub.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/otvalid/otvjstf.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/otvalid/otvmath.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/otvalid/otvmod.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/pcf/pcf.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/pcf/pcfdrivr.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/pcf/pcfread.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/pcf/pcfutil.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/pfr/pfr.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/pfr/pfrcmap.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/pfr/pfrdrivr.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/pfr/pfrgload.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/pfr/pfrload.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/pfr/pfrobjs.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/pfr/pfrsbit.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psaux/afmparse.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psaux/cffdecode.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psaux/psarrst.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psaux/psaux.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psaux/psauxmod.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psaux/psblues.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psaux/psconv.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psaux/pserror.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psaux/psfont.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psaux/psft.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psaux/pshints.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psaux/psintrp.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psaux/psobjs.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psaux/psread.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psaux/psstack.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psaux/t1cmap.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psaux/t1decode.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/pshinter/pshalgo.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/pshinter/pshglob.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/pshinter/pshinter.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/pshinter/pshmod.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/pshinter/pshrec.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psnames/psmodule.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/psnames/psnames.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/raster/ftraster.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/raster/ftrend1.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/raster/raster.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sdf/ftbsdf.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sdf/ftsdf.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sdf/ftsdfcommon.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sdf/ftsdfrend.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sdf/sdf.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sfnt/pngshim.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sfnt/sfdriver.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sfnt/sfobjs.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sfnt/sfwoff.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sfnt/sfwoff2.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sfnt/ttbdf.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sfnt/ttcmap.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sfnt/ttcolr.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sfnt/ttcpal.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sfnt/ttkern.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sfnt/ttload.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sfnt/ttmtx.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sfnt/ttpost.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sfnt/ttsbit.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sfnt/ttsvg.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/sfnt/woff2tags.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/smooth/ftgrays.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/smooth/ftsmooth.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/smooth/smooth.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/svg/ftsvg.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/svg/svg.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/truetype/truetype.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/truetype/ttdriver.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/truetype/ttgload.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/truetype/ttgxvar.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/truetype/ttinterp.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/truetype/ttobjs.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/truetype/ttpload.c
$CC -DFT2_BUILD_LIBRARY -I./include -I$PREFIX/include -fPIC -c src/winfonts/winfnt.c
$AR rcs $PREFIX/lib/libfreetype.a ftadvanc.o ftbase.o ftbbox.o ftbdf.o ftbitmap.o ftcalc.o ftcid.o ftcolor.o ftdbgmem.o ftdebug.o fterrors.o ftfntfmt.o ftfstype.o ftgasp.o ftgloadr.o ftglyph.o ftgxval.o fthash.o ftinit.o ftlcdfil.o ftmac.o ftmm.o ftobjs.o ftotval.o ftoutln.o ftpatent.o ftpfr.o ftpsprop.o ftrfork.o ftsnames.o ftstream.o ftstroke.o ftsynth.o ftsystem.o fttrigon.o fttype1.o ftutil.o ftwinfnt.o md5.o bdf.o bdfdrivr.o bdflib.o ftbzip2.o ftcache.o ftcbasic.o ftccache.o ftccmap.o ftcglyph.o ftcimage.o ftcmanag.o ftcmru.o ftcsbits.o cff.o cffcmap.o cffdrivr.o cffgload.o cffload.o cffobjs.o cffparse.o cidgload.o cidload.o cidobjs.o cidparse.o cidriver.o type1cid.o otvalid.o otvbase.o otvcommn.o otvgdef.o otvgpos.o otvgsub.o otvjstf.o otvmath.o otvmod.o pcf.o pcfdrivr.o pcfread.o pcfutil.o pfr.o pfrcmap.o pfrdrivr.o pfrgload.o pfrload.o pfrobjs.o pfrsbit.o afmparse.o cffdecode.o psarrst.o psaux.o psauxmod.o psblues.o psconv.o pserror.o psfont.o psft.o pshints.o psintrp.o psobjs.o psread.o psstack.o t1cmap.o t1decode.o pshalgo.o pshglob.o pshinter.o pshmod.o pshrec.o psmodule.o psnames.o ftraster.o ftrend1.o raster.o ftbsdf.o ftsdf.o ftsdfcommon.o ftsdfrend.o sdf.o pngshim.o sfdriver.o sfobjs.o sfwoff.o sfwoff2.o ttbdf.o ttcmap.o ttcolr.o ttcpal.o ttkern.o ttload.o ttmtx.o ttpost.o ttsbit.o ttsvg.o woff2tags.o ftgrays.o ftsmooth.o smooth.o ftsvg.o svg.o truetype.o ttdriver.o ttgload.o ttgxvar.o ttinterp.o ttobjs.o ttpload.o winfnt.o
cp -R include/freetype $PREFIX/include/freetype
cp include/ft2build.h $PREFIX/include/
cd ..

cd ..
rm -rf tmp
echo 'Finished.'
