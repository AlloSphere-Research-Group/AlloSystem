# AlloSystem C/C++ Libraries

Developed by:

[AlloSphere Research Group,](http://www.allosphere.ucsb.edu/)

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

*Note:* You can skip this section if you are just planning on running applications and projects. See section 3 below.

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

 Optionally on:
 * luajit (for alloutil)
 * GLV (for alloGLV)
 * vsr (for allovsr)

You may not need all these dependencies if you plan to build only part of Allosystem.

Some of the examples also depend on Gamma. If you don't have Gamma you will need to disable building the examples. See below.

From the AlloSystem/ root directory, cd into a module directory and run the script install_dependencies.sh. For example, to install AlloCore dependencies, you would run these commands from AlloSystem/:

	$ cd allocore
	$ ./install_dependencies.sh
	$ cd ..

This will download and install all the AlloCore dependencies using apt-get, MacPorts or homebrew.


2.2 Building Allosystem (Using Make on Linux, OS X and MSYS on Windows)
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

To enable a full build including alloGLV and allovsr, put the GLV and vsr sources next to the AlloSystem sources, and inside the AlloSystem folder do:

    cmake . -DBUILD_EVERYTHING=1

This will build GLV, vsr and all the allo modules if the rest of the dependencies are present.

Other useful configuration variables are:

    BUILD_ALLOGLV=1/0
    BUILD_ALLOUTIL=1/0
    BUILD_ALLOVSR=1/0
    BUILD_GAMMA=1/0
    BUILD_VSR=1/0
    BUILD_GLV=1/0

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

3. Running examples and projects
---

Allosystem offers an easy way to try out examples and build simple projects without having to write makefiles or configure IDE projects. Any .cpp file placed within the AlloSystem sources can be built into an application with a line like:

    ./run.sh allocore/examples/graphics/shaderSprites.cpp

This will also build any required dependencies and run cmake if needed.

You can also pass a directory, and all the source files in that directory will be built into a single application.

You can make a debug build of the libraries and the application by running:

    ./debug.sh allocore/examples/graphics/shaderSprites.cpp

This will run the file in the debugger, so if the application crashes, it will drop you to the debugger shell. If you need to specify a particular debugger instead of the default gdb, adjust the debug.sh script.

4. Installing Allosystem
----------------------------------------

You can install the AlloSystem libraries and headers, which will allow CMake AlloSystem projects to use it instead of having to include all the AlloSystem sources in your project.

Using cmake configured for Makefiles, you will be able to install all of Allosystem with headers by doing:

	sudo make install

You can specify a different install path by doing (e.g. to install in /opt/local):

	./distclean
	cmake -DCMAKE_INSTALL_PREFIX:PATH=/opt/local .
	sudo make install

You can uninstall with:

	xargs rm < install_manifest.txt

License
======

This project is licensed under the terms of the 3-clause BSD license.
