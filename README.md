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
The only mandatory dependency for AlloSystem is Cmake, which is the build system used.

Other optional dependencies are:
 * APR
 * Assimp (v.2 or v.3 supported)
 * Freeimage
 * Freetype
 * GLEW
 * GLUT
 * Libsndfile
 * luajit (for alloutil)
 * GLV (for alloGLV)

You may not need all these dependencies if you plan to build only part of Allosystem. The build system will try to find the dependencies available and build as much functionality as possible. However, if some dependencies are not available in your system, you won't have all functionality available and building some examples or existing code that uses it will fail.

AlloSystem provides cross-platform scripts to simplify downloading dpendencies. From the AlloSystem/ root directory, cd into a module directory and run the script install_dependencies.sh. For example, to install AlloCore dependencies, you would run these commands from AlloSystem/:

	$ cd allocore
	$ ./install_dependencies.sh
	$ cd ..

This will download and install all the AlloCore dependencies using apt-get, MacPorts, homebrew or will try to get the sources and build the dependencies.


2.2 Building AlloSystem libraries (Using Make on Linux, OS X and MSYS on Windows)
----------------------------------------

You need to use cmake to configure the build for your system. You can build Allocore like this:

An alternative to building the AlloSystem libraries is using the application building and running facilties provided by the *run.sh* and *debug.sh* scripts, see below.

To build the AlloSystem libraries, you need to use cmake to configure the build for your system. You can build Allocore like this:

	./distclean
	cmake .
	make

This will build all Allosystem libraries in the build/lib folder and the examples in the build/bin folder.

If you want to build without examples:

	./distclean
        cmake . -DBUILD_EXAMPLES=0
	make

To produce a debug build:

	./distclean
	cmake . -DCMAKE_BUILD_TYPE=Debug
	make

2.3 Building AlloSystem (XCode project)
----------------------------------------

Do:

	./distclean
	cmake . -GXcode
        open AlloSystem.xcodeprj

You will be able to run examples and debug from Xcode

2.4 Building Allosystem (Visual Studio project)
----------------------------------------

Coming soon...

3. Running examples and projects
------

Allosystem offers an easy way to try out examples and build simple projects without having to write makefiles or configure IDE projects. Any .cpp file placed within the AlloSystem sources can be built into an application with a line like:

    ./run.sh allocore/examples/graphics/shaderSprites.cpp

This will also build any required dependencies and run cmake if needed.

You can also pass a directory instead of a filename, and all the source files in that directory will be built into a single application (you must ensure that one and only one of those files has a *main()* function).

You can make a debug build of the libraries and the application by running:

    ./debug.sh allocore/examples/graphics/shaderSprites.cpp

This will run the file in the debugger, so if the application crashes, it will drop you to the debugger shell. If you need to specify a particular debugger instead of the default gdb, adjust the *debug.sh* script.

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

Copyright (C) 2009-2014. AlloSphere Research Group, Media Arts & Technology, UCSB.

Copyright (C) 2009-2014. The Regents of the University of California.

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

