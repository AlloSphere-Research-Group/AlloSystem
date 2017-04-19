#!/bin/bash

# If no test name is provided - die
die() {
	echo >&2 "$@"
	exit 1
}

[ "$#" -eq 1 ] || die "Please provide a test name"


# Create new directory for the test
mkdir "test$1"

# Copy over the test template
cp testTemplate/* "test$1"/

pushd "test$1"

	# Changing to appropriate test name within the files
	sed "s/Template/$1/g" <"testTemplate.cpp" >"test$1.cpp"
	sed "s/Template/$1/g" <"testTemplate.h" >"test$1.h"
	sed "s/Template/$1/g" <"CMakeLists.txt" >"CMakeLists_new.txt"

	# Cleanup
	rm testTemplate.*
	mv CMakeLists_new.txt CMakeLists.txt

popd

echo "Created test named $1Test in test$1"
echo "NOTE: Remember to put add_subdirectory(test$1) at the end of tests/CMakeLists.txt"
