#!/bin/bash

# pkg-config doesn't know about the binary gd, so we disable it and 
#  hardwire the variables we know about in the environment

HERE="`pwd`"

export PKG_CONFIG=true  
export CFLAGS="-mno-cygwin"
export LDFLAGS="-mno-cygwin"
export GDLIB_CFLAGS="-I$HERE/../../gdwin32/include" 
export GDLIB_LIBS="-L$HERE/../../gdwin32/lib -lbgd"

(cd ../../
 make distclean
 ./autogen.sh
 ./configure  
 make distcheck || (echo "Distcheck failed!"; exit))

DIST_FILE=`ls ../../mscgen-*.tar.gz`
DIST_VER=`echo "$DIST_FILE" | sed "s/^.*-\([0-9]\+.[0-9]\+\).*$/\1/"`

rm -rf binstage buildstage mscgen-$DIST_VER

# Copy and unpack the source bundle
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
(cd binstage/w32 && zip -r mscgen-w32-$DIST_VER.zip mscgen-$DIST_VER && mv mscgen-w32-$DIST_VER.zip ../..)
md5sum mscgen-w32-$DIST_VER.zip > mscgen-w32-$DIST_VER.zip.md5

# Create the installer
sed "s/\!define VERSION.*$/\!define VERSION $DIST_VER/" installer.nsi > installer-fixed.nsi

makensis installer-fixed.nsi

# Clean up
rm -rf binstage buildstage mscgen-$DIST_VER mscgen-$DIST_VER.tar.gz installer-fixed.nsi


# END OF FILE
