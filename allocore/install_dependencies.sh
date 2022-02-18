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
		portaudio19-dev \
		libglew-dev freeglut3-dev \
		libavahi-client-dev \
		libbluetooth-dev \
		libudev-dev \
		libfreetype6-dev \
		libxi-dev libxmu-dev

# It's important to check for Homebrew before MacPorts,
# because Homebrew is the most used among the AlloTeam.
elif binary_exists "brew"; then
	echo 'Found Homebrew'
	brew update
	brew install portaudio glew freetype

elif binary_exists "port"; then
	echo 'Found MacPorts'
	sudo port selfupdate
	sudo port install portaudio glew freetype

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

		if files_exist $DESTDIR/lib/libportaudio*; then
			echo 'Found PortAudio'
		else
			PKG=pa_stable_v19_20140130
			DIR=$PWD
			cd /tmp
				$DOWNLOAD http://www.portaudio.com/archives/$PKG.tgz
				tar -xzf $PKG.tgz
				mv portaudio $PKG
				
				DXDIR=/usr/local/dx7sdk/include
				DXURL=https://raw.githubusercontent.com/msys2-contrib/mingw-w64/master/mingw-w64-headers

				install -d $DXDIR
				$DOWNLOAD $DXURL/direct-x/include/dsound.h -O $DXDIR/dsound.h
				$DOWNLOAD $DXURL/direct-x/include/dsconf.h -O $DXDIR/dsconf.h
				$DOWNLOAD $DXURL/crt/_mingw_unicode.h -O $DXDIR/_mingw_unicode.h
				
				cd $PKG
					# MME may artificially cap channels at 2! WDMKS or DirectX is needed for multi-channel.
					# While WDMKS is supposedly superior to DirectX, it doesn't always give us all the devices, so we just use DirectX.
					./configure --prefix=$DESTDIR --with-winapi=wmme,directx
					#./configure --prefix=$DESTDIR --with-winapi=wmme,wdmks
					make install
				cd ..

				# DX7 headers only needed to build PA, so we can remove them
				rm -rf $DXDIR/..
				
				# Cleanup.
				rm -rf $PKG
				rm $PKG.*
			cd $DIR
		fi

		if files_exist $DESTDIR/bin/bthprops.dll; then
			echo 'Found Bluetooth'
		else
			# Windows Bluetooth lib is disguised as control panel item (!?).
			# We just make a local copy to avoid disturbing Windows...
			cp /c/Windows/System32/bthprops.cpl $DESTDIR/bin/bthprops.dll
			#cp win32/*.h $DESTDIR/include/
		fi

		if files_exist $DESTDIR/lib/*freetype*; then
			echo 'Found FreeType'
		else
			PKG=freetype-2.3.5-1-bin
			DIR=$PWD
			cd /tmp
				$DOWNLOAD http://downloads.sourceforge.net/project/gnuwin32/freetype/2.3.5-1/$PKG.zip
				unzip -q $PKG -d $PKG
				cp $PKG/bin/freetype*.dll $DESTDIR/bin/
				cp $PKG/lib/libfreetype.dll.a $DESTDIR/lib/
				cp $PKG/lib/freetype*.def $DESTDIR/lib/
				cp $PKG/lib/freetype.lib $DESTDIR/lib/
				cp -r $PKG/include/* $DESTDIR/include/

				# Cleanup.
				rm -rf $PKG
				rm $PKG.*
			cd $DIR
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
		LIBS="gcc gdb portaudio glew freeglut freetype libusb"
		PKGS=
		for L in $LIBS
		do
			PKGS=$PKGS" $MINGW_PACKAGE_PREFIX-$L"
		done

		# Libs with deps (other then gcc-libs):
		# freetype: bzip2 harfbuzz libpng zlib

		#echo $PKGS
		pacman -Syu
		pacman -S make $PKGS
	fi
else
	echo 'Error: No suitable package manager found.'
	echo 'Error: Install apt-get, MacPorts, or Homebrew and try again.'
fi
