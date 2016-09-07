#!/bin/sh

if [ $# -eq 0 ]; then
	echo "Usage: ./$(basename "$0") [FILE]"
	exit
fi

files_exist(){
	ls -u "$@" >/dev/null 2>&1;
}

# MSYS
if files_exist /msys.bat; then
	LIBDIR="/usr/local/bin"
	LIBNAMES="`ls $LIBDIR`"
	for v in $LIBNAMES; do LIBS="$LIBS $LIBDIR/$v"; done
	LIBS="$LIBS /mingw/bin/libstdc++-6.dll /mingw/bin/libgcc_s_dw2-1.dll"
	#echo $LIBS
else
	echo "Platform not supported."
	exit
fi

BUILDDIR="build"
EXENAME=$(basename "$1" | cut -d. -f1)
EXEDIR="$BUILDDIR/$EXENAME"

mkdir -p $EXEDIR
cp -u $LIBS $EXEDIR
./run.sh $1 AUTORUN=0
mv $BUILDDIR/bin/$EXENAME $EXEDIR

echo "Created standalone in $EXEDIR/"
