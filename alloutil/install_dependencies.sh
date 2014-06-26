#!/bin/sh
# AlloUtil dependencies install script

# Helper functions.
binary_exists(){
	which "$1" >/dev/null 2>&1;
}
# End helper functions.

ROOT=$(pwd)
PLATFORM=$(uname)
ARCH=$(uname -m)
echo "Installing for ${PLATFORM} ${ARCH} from ${ROOT}"

if (binary_exists apt-get); then
	echo 'Found apt-get'
	sudo apt-get update
	sudo apt-get install liblua5.1-dev

elif (binary_exists brew); then
	echo 'Found Homebrew'
	brew update
	brew install lua

elif (binary_exists port); then
	echo 'Found MacPorts'
	sudo port selfupdate
	sudo port install luajit

elif (uname | grep MINGW); then
	echo 'Found MinGW / MSYS'
	if ! (binary_exists wget); then
		echo "wget not found. Install with 'mingw-get install msys-wget'."
		exit
	elif ! (binary_exists unzip); then
		echo "unzip not found. Install with 'mingw-get install msys-unzip'."
		exit
	else
		DESTDIR=/usr/local/
		#DESTDIR=local/
		install -d $DESTDIR/bin/ $DESTDIR/include/ $DESTDIR/lib/

		PKG=lua5_1_4_Win32_mingw4_lib
		DIR=$PWD
		cd /tmp
			wget http://downloads.sourceforge.net/project/luabinaries/5.1.4/Windows%20Libraries/$PKG.zip
			#PKG=lua-5.2.1_Win32_mingw4_lib
			#wget http://downloads.sourceforge.net/project/luabinaries/5.2.1/Windows%20Libraries/Static/$PKG.zip
			unzip $PKG.zip -d $PKG
			cp $PKG/*.a $DESTDIR/lib/
			cp -r $PKG/include/* $DESTDIR/include/

			# Cleanup.
			rm -rf $PKG
			rm $PKG.zip
		cd "$DIR"
	fi

else
	echo 'Error: No suitable package manager found.'
	echo 'Error: Install apt-get, MacPorts, or Homebrew and try again.'
fi
