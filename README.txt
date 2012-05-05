AlloSphere Core C/C++ Libraries

AlloSphere Research Group
University of California, Santa Barbara


========================================
1. About
========================================
The AlloCore is a cross-platform suite of C++ components for building interactive multimedia tools and applications.

________________________________________
1.1 Directory Structure
________________________________________
The project root is organized into the following main directories:

	include/	- AlloCore header files organized by module name
	src/		- AlloCore source files (.c, .cpp)

	external/	- optional external libraries included for convenience

	linux/		- Linux specific project files and dependent libraries
	osx/		- Mac OSX specific project files and dependent libraries
	share/		- resource files for testing and demonstration purposes

	examples/	- example projects using library
	unitTests/	- unit tests for library

	build/		- default build folder, constructed on first use

The build folder (typically ./build/) is organized using a Unix-style hierarchy as follows:

	bin/		- binary executables
	include/	- library header files
	lib/		- libraries
	obj/		- built object files


========================================
2. Compilation Instructions
========================================
----------------------------------------
2.1 Installing Dependencies
----------------------------------------
For Linux, run the script

	./linux/install_dependencies.sh

which will download and install all the Allocore dependencies using apt-get.

----------------------------------------
2.2 Building a Library
----------------------------------------
........................................
Make (Linux, OS X)
........................................

The following variables can be modified to customize where built files are placed:

	BUILD_DIR		- location to build files into (default = ./build)
	DESTDIR			- location to install built files into (default = /usr/local/)

The following rules are available (to be run from the root directory):

	make allocore		- build allocore library
	make allojit		- build allocore JIT extension (optional)
	make alloutil		- build allocore utilities extension
	make gamma		- build Gamma external library
	make glv		- build GLV external library

	make install		- installs built components
	make clean		- removes binaries from build folder

	make examples/x.cpp	- builds and runs example source file x.cpp (see section 3 below)

	make test		- build and run unit tests


========================================
3. Program Execution Using Make
========================================
----------------------------------------
3.1 Automatic "Build and Run"
----------------------------------------
The AlloCore Make system permits one to build and automatically run source files with a main() function defined. This is not meant to replace a full-fledged IDE for building complex projects, but rather to serve as a quick way to prototype ideas. By default, any source files located in ./examples or any subfolder thereof, can be built and run using the command
	
	make examples/mymain.cpp

and will be linked against Allocore and its dependencies. If you are having trouble using tab autocompletion after the 'make' command, then it is likely that a completion rule has been defined elsewhere for make. You can disable this by adding to the bottom of your ~/.bashrc file

	complete -r make

which will restore the default autocompletion using the file system. If you just want to build and executable without running it, then include AUTORUN=0 with the make command.

----------------------------------------
3.2 User-defined Options
----------------------------------------
The following additional capabilities are possible and must be configured manually.

1. Definition of custom build paths.
Sometimes it is handy to build and run sources files from custom directories as well as include one's own "library" code that is linked with all build-and-run executables. To configure these create a file "Makefile.user" in ./ and add the following variables:

	RUN_DIRS	= directory1 directory2 ...
	RUN_SRC_DIRS	= directoryA directoryB ...

RUN_DIRS is a list of directories that Make searches recursively for build-and-run source files.
The directory ./examples is automatically added to this list.

RUN_SRC_DIRS is a list of directories that contain source code for objects files that are to be linked with all build-and-run sources. The sources in RUN_SRC_DIRS can be thought of as your own library source code.

2. Definition of custom build flags.
In the same directory as the build-and-run source file add a file called "flags.txt" which contains valid flags for the compiler. For example, if you need to link to libfoo.so located in /usr/local/lib, your flags.txt will contain something like:

	-I/usr/local/include -L/usr/local/lib -lfoo


