#!/bin/sh

# AlloCore dependencies install script

# Helper functions.
binary_exists(){
	command -v "$1" >/dev/null 2>&1;
}

files_exist(){
	ls -u "$@" >/dev/null 2>&1;
}

# Builds without requiring boost.
build_and_install_assimp(){
	DIR="$PWD"
	cd /tmp
		PKG="assimp-3.1.1"
		curl -LO "http://downloads.sourceforge.net/project/assimp/assimp-3.1/${PKG}_no_test_models.zip"
		unzip -q "${PKG}*.zip"
		cd "$PKG"
			cmake -DENABLE_BOOST_WORKAROUND=ON .
			make
			sudo make install
		cd ..

		# Cleanup.
		rm -rf /tmp/${PKG}
		rm /tmp/${PKG}*.zip
	cd "$DIR"
}
# End helper functions.

if binary_exists "apt-get"; then
	echo 'Found apt-get'
	sudo apt-get update
	sudo apt-get install build-essential \
						 cmake \
						 libapr1-dev libaprutil1-dev \
						 portaudio19-dev libsndfile1-dev \
						 libglew-dev freeglut3-dev \
						 libavahi-client-dev \
						 libbluetooth-dev \
						 libudev-dev \
						 libfreeimage-dev libfreetype6-dev \
						 libxi-dev libxmu-dev

	# Get version of installed assimp
	assimp_version=$(apt-cache policy libassimp-dev | grep Installed | cut -f2 -d: | sed -e 's/[ ]*//')

	# If assimp2 is installed, then prompt to remove it.
	if dpkg --compare-versions "$assimp_version" lt 3; then
		echo "assimp version ${assimp_version} detected."
		echo 'It is recommended that you remove this version to avoid configuration problems with AlloCore.'
		printf 'Would you like to remove it [Y/n]?'
		read ANS
		if [ "$ANS" = 'Y' ] || [ "$ANS" = 'y' ]; then
			echo "Removing assimp ${assimp_version}"
			sudo apt-get remove "libassimp-dev=${assimp_version}"
		fi
	fi

	# Ensure that assimp3 headers are being installed.
	available_assimp_version=$(apt-cache madison libassimp-dev | head -1 | cut -f2 -d\|)

	if dpkg --compare-versions "$available_assimp_version" ge 3; then
		sudo apt-get install libassimp-dev
	# Otherwise build assimp3 from source and install.
	else
		build_and_install_assimp
	fi

# It's important to check for Homebrew before MacPorts,
# because Homebrew is the most used among the AlloTeam.
elif binary_exists "brew"; then
	echo 'Found Homebrew'
	brew update
	brew install cmake \
				 portaudio libsndfile \
				 glew \
				 freeimage \
				 freetype

	# Use precompiled library if on Mountain Lion or higher.
	osx_version="$(sw_vers -productVersion | cut -d . -f 2)"
	if [ "$osx_version" -ge 8 ]; then
		brew install assimp
	# Otherwise build with boost workaround.
	else
		brew install assimp --without-boost
	fi

elif binary_exists "port"; then
	echo 'Found MacPorts'
	sudo port selfupdate
	sudo port install cmake \
					  portaudio libsndfile \
					  glew \
					  freeimage \
					  freetype

	build_and_install_assimp

