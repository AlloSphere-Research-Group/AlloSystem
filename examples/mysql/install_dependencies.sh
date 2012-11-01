#!/bin/bash

if [ `which apt-get` ]; then
	echo "Found apt-get"
	sudo apt-get update
	sudo apt-get install libmysql++-dev
  echo "-I/usr/include/mysql -lmysqlpp" > flags.txt

elif [ `which port` ]; then
	echo "Found MacPorts"
	sudo port selfupdate
	sudo port install mysqlxx +universal
  echo "-L/opt/local/lib -I/opt/local/include -lmysqlpp" > flags.txt

elif [ `which brew` ]; then
	echo "Found Homebrew"
	brew update
	brew install mysql++
  echo "-L/usr/local/lib -I/usr/local/include -lmysqlpp" > flags.txt

else
	echo "Error: No suitable package manager found."
	echo "Error: Install apt-get, MacPorts, or Homebrew and try again."
fi
