# AlloSystem C/C++ Libraries

AlloSphere Research Group,
University of California, Santa Barbara


1. About
========================================

AlloSystem is a cross-platform suite of C++ components for building interactive multimedia tools and applications. It is organized into separate "allo" modules that can be compiled and linked to on a per need basis. The most important module, AlloCore, comprises core components that most other modules depend on, such as math utilities, system information, audio i/o, and OpenGL-based windowing.


1.1 Directory Structure
----------------------------------------

AlloSystem modules are located in subdirectories beginning with "allo". Each module has the general directory layout:

	MODULE_NAME/	- Header files (.h, .hpp)
	src/		- Source files (.c, .cpp)
	examples/	- example code pertaining to this module
	share/		- resource files for testing and demonstration purposes
	unitTests/	- unit tests

The build folder (typically ./build/) is organized using a Unix-style hierarchy as follows:

	bin/		- binary executables
	lib/		- libraries



2. Compilation Instructions
========================================

2.1 Installing Dependencies
----------------------------------------

Allosystem depends on:

 * Cmake

 * APR

 * Assimp (v.2 or v.3 supported)

 * Freeimage

 * Freetype

 * GLEW

 * GLUT

 * Libsndfile

 * GLV

 * vsr

You may not need all these dependencies if you plan to build only part of Allosystem.

Some of the examples also depend on Gamma. If you don't have Gamma you will need to disable building the examples. See below.

From the AlloSystem/ root directory, cd into a module directory and run the script install_dependencies.sh. For example, to install AlloCore dependencies, you would run these commands from AlloSystem/:

	$ cd allocore
	$ ./install_dependencies.sh
	$ cd ..

This will download and install all the AlloCore dependencies using apt-get, MacPorts or homebrew.


2.2 Building Allosystem (Makefiles on all systems)
----------------------------------------

You need to use cmake to configure the build for your system. You can build Allocore like this: 

	./distclean
	cmake .
	make

This will build all Allosystem libraries in the build/lib folder and the examples in the build/bin folder.

If you want to build wihtout examples:

	./distclean
	cmake . -DNO_EXAMPLES=1
	make

To produce a debug build:

	./distclean
	cmake . -DCMAKE_BUILD_TYPE=Debug
	make

2.3 Building Allosystem (XCode project)
----------------------------------------

Do:

	./distclean
	cmake . -GXcode
	open Allosystem.xcodeprj

You will be able to run examples and debug from Xcode

2.4 Building Allosystem (Visual Studio project)
----------------------------------------

Coming soon...

2.4 Installing Allosystem
----------------------------------------

Using cmake configured for Makefiles, you will be able to install all of Allosystem with headers by doing:

	sudo make install

You can specify a different install path by doing (to install in /opt/local):

	./distclean
	cmake -DCMAKE_INSTALL_PREFIX:PATH=/opt/local .
	sudo make install

