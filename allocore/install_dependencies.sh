#!/bin/sh

# AlloCore dependencies install script

# Helper functions.
binary_exists(){
	command -v "$1" >/dev/null 2>&1;
}

files_exist(){
	ls -u "$@" >/dev/null 2>&1;
}

# Find package manager and install dependencies

if binary_exists "apt-get"; then
	echo 'Found apt-get'
	sudo apt-get update
	sudo apt-get install \
		build-essential \
		libglew-dev freeglut3-dev \
		libavahi-client-dev \
		libbluetooth-dev \
		libudev-dev \
		libxi-dev libxmu-dev

# It's important to check for Homebrew before MacPorts,
# because Homebrew is the most used among the AlloTeam.
elif binary_exists "brew"; then
	echo 'Found Homebrew'
	brew update
	brew install glew

elif binary_exists "port"; then
	echo 'Found MacPorts'
	sudo port selfupdate
	sudo port install glew

elif uname -o | grep -q "Msys"; then

	# MSYS
	if files_exist /msys.bat; then
		echo 'Found MinGW / MSYS'
		if ! binary_exists "wget"; then
			echo "wget not found. Install with 'mingw-get install msys-wget'."
			exit
		fi
		if ! binary_exists "unzip"; then
			echo "unzip not found. Install with 'mingw-get install msys-unzip'."
			exit
		fi
		DESTDIR=/usr/local/
		#DESTDIR=local/
		DOWNLOAD="wget --no-check-certificate"
		install -d $DESTDIR/bin/ $DESTDIR/include/ $DESTDIR/lib/

		if files_exist $DESTDIR/bin/bthprops.dll; then
			echo 'Found Bluetooth'
		else
			# Windows Bluetooth lib is disguised as control panel item (!?).
			# We just make a local copy to avoid disturbing Windows...
			cp /c/Windows/System32/bthprops.cpl $DESTDIR/bin/bthprops.dll
			#cp win32/*.h $DESTDIR/include/
		fi

		if files_exist $DESTDIR/lib/libglew32*; then
			echo 'Found GLEW'
		else
			# These MSVC binaries don't work with MinGW/Msys :(
			#PKG=glew-1.9.0-win32
			#$DOWNLOAD http://downloads.sourceforge.net/project/glew/glew/1.9.0/$PKG.zip
			#unzip $PKG.zip
			#mv glew-1.9.0 $PKG
			#cp $PKG/bin/*.dll $DESTDIR/bin/
			#cp $PKG/lib/*.lib $DESTDIR/lib/
			#cp -r $PKG/include/* $DESTDIR/include/
			GLEW_VERSION=1.13.0
			PKG=glew-$GLEW_VERSION
			DIR=$PWD
			cd /tmp
				$DOWNLOAD http://downloads.sourceforge.net/project/glew/glew/$GLEW_VERSION/$PKG.zip
				unzip -q $PKG
				cd $PKG
					make install GLEW_DEST=$DESTDIR
				cd ..

				# Cleanup.
				rm -rf $PKG
				rm $PKG.*
			cd $DIR
		fi

		if files_exist $DESTDIR/lib/glut32*; then
			echo 'Found GLUT'
		else
			PKG=glutdlls37beta
			DIR=$PWD
			cd /tmp
				$DOWNLOAD https://www.opengl.org/resources/libraries/glut/$PKG.zip
				unzip -q $PKG -d $PKG
				install -d $DESTDIR/include/GL/
				cp $PKG/glut.h $DESTDIR/include/GL/
				cp $PKG/glut32.dll $DESTDIR/bin/
				cp $PKG/glut32.lib $DESTDIR/lib/

				# Cleanup.
				rm -rf $PKG
				rm $PKG.*
			cd $DIR
		fi

	# MSYS2
	else
		echo 'Found MinGW-w64 / MSYS2'
		LIBS="gcc gdb glew freeglut libusb"
		PKGS=
		for L in $LIBS
		do
			PKGS=$PKGS" $MINGW_PACKAGE_PREFIX-$L"
		done

		#echo $PKGS
		pacman -Syu
		pacman -S make $PKGS
	fi
else
	echo 'Error: No suitable package manager found.'
	echo 'Error: Install apt-get, MacPorts, or Homebrew and try again.'
fi
