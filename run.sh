#!/bin/sh

# Pass -d to enter debugger mode.

if [ "$(uname -s)" = "Darwin" ]; then
  DEBUGGER=lldb
else
  DEBUGGER=gdb
fi

# ------------------------------------------------
# You shouldn't need to touch the stuff below.

# Runs the program through the specified debugger if -d is passed.
OPTIND=1
while getopts "d" opt; do
    case "$opt" in
    d)  debugger="$DEBUGGER"
        shift
        ;;
    esac
done

# Get the number of processors on OS X, linux, and (to-do) Windows.
NPROC=$(grep --count ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu || 2)
# Save one core for the gui.
PROC_FLAG=$((NPROC - 1))

if [ "$#" = 0 ]; then
    echo Aborting: You must provide a source filename or a directory.
    exit 1
fi

# Remove file extension.
FILENAME=$(basename "$1" | sed 's/\.[^.]*$//')
DIRNAME=$(dirname "$1")

# Replace all forward slashes with underscores.
TARGET=$(echo "${DIRNAME}_${FILENAME}_run" | sed 's/\//_/g')

if [ "$DIRNAME" != "." ]; then
  # Replace all periods with underscores.
  TARGET=$(echo "${TARGET}" | sed 's/\./_/g')
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

if [ -n "$debugger" ]; then
  cmake . "$TARGET_FLAG" "$DBUILD_FLAG" -DRUN_IN_DEBUGGER=1 "-DALLOSYSTEM_DEBUGGER=${debugger}" -DCMAKE_BUILD_TYPE=Debug > cmake_log.txt
else
  cmake . "$TARGET_FLAG" "$DBUILD_FLAG" -DRUN_IN_DEBUGGER=0 -DCMAKE_BUILD_TYPE=Release -Wno-dev > cmake_log.txt
fi

make $TARGET -j$PROC_FLAG $*

