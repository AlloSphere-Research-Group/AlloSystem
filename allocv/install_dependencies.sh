#!/bin/sh
# AlloCV (AlloCore / OpenCV binding) dependencies install script

# Helper functions.
binary_exists(){
	command -v "$1" >/dev/null 2>&1;
}

files_exist(){
	ls -u "$@" >/dev/null 2>&1;
}
# End helper functions.

# Linux / APT
if binary_exists "apt-get"; then
	echo 'Found apt-get'
	sudo apt-get update
	sudo apt-get install libopencv-dev

# OSX / Homebrew
elif binary_exists "brew"; then
	echo 'Found Homebrew'
	echo 'Error: Installation procedure is not yet implemented'
	exit
	#brew update
	#brew install pkgconfig

# OSX / MacPorts
elif binary_exists "port"; then
	echo 'Found MacPorts'
	sudo port selfupdate
	sudo port install pkgconfig 
	sudo port install opencv +openni

# Windows / MinGW
elif uname | grep "MINGW"; then
	echo 'Found MinGW / MSYS'

	echo 'Error: Installation procedure is not yet implemented'
	exit

	if ! binary_exists "wget"; then
		echo "wget not found. Install with 'mingw-get install msys-wget'."
		exit
	elif ! binary_exists "unzip"; then
		echo "unzip not found. Install with 'mingw-get install msys-unzip'."
		exit
	else
		DESTDIR=/usr/local/
		install -d "${DESTDIR}/bin/" "${DESTDIR}/include/" "${DESTDIR}/lib/"

		if files_exist "$DESTDIR/lib/opencv*"; then
			echo 'Found opencv'
		else
			# 2.4.7 does not have MinGW libs
			#PKG=OpenCV-2.4.7
			#wget http://downloads.sourceforge.net/project/opencvlibrary/opencv-win/2.4.7/$PKG.exe
			#cp $PKG/build/x86/vc11/bin/*[^d].dll $DESTDIR/bin/
			#cp $PKG/build/x86/vc11/lib/*[^d].lib $DESTDIR/lib/

			PKG=OpenCV-2.4.6.0
			DIR="$PWD"
			cd /tmp
				wget "http://downloads.sourceforge.net/project/opencvlibrary/opencv-win/2.4.6/${PKG}.exe"
				# extracts to opencv/
				./$PKG.exe
				mv opencv "$PKG"
				cp "${PKG}/build/x86/mingw/bin/*[!d].dll" "${DESTDIR}/bin/"
				cp "${PKG}/build/x86/mingw/lib/*[!d].dll.a" "${DESTDIR}/lib/"
				cp -r "${PKG}/build/include/*" "${DESTDIR}/include/"

				# Cleanup.
				rm -rf "$PKG"
				rm "${PKG}.exe"
			cd "$DIR"
		fi
	fi

else
	echo 'Error: No suitable package manager found.'
	echo 'Error: Install apt-get, MacPorts, or Homebrew and try again.'
fi
