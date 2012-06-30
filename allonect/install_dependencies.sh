#!/bin/bash

if [ `which apt-get` ]; then
	echo "Found apt-get"
#	sudo apt-get update
#	sudo apt-get install libfreenect-dev # no good; this version is too old
	sudo apt-get install libusb-1.0-0-dev libxmu-dev libxi-dev cmake
	# this needs to be run from the allocore root folder
	# grab libfreenect:
	git submodule init allonect/libfreenect && git submodule update allonect/libfreenect
	cd allonect/libfreenect
	cmake .
	make
	sudo make install
	cd ../
	echo You need to give freenect permissions:
	echo sudo cp 66-kinect.rules /etc/udev/rules.d/66-kinect.rules
	echo sudo adduser YOURNAME video
	echo and reboot

elif [ `which port` ]; then
	echo "Found MacPorts"
	sudo port selfupdate
#	port install freenect

elif [ `which brew` ]; then
	echo "Found Homebrew"
	cd /usr/local/Library/Formula
	curl --insecure -O "https://raw.github.com/OpenKinect/libfreenect/master/platform/osx/homebrew/libfreenect.rb"
	curl --insecure -O "https://raw.github.com/OpenKinect/libfreenect/master/platform/osx/homebrew/libusb-freenect.rb"
	brew install libfreenect

else
	echo "Error: No suitable package manager found."
	echo "Error: Install apt-get, MacPorts, or Homebrew and try again."
fi
