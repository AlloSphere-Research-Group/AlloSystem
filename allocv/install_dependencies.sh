#!/bin/bash
# AlloCV (AlloCore / OpenCV binding) dependencies install script

# Linux / APT
if [ `which apt-get 2>/dev/null` ]; then
	echo "Found apt-get"
	sudo apt-get update
	sudo apt-get install libopencv-dev

# OSX / MacPorts
elif [ `which port 2>/dev/null` ]; then
	echo "Found MacPorts"
	sudo port selfupdate 
	sudo port install pkgconfig opencv

# OSX / Homebrew
elif [ `which brew 2>/dev/null` ]; then
	echo "Found Homebrew"
	echo "Error: Installation procedure is not yet implemented"
	exit
	#brew update
	#brew install pkgconfig

# Windows / MinGW
elif [ `uname | grep MINGW` ]; then
	echo "Found MinGW / MSYS"

	echo "Error: Installation procedure is not yet implemented"
	exit

	if [ ! `which wget` ]; then
		echo "wget not found. Install with 'mingw-get install msys-wget'."
		exit
	elif [ ! `which unzip` ]; then
		echo "unzip not found. Install with 'mingw-get install msys-unzip'."
		exit
	else
		DESTDIR=/usr/local/
		install -d $DESTDIR/bin/ $DESTDIR/include/ $DESTDIR/lib/

		LIBFILES=($DESTDIR/lib/opencv*)
		if [ -e ${LIBFILES[0]} ]; then
			echo "Found opencv"
		else
			# 2.4.7 does not have MinGW libs
			#PKG=OpenCV-2.4.7
			#wget http://downloads.sourceforge.net/project/opencvlibrary/opencv-win/2.4.7/$PKG.exe
			#cp $PKG/build/x86/vc11/bin/*[^d].dll $DESTDIR/bin/
			#cp $PKG/build/x86/vc11/lib/*[^d].lib $DESTDIR/lib/

			PKG=OpenCV-2.4.6.0
			wget http://downloads.sourceforge.net/project/opencvlibrary/opencv-win/2.4.6/$PKG.exe
			# extracts to opencv/
			./$PKG.exe
			mv opencv $PKG
			cp $PKG/build/x86/mingw/bin/*[^d].dll $DESTDIR/bin/
			cp $PKG/build/x86/mingw/lib/*[^d].dll.a $DESTDIR/lib/
			cp -r $PKG/build/include/* $DESTDIR/include/
			rm -rf $PKG
			rm $PKG.exe
		fi
	fi

else
	echo "Error: No suitable package manager found."
	echo "Error: Install apt-get, MacPorts, or Homebrew and try again."
fi

