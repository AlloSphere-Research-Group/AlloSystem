#!/bin/sh

# Helper functions.
binary_exists(){
	which "$1" >/dev/null 2>&1;
}
# End helper functions.

if (binary_exists apt-get); then
	echo 'Found apt-get'
#	sudo apt-get update
	sudo apt-get install libfreenect-dev
	sudo apt-get install libusb-1.0-0-dev libxmu-dev libxi-dev
	# Saucy's libfreenect-dev should be new enough now.
	# this needs to be run from the allocore root folder
	# grab libfreenect:
	# sudo apt-get install cmake
	# DIR=$PWD
	# cd /tmp
	# git submodule init allonect/libfreenect && git submodule update allonect/libfreenect
	# cd allonect/libfreenect
	# cmake .
	# make
	# sudo make install
	# cd $DIR
	# echo You need to give freenect permissions:
	# echo sudo cp 66-kinect.rules /etc/udev/rules.d/66-kinect.rules
	# echo sudo adduser YOURNAME video
	# echo and reboot

elif (binary_exists brew); then
	echo 'Found Homebrew'
	DIR=$PWD
	cd /usr/local/Library/Formula
		curl --insecure -O "https://raw.github.com/OpenKinect/libfreenect/master/platform/osx/homebrew/libfreenect.rb"
		curl --insecure -O "https://raw.github.com/OpenKinect/libfreenect/master/platform/osx/homebrew/libusb-freenect.rb"
		brew install libfreenect
	cd "$DIR"

elif (binary_exists port); then
	echo 'Tell us if a port appears in MacPorts'
	sudo port selfupdate
	port search freenect
#	port install freenect

else
	echo 'Error: No suitable package manager found.'
	echo 'Error: Install apt-get, MacPorts, or Homebrew and try again.'
fi
