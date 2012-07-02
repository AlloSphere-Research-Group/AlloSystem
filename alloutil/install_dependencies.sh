#!/bin/bash
# AlloUtil dependencies install script

ROOT=`pwd`
PLATFORM=`uname`
ARCH=`uname -m`
echo Installing for $PLATFORM $ARCH from $ROOT 

if [ `which apt-get` ]; then
	echo "Found apt-get"
	sudo apt-get update
	
elif [ `which port` ]; then
	echo "Found MacPorts"
	sudo port selfupdate 
	port install lua

elif [ `which brew` ]; then
	echo "Found Homebrew"
	brew install lua

else
	echo "Error: No suitable package manager found."
	echo "Error: Install apt-get, MacPorts, or Homebrew and try again."
fi

