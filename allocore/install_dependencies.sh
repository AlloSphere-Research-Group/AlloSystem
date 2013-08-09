#!/bin/bash
# AlloCore dependencies install script

if [ `which apt-get 2>/dev/null` ]; then
	echo "Found apt-get"
	sudo apt-get update
	sudo apt-get install libapr1-dev libaprutil1-dev
	sudo apt-get install portaudio19-dev libsndfile1-dev
	sudo apt-get install libglew-dev freeglut3-dev 
	sudo apt-get install libavahi-client-dev	# for protocol/al_ZeroConf
	sudo apt-get install libudev-dev libusb-1.0-0-dev # for io/al_HID
	sudo apt-get install libfreeimage-dev libfreetype6-dev
	
	# Ensure that assimp3 headers are being installed.
	available_assimp_version=$(apt-cache madison libassimp-dev | head -1 | cut -f2 -d\|)
	assimp3_available=$(dpkg --compare-versions ${available_assimp_version} ge 3);

	if [[ $assimp3_available -eq $zero ]]; then
  		sudo apt-get install libassimp-dev
	# Otherwise build assimp3 from source and install.
	else
		pushd .

		TMP_DIR=$(mktemp -d)
		cd ${TMP_DIR}

		wget http://downloads.sourceforge.net/project/assimp/assimp-3.0/assimp--3.0.1270-s$
		# Only one dependency and it's a whopper!
		wget http://downloads.sourceforge.net/project/boost/boost/1.54.0/boost_1_54_0.tar.$

		unzip -qq assimp--3.0.1270-source-only.zip
		tar -zxf boost_1_54_0.tar.gz

		cd assimp--3.0.1270-source-only
		export BOOST_ROOT="../boost_1_54_0"
		cmake .
		make
		sudo make install

		unset BOOST_ROOT

		popd
	fi

elif [ `which port 2>/dev/null` ]; then
	echo "Found MacPorts"
	sudo port selfupdate
	sudo port install portaudio libsndfile +universal
	sudo port install glew +universal
	sudo port install assimp +universal
	sudo port install freeimage +universal
	sudo port install freetype +universal

elif [ `which brew 2>/dev/null` ]; then
	echo "Found Homebrew"
	sudo brew update
	brew install portaudio libsndfile
	brew install glew
	brew install assimp
	brew install freeimage
	brew install freetype

