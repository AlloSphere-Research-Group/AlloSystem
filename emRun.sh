#!/bin/sh

# Build and run source file in browser using Emscripten
# Note that the first time running this may take some time as Emscripten configures its libraries.

if [ $# -eq 0 ]; then
	echo "Usage: ./$(basename "$0") SOURCE_FILE [MAKE_ARGS]"
	exit 1
fi

if [[ `echo $EM_CONFIG` ]]; then
	# Assume PATH already configured via emsdk_env.sh
	EM_DIR=
elif [[ `echo $EMSDK` ]]; then
	EM_DIR=${EMSDK}/upstream/emscripten/
else
	echo "Emscripten directory not found"
	exit
fi

#BROWSER=chrome
#BROWSER=firefox
if [[ $BROWSER ]]; then BROWSER="--browser $BROWSER"; fi

EM_RUN="${EM_DIR}emrun --no_emrun_detect $BROWSER"

EXT="${1##*.}"

# If passed a .html, just run it
if [ $EXT == "html" ]; then
	$EM_RUN "$1"
	exit
fi

ALLO_DIR=$PWD
BUILD_DIR=$ALLO_DIR/build/em/
PROJ_NAME=$(basename "$1" | cut -d. -f1)
SOURCE_DIR="$(dirname "$1")/"
OUTPUT_DIR="${BUILD_DIR}bin/$PROJ_NAME/"

# Should this be in makefile?
mkdir -p $OUTPUT_DIR

# Parse emcc (linker) options from source code
PRAGMA_KEY="#pragma EM"
# Find lines with pragma key and excluding those in comments
OPTIONS="$(grep "$PRAGMA_KEY" $1 | grep -v "^[[:blank:]]*//")"
# Remove pragma directive
OPTIONS="${OPTIONS//$PRAGMA_KEY /}"
OPTIONS="${OPTIONS//RUN_MAIN_SOURCE_DIR/$SOURCE_DIR}"
# Strip whitespace at start/end and convert newlines to spaces (last sed strips off remaining space)
OPTIONS="$(echo "$OPTIONS" | sed 's/^[[:blank:]]*//;s/[[:blank:]]*$//' | tr '\n' ' ' | sed 's/ $//')"
#echo "[$OPTIONS]"; exit

# Call make and forward all command-line args to it
${EM_DIR}emmake make $@ PLATFORM=em ARCH=none BUILD_DIR=$BUILD_DIR AUTORUN=0 EXE_DIR=$OUTPUT_DIR EXE_EXT=.js RUN_DIRS=$SOURCE_DIR CXX=${EM_DIR}emcc RUN_USER_LDFLAGS="$OPTIONS" LDFLAGS=

# Exit if compilation errors...
if [[ $? != 0 ]]; then exit $?; fi

HTML_NAME=$PROJ_NAME

# Check for HTML_NAME specified in arguments
# It will be passed to make above, but does nothing there.
for arg in "$@"; do
	if [ "${arg%%=*}" = "HTML_NAME" ]; then
		HTML_NAME="${arg##*=}"
		break
	fi
done

# Create a minimal HTML page
sed s/PROJ_NAME/$PROJ_NAME/g emMain.html > $OUTPUT_DIR/$HTML_NAME.html

# Run HTML
# You cannot simply double-click the .html since you need a local server.
# Note that this blocks in terminal even after page close!
$EM_RUN $OUTPUT_DIR/$HTML_NAME.html
