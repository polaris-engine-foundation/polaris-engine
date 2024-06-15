#!/bin/sh

set -eu

PREFIX=`pwd`/libroot

export MACOSX_DEPLOYMENT_TARGET=10.13

rm -rf tmp libroot
mkdir -p tmp libroot/include libroot/lib

cd tmp

echo 'Building brotli...'
tar xzf ../../libsrc/brotli-1.1.0.tar.gz
cd brotli-1.1.0
cmake -DBUILD_SHARED_LIBS="off" .
make
cp libbrotlidec.a libbrotlicommon.a ../../libroot/lib
cp -R c/include/brotli ../../libroot/include/
cd ..

echo 'building zlib...'
tar xzf ../../libsrc/zlib-1.2.11.tar.gz
cd zlib-1.2.11
./configure --prefix=$PREFIX --static --archs="-arch arm64 -arch x86_64"
make
make install
cd ..

echo 'building libpng...'
tar xzf ../../libsrc/libpng-1.6.43.tar.gz
cd libpng-1.6.43
./configure --prefix=$PREFIX --disable-shared CPPFLAGS=-I$PREFIX/include CFLAGS="-arch arm64 -arch x86_64" LDFLAGS="-L$PREFIX/lib -arch arm64 -arch x86_64"
make
make install
cd ..

echo 'Building bzip2...'
tar xzf ../../libsrc/bzip2-1.0.6.tar.gz
cd bzip2-1.0.6
make libbz2.a CFLAGS='-arch arm64 -arch x86_64 -O3 -ffunction-sections -fdata-sections'
cp bzlib.h ../../libroot/include/
cp libbz2.a ../../libroot/lib/
cd ..

echo 'Building libwebp...'
tar xzf ../../libsrc/libwebp-1.3.2.tar.gz
cd libwebp-1.3.2
./configure --prefix=$PREFIX --enable-static --disable-shared CPPFLAGS=-I$PREFIX/include CFLAGS="-arch arm64 -arch x86_64" LDFLAGS="-L$PREFIX/lib -arch arm64 -arch x86_64"
make
make install
cd ..

echo 'building jpeg9...'
tar xzf ../../libsrc/jpegsrc.v9e.tar.gz
cd jpeg-9e
./configure --prefix=$PREFIX --disable-shared CPPFLAGS=-I$PREFIX/include CFLAGS="-arch arm64 -arch x86_64" LDFLAGS="-L$PREFIX/lib -arch arm64 -arch x86_64"
make
make install
cd ..

tar xzf ../../libsrc/libogg-1.3.3.tar.gz
cd libogg-1.3.3
./configure --prefix=$PREFIX --disable-shared CFLAGS="-arch arm64 -arch x86_64" LDFLAGS="-arch arm64 -arch x86_64"
make
make install
cd ..

tar xzf ../../libsrc/libvorbis-1.3.6.tar.gz
cd libvorbis-1.3.6
sed -e 's/-force_cpusubtype_ALL//g' < configure > configure.new
chmod +x configure.new
./configure.new --prefix=$PREFIX --disable-shared PKG_CONFIG="" --with-ogg-includes=$PREFIX/include --with-ogg-libraries=$PREFIX/lib CFLAGS="-arch arm64 -arch x86_64" LDFLAGS="-arch arm64 -arch x86_64"
make
make install
cd ..

tar xzf ../../libsrc/freetype-2.13.2.tar.gz
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
./configure --prefix=$PREFIX --enable-static --disable-shared --with-png=yes --with-harfbuzz=no --with-zlib=yes --with-bzip2=yes --with-brotli=yes CFLAGS='-arch arm64 -arch x86_64' LDFLAGS="-arch arm64 -arch x86_64" ZLIB_CFLAGS='-I../../libroot/include' ZLIB_LIBS='-L../../libroot/lib -lz' BZIP2_CFLAGS='-I../../libroot/include' BZIP2_LIBS='-L../../libroot/lib -lbz2' LIBPNG_CFLAGS='-I../../libroot/include' LIBPNG_LIBS='-L../../libroot/lib -lpng' BROTLI_CFLAGS='-I../../libroot/include' BROTLI_LIBS='-L../../libroot/lib -lbrotlidec -lbrotlicommon'
make
make install
cd ..

cd ..
rm -rf tmp

rm -rf libroot/bin libroot/share

unset MACOSX_DEPLOYMENT_TARGET
