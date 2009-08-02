#!/bin/bash

# pkg-config doesn't know about the binary gd, so we disable it and 
#  hardwire the variables we know about in the environment

export PKG_CONFIG=true  
export CFLAGS="-mno-cygwin"
export LDFLAGS="-mno-cygwin"
export GDLIB_CFLAGS="-I`pwd`/../../gdwin32/include" 
export GDLIB_LIBS="-L`pwd`/../../gdwin32/lib -lbgd"

(cd ../../
 make distclean
 ./autogen.sh
 ./configure  
 make distcheck || (echo "Distcheck failed!"; exit))

DIST_FILE=`ls ../../mscgen-*.tar.gz`
DIST_VER=`echo "$DIST_FILE" | sed "s/^.*-\([0-9]\+.[0-9]\+\).*$/\1/"`

rm -rf binstage buildstage mscgen-$DIST_VER

# Copy any unpack the source bundle
cp $DIST_FILE mscgen-src-$DIST_VER.tar.gz
tar -xzf mscgen-src-$DIST_VER.tar.gz

# Build the Windows native version
mkdir -p buildstage/w32
mkdir -p binstage/w32/mscgen-$DIST_VER
(cd buildstage/w32 &&
 ../../mscgen-$DIST_VER/configure \
  --prefix="`pwd`/../../binstage/w32/mscgen-$DIST_VER" \
  && make install-strip)
cp ../../gdwin32/bin/bgd.dll binstage/w32/mscgen-$DIST_VER/bin
chmod a+x binstage/w32/mscgen-$DIST_VER/bin/bgd.dll
tar -C binstage/w32 -czf mscgen-w32-$DIST_VER.tar.gz mscgen-$DIST_VER
md5sum mscgen-w32-$DIST_VER.tar.gz > mscgen-w32-$DIST_VER.tar.gz.md5

# Clean up
rm -rf binstage buildstage mscgen-$DIST_VER mscgen-$DIST_VER.tar.gz


# END OF FILE