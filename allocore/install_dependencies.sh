#!/bin/bash

if [ `which apt-get` ]; then
	echo "Found apt-get"

elif [ `which port` ]; then
	echo "Found MacPorts"
#	sudo port selfupdate
#	port install assimp freeimage 
#	port install portaudio libsndfile

elif [ `which brew` ]; then
	echo "Found Homebrew"
#	brew install portaudio libsndfile

else
	echo "Error: No suitable package manager found."
	echo "Error: Install apt-get, MacPorts, or Homebrew and try again."
fi
