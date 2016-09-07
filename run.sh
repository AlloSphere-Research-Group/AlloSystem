#!/bin/sh

if [ $# -eq 0 ]; then
	echo "Usage: ./$(basename "$0") [SOURCE FILE]"
	exit
fi

make $* RUN_DIRS=$(dirname "$1")/
