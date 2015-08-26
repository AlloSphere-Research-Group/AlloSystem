#!/bin/sh

# Install only core dependencies by default
modules="allocore"

# Override if modules are passed in as arguments.
if [ "$#" -gt 0 ]; then
  modules="$*"
fi

ROOT=`pwd`
PLATFORM=`uname`
ARCH=`uname -m`
echo "Installing AlloSystem dependencies for ${PLATFORM} ${ARCH} from ${ROOT}"

for module in $modules; do
	cd $module
	./install_dependencies.sh
	cd ..
done

