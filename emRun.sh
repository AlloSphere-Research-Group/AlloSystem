#!/bin/sh

# Build and run source file in browser using Emscripten
# Note that the first time running this may take some time as Emscripten configures its libraries.

EM_DIR=`grep EMSCRIPTEN_ROOT ~/.emscripten | cut -d "=" -f 2 | tr -d "'"`
ALLO_DIR=$PWD
BUILD_DIR=$ALLO_DIR/build/em/
PROJ_NAME=$(basename "$1" | cut -d. -f1)
OUTPUT_DIR="$BUILD_DIR/$PROJ_NAME"

OBJS=`ls $BUILD_DIR/obj/*.o`
CPPFLAGS="-I$BUILD_DIR/include -O2"
CXXFLAGS="-std=c++11"
EMFLAGS+=" -s USE_SDL=2"
#EMFLAGS+=" -s LEGACY_GL_EMULATION=1"
#EMFLAGS+=" -s USE_WEBGL2=1" #default, recommended setting
EMFLAGS+=" -s FULL_ES2=1" #OpenGL ES 2.0 emulation (req'd for client-side arrays)
#EMFLAGS+=" -s EMTERPRETIFY=1"
#EMFLAGS+=" -s LINKABLE=1"
EMFLAGS+=" --emrun" # necessary to capture stdout, stderr, and exit

mkdir -p $OUTPUT_DIR

$EM_DIR/emcc $CPPFLAGS $CXXFLAGS $1 $EMFLAGS $OBJS -o $OUTPUT_DIR/$PROJ_NAME.html

# Exit if compilation errors...
if [[ $? != 0 ]]; then exit $?; fi

# Run HTML
# You cannot simply double-click the .html since you need a local server.
# Note that this blocks in terminal even after page close!
$EM_DIR/emrun --no_emrun_detect $OUTPUT_DIR/$PROJ_NAME.html

# Run local server
#python -m SimpleHTTPServer 8888
#http://localhost:8888/
