#!/bin/sh

# Helper functions.
binary_exists(){
	command -v "$1" >/dev/null 2>&1;
}
# End helper functions.

if binary_exists "apt-get"; then
	echo 'Found apt-get'
	sudo apt-get update
	# Saucy's libfreenect-dev should be new enough now. - 9/2014
	sudo apt-get install libfreenect-dev
	sudo apt-get install libusb-1.0-0-dev libxmu-dev libxi-dev

elif binary_exists "brew"; then
	echo 'Found Homebrew'
	DIR="$PWD"
	cd /usr/local/Library/Formula
		curl --insecure -O "https://raw.github.com/OpenKinect/libfreenect/master/platform/osx/homebrew/libfreenect.rb"
		curl --insecure -O "https://raw.github.com/OpenKinect/libfreenect/master/platform/osx/homebrew/libusb-freenect.rb"
		brew install libfreenect
	cd "$DIR"

elif binary_exists "port"; then
	echo 'Tell us if a port appears in MacPorts'
	sudo port selfupdate
	port search freenect
#	port install freenect

else
	echo 'Error: No suitable package manager found.'
	echo 'Error: Install apt-get, MacPorts, or Homebrew and try again.'
fi
