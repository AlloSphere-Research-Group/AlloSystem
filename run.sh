#!/bin/bash

# ------------------------------------------------
# You shouldn't need to touch the stuff below

if [ $# == 0 ]
then
    echo Aborting: You must provide a source filename or a directory
    exit 1
fi

FILENAME=$(basename "$1")
DIRNAME=$(dirname "$1")

if [ ${DIRNAME} == "." ]
then
  FILENAME="${FILENAME%.*}"
else
  FILENAME="${DIRNAME//./_}_${FILENAME%.*}"
fi

#echo FILENAME: ${FILENAME}
TARGET=${FILENAME//\//_}_run
#echo TARGET: ${TARGET}

if [ -f $1 ]
then
  TARGET_FLAG="-DBUILD_APP_FILE=$1 -DBUILD_DIR=0"
  echo RUN SCRIPT: Building file $1
elif [ -d $1 ]
then
  TARGET_FLAG="-DBUILD_APP_DIR=$1 -DBUILD_DIR=1"
  echo RUN SCRIPT: Building all files in dir $1
else
  echo Aborting: $1 is neither a file nor directory
  exit 1
fi

cmake . ${TARGET_FLAG} -DRUN_IN_DEBUGGER=0 -DCMAKE_BUILD_TYPE=Release -Wno-dev
make $TARGET -j7 $*
