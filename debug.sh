#!/bin/bash

DEBUGGER=gdb

# ------------------------------------------------
# You shouldn't need to touch the stuff below

if [ $# == 0 ]
then
    echo Aborting: You must provide a source filename or a directory
    exit 1
fi

FILENAME=$(basename "$1")
DIRNAME=$(dirname "$1")
FILENAME="${DIRNAME//./_}_${FILENAME%.*}"

echo FILENAME: ${FILENAME}
TARGET=${FILENAME//\//_}_run
echo TARGET: ${TARGET}

if [ -f $1 ]
then
  TARGET_FLAG="-DBUILD_APP_FILE=$1 -DBUILD_DIR=0"
elif [ -d $1 ]
then
  TARGET_FLAG="-DBUILD_APP_DIR=$1 -DBUILD_DIR=1"
else
  echo Aborting: $1 is neither a file nor directory
  exit 1
fi

cmake . ${CMAKE_FLAGS} ${TARGET_FLAG} -DRUN_IN_DEBUGGER=1 -DALLOSYSTEM_DEBUGGER=${DEBUGGER} -DCMAKE_BUILD_TYPE=Debug
make $TARGET -j4 $*
