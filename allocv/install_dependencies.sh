#!/bin/bash
# AlloCV (AlloCore / OpenCV binding) dependencies install script

# Linux / APT
if [ `which apt-get` ]; then
	echo "Found apt-get"
	sudo apt-get update
	sudo apt-get install libopencv-dev

# OSX / MacPorts
elif [ `which port` ]; then
	echo "Found MacPorts"
	sudo port selfupdate 
	sudo port install opencv

# OSX / Homebrew
elif [ `which brew` ]; then
	echo "Found Homebrew"
	echo "Error: Installation procedure is not yet implemented"
	exit
	#brew update
	#brew install ?

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

		#PKG=lua5_1_4_Win32_mingw4_lib
		#wget http://downloads.sourceforge.net/project/luabinaries/5.1.4/Windows%20Libraries/$PKG.zip
		#unzip $PKG.zip -d $PKG
		#cp $PKG/*.a $DESTDIR/lib/
		#cp -r $PKG/include/* $DESTDIR/include/
		#rm -rf $PKG
		#rm $PKG.zip
	fi

else
	echo "Error: No suitable package manager found."
	echo "Error: Install apt-get, MacPorts, or Homebrew and try again."
fi

