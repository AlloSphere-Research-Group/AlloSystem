#!/bin/sh

if [ $# -eq 0 ]; then
	echo "Usage: ./$(basename "$0") [FILE]"
	exit
fi

files_exist(){
	ls -u "$@" >/dev/null 2>&1;
}

# Build executable first as we may need it to check for library dependencies
./run.sh $1 AUTORUN=0

BUILDDIR="build"
EXENAME=$(basename "$1" | cut -d. -f1)
EXEDIR="$BUILDDIR/$EXENAME"
LIBS=

# MSYS
if files_exist /msys.bat; then

	LIBNAMES="`objdump -p $BUILDDIR/bin/${EXENAME}.exe | grep "DLL Name"`"
	LIBNAMES="${LIBNAMES//DLL Name: /}"
	#echo $LIBNAMES
	LIBDIRS="/mingw/bin/ /usr/local/bin/"

	for dir in $LIBDIRS; do
		for lib in $LIBNAMES; do
			DIR_LIB=$dir$lib
			if files_exist $DIR_LIB; then LIBS="$LIBS $DIR_LIB"; fi
		done
	done
	#echo $LIBS

else
	echo "Error: Could not build standalone. Platform not supported."
	exit
fi

mkdir -p $EXEDIR
cp -u $LIBS $EXEDIR
mv $BUILDDIR/bin/$EXENAME $EXEDIR

echo "Created standalone in $EXEDIR/"
