#!/bin/bash

if [ -f /opt/local/bin/port ]; then
 	echo "Found MacPorts"
	sudo port selfupdate
#	port install portaudio libsndfile

elif [ -f /usr/local/bin/brew ]; then
	echo "Found Homebrew"
	brew install portaudio libsndfile

else
	echo "Error: No suitable package manager found."
	echo "Error: Install either MacPorts or Homebrew and try again."
fi
