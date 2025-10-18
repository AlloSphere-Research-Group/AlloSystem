#!/bin/sh

#NOTE: emmake and emar shell scripts removed as of version 4.0.13! Therefore, if using version 4.0.13 or higher, this script will fail. The latest supported emscripten is 4.0.12:
#https://github.com/emscripten-core/emscripten/tree/4.0.12

# List of libraries to compile
LIB_DIRS="../Gamma allocore"

ALLO_DIR=$PWD
BUILD_DIR=$ALLO_DIR/build/em/

if [[ `echo $EM_CONFIG` ]]; then
	# Assume PATH already configured via emsdk_env.sh
	EM_DIR=
elif [[ `echo $EMSDK` ]]; then
	EM_DIR=${EMSDK}/upstream/emscripten/
else
	echo "Emscripten directory not found"
	exit
fi

for libdir in $LIB_DIRS; do
	[ -d "$libdir" ] || continue # skip if directory does not exist
	cd $libdir
	${EM_DIR}emmake make install $* PLATFORM=em ARCH=none SDL_VERSION=3 WINDOW_BINDING=SDL AUDIO_BINDING=SDL USE_HID=0 USE_MIDI=0 USE_ZEROCONF=0 NO_AUDIO_IO=1 BUILD_DIR=$BUILD_DIR DESTDIR=$BUILD_DIR
	cd $ALLO_DIR
	#mkdir -p $BUILD_DIR/include/$lib
	#cp -R ../$lib/$lib $BUILD_DIR/include/
done
