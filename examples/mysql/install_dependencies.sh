#!/bin/sh

 # AlloSystem mysql example dependencies install script

# Helper functions.
binary_exists(){
	which "$1" >/dev/null 2>&1;
}
# End helper functions.

# Put flags.txt next to the mysql example.
MYSQL_DIR=$( cd "$( dirname "$0" )" && pwd )

if (binary_exists apt-get); then
	echo 'Found apt-get'
	sudo apt-get update
	sudo apt-get install libmysql++-dev
  echo '-I/usr/include/mysql -lmysqlpp' > "${MYSQL_DIR}/flags.txt"

elif (binary_exists brew); then
	echo 'Found Homebrew'
	brew update
	brew install mysql++
  echo '-L/usr/local/lib -I/usr/local/include -lmysqlpp' > "${MYSQL_DIR}/flags.txt"

elif (binary_exists port); then
	echo 'Found MacPorts'
	sudo port selfupdate
	sudo port install mysqlxx +universal
  echo '-L/opt/local/lib -I/opt/local/include -lmysqlpp' > "${MYSQL_DIR}/flags.txt"

else
	echo 'Error: No suitable package manager found.'
	echo 'Error: Install apt-get, MacPorts, or Homebrew and try again.'
fi
