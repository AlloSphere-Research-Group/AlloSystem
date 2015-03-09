#!/bin/sh

# This script downloads source code from the web and rebuilds allo libs.
# Note this script should only used if you are not using a local git repository.
# Otherwise, use 'git pull origin makefile-build'.

# Helper functions.
binary_exists(){
	command -v "$1" >/dev/null 2>&1;
}

if binary_exists "wget"; then
	DOWNLOAD="wget --no-clobber --no-check-certificate"
elif binary_exists "curl"; then
	DOWNLOAD="curl -LO"
else
	echo "ERROR: No network download tool found (e.g., wget, curl). Update aborted."
	exit
fi

if binary_exists "unzip"; then
	UNZIP="unzip -q"
else
	echo "ERROR: No archiving tool found (e.g., unzip). Update aborted."
	exit
fi


ALLO_DIR=$PWD

cd /tmp
	PKG=makefile-build
	PKG_DIR=AlloSystem-$PKG

	$DOWNLOAD https://github.com/AlloSphere-Research-Group/AlloSystem/archive/$PKG.zip

	echo "Decompressing $PKG_DIR.zip"
	$UNZIP $PKG

	echo "Updating existing source"
	cp -ruv $PKG_DIR/* $ALLO_DIR

cd $ALLO_DIR
	BUILT_MODULES=`ls build/lib/ | sed 's/\.a//g; s/lib//g'`
	echo "Making" $BUILT_MODULES
	make $BUILT_MODULES

cd /tmp
	rm -rf $PKG_DIR/
	rm $PKG.*

