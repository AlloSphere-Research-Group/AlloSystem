# AlloSystem C/C++ Libraries

AlloSphere Research Group,
University of California, Santa Barbara


## 1. About

AlloSystem is a cross-platform suite of C++ components for building interactive multimedia tools and applications. It is organized into separate "allo" modules that can be compiled and linked to on a per need basis. The most important module, AlloCore, comprises core components that most other modules depend on, such as math utilities, system information, audio i/o, and OpenGL-based windowing.


### 1.1 Directory Structure

AlloSystem modules are located in subdirectories beginning with "allo". Each module has the general directory layout:

	include/	- Header files (.h, .hpp)
	src/		- Source files (.c, .cpp)
	examples/	- example code pertaining to this module
	share/		- resource files for testing and demonstration purposes
	unitTests/	- unit tests
	build/		- default build folder, constructed on first use

The build folder (typically ./build/) is organized using a Unix-style hierarchy as follows:

	bin/		- binary executables
	include/	- library header files
	lib/		- libraries
	obj/		- built object files


## 2. Compilation Instructions

### 2.1 Installing Dependencies

From the AlloSystem/ root directory, cd into a module directory and run the script install_dependencies.sh. For example, to install AlloCore dependencies, you would run these commands from AlloSystem/:

	$ cd allocore
	$ ./install_dependencies.sh
	$ cd ..

This will download and install all the AlloCore dependencies using apt-get, MacPorts or homebrew.


### 2.2 Building a Library

#### Make (Linux, OS X)

The following variables can be modified to customize where built files are placed:

	BUILD_DIR		- location to build files into (default = ./build)
	DESTDIR			- location to install built files into (default = /usr/local/)

The following rules are available (to be run from the root directory):

	make allocore		- build allocore module
	make alloutil		- build utilities extension
	make Gamma		- build Gamma external library
	make GLV		- build GLV external library

	make clean		- removes binaries from build folder


## 3. Program Execution Using Make

### 3.1 Automatic "Build and Run"

The AlloCore Make system permits one to build and automatically run source files with a main() function defined. This is not meant to replace a full-fledged IDE for building complex projects, but rather to serve as a quick way to prototype ideas. Any source files can be built and run using the command
	
	./run.sh examples/mymain.cpp

and will be linked against Allocore and its dependencies. If you just want to build an executable without running it, then include AUTORUN=0 with the command.


### 3.2 User-defined Options

The following additional capabilities are possible and must be configured manually.

#### Custom Build Paths
Sometimes it is handy to build and run sources files from custom directories as well as include one's own "library" code that is linked with all build-and-run executables. To configure these, first create a new file "Makefile.user" in ./ . For convenience, you can copy the file Makefile.usertemplate. Next, add/modify the following variables:

	RUN_DIRS	= directory1 directory2 ...
	RUN_SRC_DIRS	= directoryA directoryB ...

RUN_DIRS is a list of directories that Make searches recursively for build-and-run source files.
The directory ./examples is automatically added to this list.

RUN_SRC_DIRS is a list of directories that contain source code for objects files that are to be linked with all build-and-run sources. The sources in RUN_SRC_DIRS can be thought of as your own library source code.

#### Custom Build Flags
In the same directory as the build-and-run source file add a file called "flags.txt" which contains valid flags for the compiler. For example, if you need to link to libfoo.so located in /usr/local/lib, your flags.txt will contain something like:

	-I/usr/local/include -L/usr/local/lib -lfoo

It is also possible to use the variable $(PROJECT_DIR) in flags.txt to refer to the directory the build-and-run source file is located in.