elif [ `uname | grep MINGW` ]; then
	echo "Found MinGW / MSYS"
	if [ ! `which wget` ]; then
		echo "wget not found. Install with 'mingw-get install msys-wget'."
		exit
	elif [ ! `which unzip` ]; then
		echo "unzip not found. Install with 'mingw-get install msys-unzip'."
		exit
	else
		DESTDIR=/usr/local/
		#DESTDIR=local/
		install -d $DESTDIR/bin/ $DESTDIR/include/ $DESTDIR/lib/
		
		LIBFILES=($DESTDIR/lib/libsndfile*)
		if [ -e ${LIBFILES[0]} ]; then
			echo "Found libsndfile"
		else
			PKG=libsndfile-1.0.25
			wget http://www.mega-nerd.com/libsndfile/files/$PKG.tar.gz
			tar -xzf $PKG.tar.gz
			pushd $PKG
				./configure --prefix=$DESTDIR
				make install -j3
			popd
			rm -rf $PKG
			rm $PKG.tar.gz
		fi

		LIBFILES=($DESTDIR/lib/libportaudio*)
		if [ -e ${LIBFILES[0]} ]; then
			echo "Found PortAudio"
		else
			PKG=pa_stable_v19_20111121
			wget http://www.portaudio.com/archives/$PKG.tgz
			tar -xzf $PKG.tgz
			mv portaudio $PKG
			pushd $PKG
				./configure --prefix=$DESTDIR
				make install -j3
			popd
			rm -rf $PKG
			rm $PKG.tgz
		fi

		LIBFILES=($DESTDIR/lib/libapr*)
		if [ -e ${LIBFILES[0]} ]; then
			echo "Found APR"
		else
			PKG=apr-1.3.6-iconv-1.2.1-util-1.3.8-win32-x86-msvcrt60
			wget http://mirrors.rackhosting.com/apache/apr/binaries/win32/$PKG.zip
			unzip -q $PKG.zip
			mv apr-dist $PKG
			cp $PKG/bin/libapr-1.dll		$DESTDIR/bin/
			cp $PKG/bin/libaprutil-1.dll	$DESTDIR/bin/
			cp $PKG/lib/libapr-1.lib		$DESTDIR/lib/libapr-1.dll.a
			cp $PKG/lib/libaprutil-1.lib	$DESTDIR/lib/libaprutil-1.dll.a
			install -d $DESTDIR/include/apr-1/
			cp -r $PKG/include/*			$DESTDIR/include/apr-1/
			rm -rf $PKG
			rm $PKG.zip
		fi
	
		LIBFILES=($DESTDIR/lib/*freetype*)
		if [ -e ${LIBFILES[0]} ]; then
			echo "Found FreeType"
		else
			PKG=freetype-dev_2.4.2-1_win32
			wget http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/$PKG.zip
			unzip -q $PKG.zip -d $PKG
			cp $PKG/lib/* $DESTDIR/lib/
			cp -r $PKG/include/* $DESTDIR/include/
			rm -rf $PKG
			rm $PKG.zip
		fi

		LIBFILES=($DESTDIR/lib/FreeImage*)
		if [ -e ${LIBFILES[0]} ]; then
			echo "Found FreeImage"
		else
			PKG=FreeImage3153Win32
			wget http://downloads.sourceforge.net/freeimage/$PKG.zip
			unzip -q $PKG.zip
			mv FreeImage $PKG
			cp $PKG/Dist/FreeImage.dll $DESTDIR/bin/
			cp $PKG/Dist/FreeImage.lib $DESTDIR/lib/
			cp $PKG/Dist/FreeImage.h $DESTDIR/include/
			rm -rf $PKG
			rm $PKG.zip
		fi

		LIBFILES=($DESTDIR/lib/assimp*)
		if [ -e ${LIBFILES[0]} ]; then
			echo "Found AssImp"
		else
			PKG=assimp--2.0.863-sdk
			wget http://downloads.sourceforge.net/project/assimp/assimp-2.0/$PKG.zip
			unzip -q $PKG.zip
			install -d $DESTDIR/include/assimp/Compiler/
			cp -r $PKG/include/* $DESTDIR/include/assimp/
			cp $PKG/bin/assimp_release-dll_win32/Assimp32.dll $DESTDIR/bin/
			cp $PKG/lib/assimp_release-dll_win32/assimp.lib $DESTDIR/lib/
			rm -rf $PKG
			rm $PKG.zip
		fi

		LIBFILES=($DESTDIR/lib/libglew32*)
		if [ -e ${LIBFILES[0]} ]; then
			echo "Found GLEW"
		else
			# These MSVC binaries don't work with MinGW/Msys :(
			#PKG=glew-1.9.0-win32
			#wget http://downloads.sourceforge.net/project/glew/glew/1.9.0/$PKG.zip
			#unzip $PKG.zip
			#mv glew-1.9.0 $PKG
			#cp $PKG/bin/*.dll $DESTDIR/bin/
			#cp $PKG/lib/*.lib $DESTDIR/lib/
			#cp -r $PKG/include/* $DESTDIR/include/
			#rm -rf $PKG
			#rm $PKG.zip
	
			PKG=glew-1.9.0
			wget http://downloads.sourceforge.net/project/glew/glew/1.9.0/$PKG.zip
			unzip -q $PKG.zip
			pushd $PKG
				make install GLEW_DEST=/usr/local/ -j3
			popd
			rm -rf $PKG
			rm $PKG.zip
		fi

		LIBFILES=($DESTDIR/lib/glut32*)
		if [ -e ${LIBFILES[0]} ]; then
			echo "Found GLUT"
		else
			# This site always seems to be down...
			#PKG=glut-3.7.6-bin
			#wget http://user.xmission.com/~nate/glut/$PKG.zip
			#unzip -q $PKG.zip
			#install -d $DESTDIR/include/GL/
			#cp $PKG/glut.h $DESTDIR/include/GL/
			#cp $PKG/glut32.dll $DESTDIR/bin/
			#cp $PKG/glut32.lib $DESTDIR/lib/
			#rm -rf $PKG
			#rm $PKG.zip
	
			PKG=glutdlls37beta
			wget http://www.opengl.org/resources/libraries/glut/$PKG.zip
			unzip -q $PKG.zip -d $PKG
			install -d $DESTDIR/include/GL/
			cp $PKG/glut.h $DESTDIR/include/GL/
			cp $PKG/glut32.dll $DESTDIR/bin/
			cp $PKG/glut32.lib $DESTDIR/lib/
			rm -rf $PKG
			rm $PKG.zip
		fi
	fi

else
	echo "Error: No suitable package manager found."
	echo "Error: Install apt-get, MacPorts, or Homebrew and try again."
fi
