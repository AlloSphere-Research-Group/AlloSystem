#!/bin/bash

# Change this to suit your needs
BUILD_ALLOUTIL=1
BUILD_ALLOGLV=1
BUILD_GLV=0
BUILD_VSR=0
BUILD_GAMMA=0

# ------------------------------------------------
# You shouldn't need to touch the stuff below

CMAKE_FLAGS="-DBUILD_ALLOUTIL=${BUILD_ALLOUTIL} -DBUILD_ALLOGLV=${BUILD_ALLOGLV} -DBUILD_GLV=${BUILD_GLV} -DBUILD_VSR=${BUILD_VSR} -DBUILD_GAMMA=${BUILD_GAMMA}"

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

cmake . ${CMAKE_FLAGS} ${TARGET_FLAG} -DNO_EXAMPLES=1 -DRUN_IN_DEBUGGER=1 -DCMAKE_BUILD_TYPE=Debug
make $TARGET -j4 $*
