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
BUILDEXEBASE=$BUILDDIR/bin/$EXENAME
EXEDIR="$BUILDDIR/$EXENAME"
LIBS=
COPY="cp -u"

# MSYS
if files_exist /msys.bat; then

	LIBNAMES="`objdump -p ${BUILDEXEBASE}.exe | grep "DLL Name"`"
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

elif uname | grep -q Darwin; then
	LIBNAMES="`otool -L $BUILDEXEBASE | grep '/local/' | cut -d " " -f1`"
	#echo $LIBNAMES
	for lib in $LIBNAMES; do
		install_name_tool -change $lib @executable_path/${lib##*/} $BUILDEXEBASE
	done
	LIBS=$LIBNAMES
	COPY="cp"

else
	echo "Error: Could not build standalone. Platform not supported."
	exit
fi

mkdir -p $EXEDIR
$COPY $LIBS $EXEDIR
mv $BUILDEXEBASE $EXEDIR

echo "Created standalone in $EXEDIR/"
