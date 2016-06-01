#!/bin/sh

#TODO: hand other flags to make?
make $1 RUN_DIRS=$(dirname "$1")/