elif uname | grep "MINGW"; then
	echo 'Found MinGW / MSYS'
	if ! binary_exists "wget"; then
		echo "wget not found. Install with 'mingw-get install msys-wget'."
		exit
	elif ! binary_exists "unzip"; then
		echo "unzip not found. Install with 'mingw-get install msys-unzip'."
		exit
	else
		DESTDIR=/usr/local/
		#DESTDIR=local/
		install -d $DESTDIR/bin/ $DESTDIR/include/ $DESTDIR/lib/

		if files_exist $DESTDIR/lib/libsndfile*; then
			echo 'Found libsndfile'
		else
			PKG=libsndfile-1.0.25
			DIR=$PWD
			cd /tmp
				wget http://www.mega-nerd.com/libsndfile/files/$PKG.tar.gz
				tar -xzf $PKG.tar.gz

				cd $PKG
					./configure --prefix=$DESTDIR --disable-external-libs
					# make from src/ to avoid building tests and other gunk
					cd src
						make install
					cd ..
				cd ..

				# Cleanup.
				rm -rf $PKG
				rm $PKG.*
			cd $DIR
		fi

		if files_exist $DESTDIR/lib/libportaudio*; then
			echo 'Found PortAudio'
		else
			PKG=pa_stable_v19_20111121
			DIR=$PWD
			cd /tmp
				wget http://www.portaudio.com/archives/$PKG.tgz
				tar -xzf $PKG.tgz
				mv portaudio $PKG
				cd $PKG
					./configure --prefix=$DESTDIR
					make install
				cd ..

				# Cleanup.
				rm -rf $PKG
				rm $PKG.*
			cd $DIR
		fi

		if files_exist $DESTDIR/lib/libapr*; then
			echo 'Found APR'
		else
			#http://www.apachelounge.com/download/win32/binaries/httpd-2.4.7-win32.zip
			#http://www.apachelounge.com/download/VC11/binaries/httpd-2.4.7-win32-VC11.zip
			PKG=apr-1.3.6-iconv-1.2.1-util-1.3.8-win32-x86-msvcrt60
			DIR=$PWD
			cd /tmp
				#wget http://mirrors.rackhosting.com/apache/apr/binaries/win32/$PKG.zip
				#wget http://www.powertech.no/apache/dist/apr/binaries/win32/$PKG.zip
				wget http://archive.apache.org/dist/apr/binaries/win32/$PKG.zip
				unzip -q $PKG
				mv apr-dist "$PKG"
				cp $PKG/bin/libapr-1.dll		$DESTDIR/bin/
				cp $PKG/bin/libaprutil-1.dll	$DESTDIR/bin/
				cp $PKG/lib/libapr-1.lib		$DESTDIR/lib/libapr-1.dll.a
				cp $PKG/lib/libaprutil-1.lib	$DESTDIR/lib/libaprutil-1.dll.a
				install -d $DESTDIR/include/apr-1/
				cp -r $PKG/include/*			$DESTDIR/include/apr-1/

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
			PKG=freetype-dev_2.4.2-1_win32
			DIR=$PWD
			cd /tmp
				wget http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/$PKG.zip
				unzip -q $PKG -d $PKG
				cp $PKG/lib/* $DESTDIR/lib/
				cp -r $PKG/include/* $DESTDIR/include/

				# Cleanup.
				rm -rf $PKG
				rm $PKG.*
			cd $DIR
		fi

		if files_exist $DESTDIR/lib/FreeImage*; then
			echo 'Found FreeImage'
		else
			PKG=FreeImage3160Win32
			DIR=$PWD
			cd /tmp
				wget http://downloads.sourceforge.net/project/freeimage/Binary%20Distribution/3.16.0/$PKG.zip
				unzip -q $PKG
				mv FreeImage $PKG
				cp $PKG/Dist/FreeImage.dll $DESTDIR/bin/
				cp $PKG/Dist/FreeImage.lib $DESTDIR/lib/
				cp $PKG/Dist/FreeImage.h $DESTDIR/include/

				# Cleanup.
				rm -rf $PKG
				rm $PKG.*
			cd $DIR
		fi

		# Remove assimp2 if found
		if [ -e $DESTDIR/include/assimp/assimp.h ]; then
			echo 'Found AssImp2. This will be removed to update to AssImp3...'
			rm -rf $DESTDIR/include/assimp
			rm $DESTDIR/bin/Assimp32.dll
			rm $DESTDIR/lib/assimp.lib
		fi

		if files_exist $DESTDIR/lib/assimp*; then
			echo 'Found AssImp'
		else
			PKG=assimp--3.0.1270-full
			#PKG=assimp-3.1.1-win-binaries
			DIR=$PWD
			cd /tmp
				wget -nc http://downloads.sourceforge.net/project/assimp/assimp-3.0/$PKG.zip
				#wget -nc http://downloads.sourceforge.net/project/assimp/assimp-3.1/$PKG.zip
				unzip -q $PKG

				# 3.0.1270
				mv assimp--3.0.1270-sdk $PKG
				cp -r $PKG/include/* $DESTDIR/include/
				cp $PKG/bin/assimp_release-dll_win32/Assimp32.dll $DESTDIR/bin/
				cp $PKG/lib/assimp_release-dll_win32/assimp.lib $DESTDIR/lib/

				# 3.1.1
				#cp -r $PKG/include/* $DESTDIR/include/
				#cp $PKG/bin32/assimp.dll $DESTDIR/bin/
				#cp $PKG/lib32/assimp.lib $DESTDIR/lib/

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
			#wget http://downloads.sourceforge.net/project/glew/glew/1.9.0/$PKG.zip
			#unzip $PKG.zip
			#mv glew-1.9.0 $PKG
			#cp $PKG/bin/*.dll $DESTDIR/bin/
			#cp $PKG/lib/*.lib $DESTDIR/lib/
			#cp -r $PKG/include/* $DESTDIR/include/

			PKG=glew-1.9.0
			DIR=$PWD
			cd /tmp
				wget http://downloads.sourceforge.net/project/glew/glew/1.9.0/$PKG.zip
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
				wget --no-check-certificate https://www.opengl.org/resources/libraries/glut/$PKG.zip
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
	fi

else
	echo 'Error: No suitable package manager found.'
	echo 'Error: Install apt-get, MacPorts, or Homebrew and try again.'
fi
