#!/bin/sh

# ------------------------------------------------
# You shouldn't need to touch the stuff below

if [ $# = 0 ]
then
    echo Aborting: You must provide a source filename or a directory
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

#echo FILENAME: ${FILENAME}
#echo TARGET: ${TARGET}

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

cmake . "$TARGET_FLAG" "$DBUILD_FLAG" -DRUN_IN_DEBUGGER=0 -DCMAKE_BUILD_TYPE=Release -Wno-dev
make "$TARGET" -j7 "$*"
