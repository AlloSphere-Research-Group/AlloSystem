#!/bin/sh

# List of libraries to compile
LIB_DIRS="../Gamma allocore"

ALLO_DIR=$PWD
BUILD_DIR=$ALLO_DIR/build/em/

if [[ `echo $EM_CONFIG` ]]; then
	# Assume PATH already configured via emsdk_env.sh
	EM_DIR=
else
	EM_DIR=`grep EMSCRIPTEN_ROOT ~/.emscripten | cut -d = -f 2 | sed -e s/^..// -e s/.$//`
	export PATH=$EM_DIR:$PATH # Change PATH since emmake calls emar, etc.
fi

for libdir in $LIB_DIRS; do
	cd $libdir
	emmake make install $* PLATFORM=em ARCH=none WINDOW_BINDING=SDL AUDIO_BINDING=SDL USE_HID=0 USE_MIDI=0 USE_ZEROCONF=0 USE_FONT=0 NO_AUDIO_IO=1 BUILD_DIR=$BUILD_DIR DESTDIR=$BUILD_DIR
	cd $ALLO_DIR
	#mkdir -p $BUILD_DIR/include/$lib
	#cp -R ../$lib/$lib $BUILD_DIR/include/
done
