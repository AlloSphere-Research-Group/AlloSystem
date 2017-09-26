#!/bin/sh

sourceFile="$1"

# Which modules to view
#modules="allocore"
modules=`ls | grep allo`

# Override if modules are passed in as arguments.
#if [ "$#" -gt 0 ]; then
#  modules="$*"
#fi

# Extra modules sitting alongside AlloSystem
modulesExt="Gamma GLV"

ALLO_DIR=$PWD
PLATFORM=`uname`

### OSX / Xcode
STR='<?xml version="1.0" encoding="UTF-8"?>\n'\
'<Workspace version="1.0">\n'

if [ "${sourceFile}" ]; then
	STR=${STR}'<FileRef location = "group:'${ALLO_DIR}'/'${sourceFile}'"></FileRef>\n'
fi

for m in $modules; do
	STR=${STR}'<Group location="container:" name="'${m}'">'
	STR=${STR}'<FileRef location="group:'${ALLO_DIR}'/'${m}'/'${m}'"></FileRef>\n'
	STR=${STR}'<FileRef location="group:'${ALLO_DIR}'/'${m}'/src"></FileRef>\n'
	STR=${STR}'<FileRef location="group:'${ALLO_DIR}'/'${m}'/examples"></FileRef>\n'
	STR=${STR}'</Group>'
done

for m in $modulesExt; do
	STR=${STR}'<Group location="container:" name="'${m}'">'
	STR=${STR}'<FileRef location="group:'${ALLO_DIR}'/../'${m}'/'${m}'"></FileRef>\n'
	STR=${STR}'<FileRef location="group:'${ALLO_DIR}'/../'${m}'/src"></FileRef>\n'
	STR=${STR}'<FileRef location="group:'${ALLO_DIR}'/../'${m}'/examples"></FileRef>\n'
	STR=${STR}'</Group>'
done

STR=${STR}'</Workspace>'

# Create the files
DEST_DIR=build/
PROJ_DIR=allo.xcworkspace

mkdir $DEST_DIR >/dev/null 2>&1;
cd $DEST_DIR
mkdir $PROJ_DIR >/dev/null 2>&1;
#TEMP_DIR=`mktemp -d allo_XXXX`
#PROJ_DIR=${TEMP_DIR}.xcworkspace
#mv $TEMP_DIR $PROJ_DIR
cd $PROJ_DIR
echo $STR > contents.xcworkspacedata
cd ..
open $PROJ_DIR
