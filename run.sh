#!/bin/sh

# Pass -d to enter debugger mode.

if [ "$(uname -s)" = "Darwin" ]; then
  DEBUGGER=lldb
else
  DEBUGGER=gdb
fi

# ------------------------------------------------
# You shouldn't need to touch the stuff below.

AUTORUN=1
ALLOSPHERE_APP_FLAG="-DBUILD_ALLOSPHERE_APP=0"

# Runs the program through the specified debugger if -d is passed.
OPTIND=1
while getopts ":dvnas" opt; do
  case "$opt" in
  d)  debugger="$DEBUGGER"
      shift
      ;;
  v)  VERBOSE="VERBOSE=1"
      shift
      ;;
  n)  AUTORUN=0
      shift
      ;;
  a)  ALLOSPHERE_APP_FLAG="-DBUILD_ALLOSPHERE_APP=1 -DBUILD_ALLOSPHERE_APP_AUDIO_RENDERER=1"
      shift
      ;;
  s)  ALLOSPHERE_APP_FLAG="-DBUILD_ALLOSPHERE_APP=1 -DBUILD_ALLOSPHERE_APP_AUDIO_RENDERER=0"
      shift
      ;;
  esac
done

# Get the number of processors on OS X; Linux; or MSYS2, or take a best guess.
NPROC=$(grep --count ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu || nproc || echo 2)
# Save one core for the gui.
PROC_FLAG=$((NPROC - 1))

# -eq performs a numerical comparison.
if [ "$#" -eq 0 ]; then
  echo Aborting: You must provide a source filename or a directory.
  exit 1
fi

# Remove file extension.
FILENAME=$(basename "$1" | sed 's/\.[^.]*$//')
DIRNAME=$(dirname "$1")

# Build name replacing all forward slashes with underscores.
if [ "$DIRNAME" != "." ]; then
  TARGET=$(echo "${DIRNAME}_${FILENAME}" | sed 's/\//_/g')
    # Replace all periods with underscores.
    TARGET=$(echo "${TARGET}" | sed 's/\./_/g')
else
  TARGET=$(echo "${FILENAME}" | sed 's/\//_/g')
fi

if [ "$AUTORUN" -eq 1 ]; then
  TARGET="${TARGET}_run"
fi

# Change behavior if the target is a file or directory.
if [ -f "$1" ]; then
  TARGET_FLAG="-DALLOSYSTEM_BUILD_APP_FILE=$1"
  DBUILD_FLAG="-DALLOSYSTEM_BUILD_DIR=0"
  echo RUN SCRIPT: Building file "$1".
elif [ -d "$1" ]; then
  TARGET_FLAG="-DALLOSYSTEM_BUILD_APP_DIR=$1"
  DBUILD_FLAG="-DALLOSYSTEM_BUILD_DIR=1"
  echo RUN SCRIPT: Building all files in dir "$1".
else
  echo Aborting: "$1" is neither a file nor directory.
  exit 1
fi
# Don't pass target as Make flag.
shift

if [ "$MSYSTEM" = "MINGW64" -o "$MSYSTEM" = "MINGW32" ]; then
  GENERATOR_FLAG="-GMSYS Makefiles"
fi

FLAGS="$TARGET_FLAG $DBUILD_FLAG $ALLOSPHERE_APP_FLAG"

if [ ! -d "build" ]; then
  mkdir build
fi

cd build


# GENERATOR_FLAG needs to be separated out in order to be parsed as a commandline flag with an argument with a space in it.
if [ -n "$debugger" ]; then
  cmake "$GENERATOR_FLAG" $FLAGS -DRUN_IN_DEBUGGER=1 "-DALLOSYSTEM_DEBUGGER=${debugger}" -DCMAKE_BUILD_TYPE=Debug .. > cmake_log.txt
else
  cmake "$GENERATOR_FLAG" $FLAGS -DRUN_IN_DEBUGGER=0 -DCMAKE_BUILD_TYPE=Release -Wno-dev .. > cmake_log.txt
fi

make $VERBOSE $TARGET -j$PROC_FLAG $*
