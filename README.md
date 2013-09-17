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

Allosystem offers an easy way to try out examples and build simple projects without having to write makefiles or configure IDE projects. Any .cpp file placed within the examples folder (of AlloSystem and the submodules) will be built into an application with a line like:

    ./buildandrun.sh allocore/examples/graphics/shaderSprites.cpp 

This will also build any required dependencies and run cmake if needed.

Note that you will have to run cmake if you want to use alloGLV or allovsr, as they are not part of the default build.

4. Installing Allosystem
----------------------------------------

Using cmake configured for Makefiles, you will be able to install all of Allosystem with headers by doing:

	sudo make install

You can specify a different install path by doing (to install in /opt/local):

	./distclean
	cmake -DCMAKE_INSTALL_PREFIX:PATH=/opt/local .
	sudo make install

You can uninstall with:

	xargs rm < install_manifest.txt

License
======

Copyright (C) 2009-2013. AlloSphere Research Group, Media Arts & Technology, UCSB.

Copyright (C) 2009-2013. The Regents of the University of California.

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

Neither the name of the University of California nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

