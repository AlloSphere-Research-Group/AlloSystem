#!/bin/sh

if [ $# -eq 0 ]; then
	echo "Usage: ./$(basename "$0") [FILE]"
	exit
fi

files_exist(){
	ls -u "$@" >/dev/null 2>&1;
}

BUILDDIR="build"
EXENAME=$(basename "$1" | cut -d. -f1)
BUILDEXEBASE=$BUILDDIR/bin/$EXENAME
EXEDIR="$BUILDDIR/$EXENAME"
LIBS=
COPY="cp -u"

# Build executable first as we may need it to check for library dependencies
./run.sh $* AUTORUN=0

# Check if executable built
files_exist ${BUILDEXEBASE}* || {
	echo "No standalone created: failed to build $1"
	exit
}

# MinGW
if uname | grep -q MINGW; then

	if files_exist /msys.bat; then
		LIBDIRS="/mingw/bin/ /usr/local/bin/"
	elif uname | grep -q MINGW64; then
		LIBDIRS="/mingw64/bin/ /usr/bin/"
	elif uname | grep -q MINGW32; then
		LIBDIRS="/mingw32/bin/ /usr/bin/"
	fi

	getDeps(){
		LIBNAMES="`objdump -p $1 | grep "DLL Name"`"
		LIBNAMES="${LIBNAMES//DLL Name: /}"
		LIBPATHS=
		#echo $LIBNAMES
		for dir in $LIBDIRS; do
			for lib in $LIBNAMES; do
				DIR_LIB=$dir$lib
				if files_exist $DIR_LIB; then
					LIBPATHS+=" $DIR_LIB";
					# Recurse to handle evil dependent dependencies
					NEWLIBS="`getDeps $DIR_LIB`"
					#echo $NEWLIBS
					for newlib in $NEWLIBS; do
						if [[ $LIBPATHS != *"$newlib"* ]]; then
							LIBPATHS+=" $newlib"
						fi
					done
				fi

			done
		done
		echo $LIBPATHS
	}

	LIBS=`getDeps "${BUILDEXEBASE}.exe"`
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
