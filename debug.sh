#!/bin/sh

DEBUGGER=gdb

# ------------------------------------------------
# You shouldn't need to touch the stuff below

# Get the number of processors on OS X, linux, and (to-do) Windows.
NPROC=$(grep --count ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu || 2)
# Save one core for the gui.
PROC_FLAG=$((NPROC - 1))

if [ $# = 0 ]
then
    echo Aborting: You must provide a source filename or a directory
    exit 1
fi

# Remove file extension.
FILENAME=$(basename "$1" | sed 's/\.[^.]*$//')
DIRNAME=$(dirname "$1")

# Combine and replace periods and slashes with underscores.
TARGET=$(echo "${DIRNAME}_${FILENAME}_run" | sed 's/\./_/g' | sed 's/\//_/g')

# Had to separate out DBUILD_FLAG to ensure correct globbing in the cmake line.
if [ -f "$1" ]; then
  TARGET_FLAG="-DBUILD_APP_FILE=$1"
  DBUILD_FLAG="-DBUILD_DIR=0"
  echo RUN SCRIPT: Building file "$1"
elif [ -d "$1" ]; then
  TARGET_FLAG="-DBUILD_APP_DIR=$1"
  DBUILD_FLAG="-DBUILD_DIR=1"
  echo RUN SCRIPT: Building all files in dir "$1"
else
  echo Aborting: "$1" is neither a file nor directory
  exit 1
fi

cmake . "$CMAKE_FLAGS" "$TARGET_FLAG" "$DBUILD_FLAG" -DRUN_IN_DEBUGGER=1 -DALLOSYSTEM_DEBUGGER=${DEBUGGER} -DCMAKE_BUILD_TYPE=Debug
make "$TARGET" -j"$PROC_FLAG" "$*"
