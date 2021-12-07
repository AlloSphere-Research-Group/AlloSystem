#!/bin/sh

# Build and run source file in browser using Emscripten
# Note that the first time running this may take some time as Emscripten configures its libraries.

if [[ `echo $EM_CONFIG` ]]; then
	# Assume PATH already configured via emsdk_env.sh
	EM_DIR=
else
	# EM_DIR will be something like */emsdk/emscripten/1.37.34
	EM_DIR=`grep EMSCRIPTEN_ROOT ~/.emscripten | cut -d "=" -f 2 | sed -e s/^..// -e s/.$//`
	EM_DIR+=/
fi

#BROWSER=chrome
#BROWSER=firefox
if [[ $BROWSER ]]; then BROWSER="--browser $BROWSER"; fi

# If passed a .html, just emrun it
if [ "${1##*.}" == "html" ]; then
	${EM_DIR}emrun --no_emrun_detect $BROWSER "$1"
	exit
fi

ALLO_DIR=$PWD
BUILD_DIR=$ALLO_DIR/build/em/
PROJ_NAME=$(basename "$1" | cut -d. -f1)
SOURCE_DIR="$(dirname "$1")/"
OUTPUT_DIR="${BUILD_DIR}bin/$PROJ_NAME"

OBJS=`ls ${BUILD_DIR}obj/*.o`
CPPFLAGS="-I${BUILD_DIR}include -O2"
CPPFLAGS+=' -DRUN_MAIN_SOURCE_DIR="./"'
CFLAGS="-O2"
CXXFLAGS="-std=c++14"
EMFLAGS+=" -s USE_SDL=2"
#EMFLAGS+=" -s LEGACY_GL_EMULATION=1"
#EMFLAGS+=" -s USE_WEBGL2=1" #default, recommended setting
EMFLAGS+=" -s FULL_ES2=1" #OpenGL ES 2.0 emulation (req'd for client-side arrays)
EMFLAGS+=" --emrun" # necessary to capture stdout, stderr, and exit
#EMFLAGS+=" -s ASSERTIONS=1" # get more info on runtime errors
#EMFLAGS+=" --cpuprofiler" # adds profiler to generated page
#EMFLAGS+=" -lwebsocket.js" #WebSockets API
#EMFLAGS+=" -lwebsocket.js -s PROXY_POSIX_SOCKETS=1 -s USE_PTHREADS=1 -s PROXY_TO_PTHREAD=1" #full POSIX socket emulation over WebSockets

mkdir -p $OUTPUT_DIR

# Parse emcc options from source code
PRAGMA_KEY="#pragma EM"
# strip leading slash
#SOURCE_DIR="${SOURCE_DIR#/}"
OPTIONS="$(grep "$PRAGMA_KEY" $1 | grep -v "^[[:blank:]]*//")"
OPTIONS="${OPTIONS//$PRAGMA_KEY /}"
OPTIONS="${OPTIONS//RUN_MAIN_SOURCE_DIR/$SOURCE_DIR}"
OPTIONS="$(echo "$OPTIONS" | sed 's/^[ \t]*//;s/[ \t]*$//' | tr '\n' ' ')"
#echo "$OPTIONS"
#exit

# Build objects of any build-and-run sources
${EM_DIR}emmake make runobjs PLATFORM=em ARCH=none BUILD_DIR=$BUILD_DIR RUN_DIR=$SOURCE_DIR

#${EM_DIR}emcc $CPPFLAGS $CFLAGS $CXXFLAGS $1 $EMFLAGS $OBJS -o $OUTPUT_DIR/$PROJ_NAME.html
${EM_DIR}emcc $CPPFLAGS $CFLAGS $CXXFLAGS $1 $EMFLAGS $OBJS -o $OUTPUT_DIR/$PROJ_NAME.js $OPTIONS

# Exit if compilation errors...
if [[ $? != 0 ]]; then exit $?; fi

# Create a minimal HTML page
sed s/PROJ_NAME/$PROJ_NAME/g emMain.html > $OUTPUT_DIR/$PROJ_NAME.html

# Run HTML
# You cannot simply double-click the .html since you need a local server.
# Note that this blocks in terminal even after page close!
${EM_DIR}emrun --no_emrun_detect $BROWSER $OUTPUT_DIR/$PROJ_NAME.html

# Run local server
#python -m SimpleHTTPServer 8888
#http://localhost:8888/
