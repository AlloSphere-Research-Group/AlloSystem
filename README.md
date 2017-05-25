# AlloSystem C/C++ Libraries

Developed by:

[AlloSphere Research Group](http://www.allosphere.ucsb.edu/)

University of California, Santa Barbara

<a href="https://scan.coverity.com/projects/5548">
  <img alt="Coverity Scan Build Status"
     src="https://scan.coverity.com/projects/5548/badge.svg"/>
</a>
[![Build Status](https://travis-ci.org/AlloSphere-Research-Group/AlloSystem.svg?branch=devel)](https://travis-ci.org/AlloSphere-Research-Group/AlloSystem)

## 1. About

AlloSystem is a cross-platform suite of C++ components for building interactive multimedia tools and applications. It is organized into separate "allo" modules that can be compiled and linked to on a per need basis. The most important module, AlloCore, comprises core components that most other modules depend on, such as math utilities, system information, audio IO, and OpenGL-based windowing.


### 1.1 Directory Structure

AlloSystem modules are located in subdirectories beginning with "allo". Each module has the general directory layout:

```
  MODULE_NAME/  - Header files (.h, .hpp)
  src/      - Source files (.c, .cpp)
  examples/   - Example code pertaining to this module
  share/      - Resource files for testing and demonstration purposes
  unitTests/    - Unit tests
```

The build folder (typically `./build/`) is organized using a Unix-style hierarchy as follows:

```
  bin/      - Binary executables
  include/    - Library header files
  lib/      - Libraries
```

## 2. Installing Dependencies

The only mandatory dependency for AlloSystem is CMake, which is the build system used.

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
 * Gamma (For audio examples and alloaudio)
 * FFTW3 (For alloaudio)

You may not need all these dependencies if you plan to build only part of AlloSystem. The build system will try to find the dependencies available and build as much functionality as possible. However, if some dependencies are not available in your system, you won't have all functionality available and building some examples or existing code that uses it will fail.

AlloSystem provides cross-platform scripts to simplify downloading dependencies. From the `AlloSystem/` root directory,  run the script `install_all_dependencies.sh`. Provide one or more module names to install dependencies for the specified modules. For example, to install AlloCore dependencies, you would run these commands from `AlloSystem/`:

```
$ ./install_all_dependencies.sh allocore
```

This will download and install all of the AlloCore dependencies using APT on Linux, Homebrew (or MacPorts) on Mac OS X, or building from source.

### 2.1 GLV and Gamma

The AlloSystem build system can incorporate the building and linking of GLV and Gamma. GLV provides a set of GL widgets for GUI creation within an OpenGL window and Gamma provides a C++ audio synthesis and DSP library. If they are placed side by side with the AlloSystem sources, they will be found and built by default. You can get these from git with the commands:

```
git clone git@github.com:AlloSphere-Research-Group/GLV.git
git clone git@github.com:AlloSphere-Research-Group/Gamma.git
```

## 3. Running examples and projects

AlloSystem offers an easy way to try out examples and build simple projects without having to write makefiles or configure IDE projects. Any .cpp file placed within the AlloSystem sources can be built into an application with a line like:

```
  ./run.sh allocore/examples/graphics/shaderSprites.cpp
```

This will also build any required dependencies and run CMake if needed.

You can also pass a directory instead of a filename, and all the source files in that directory will be built into a single application (you must ensure that one and only one of those files has a `main()` function).

You can make a debug build of the libraries and the application by running:

```
  ./run.sh -d allocore/examples/graphics/shaderSprites.cpp
```

This will run the file in the debugger, so if the application crashes, it will drop you to the debugger shell. If you need to specify a particular debugger instead of the default `gdb`, adjust the `run.sh` script.

To build all files in a directory into a single application, just provide the directory to the run script:

```
  ./run.sh -d project/many_files_in_this_folder
```

If you just want to build an executable without running it, then include the `-n` flag:

```
  ./run.sh -n allocore/examples/graphics/shaderSprites.cpp
```

### 3.1 Application dependencies and build flags

If a file called `flags.cmake` is found in the source directory for the run script, the CMake commands found in it will be passed to the CMake build system. This enables writing any kind of build instructions and commands that are specific to the files on that folder, and can be used to specify the required information for additional dependencies like library paths and names, include directories. A `flags.txt` file that adds support for an additional library will look like: 

```
  include_directories(/path/to/headers)
  target_link_libraries("${APP_NAME}" libname)
```

It can also be used to do platform specific actions, for example for setting compiler flags:

```
  if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-deprecated-declarations")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wno-deprecated-declarations")
  endif()
```

###3.2 Additional tips
Note: If you are having trouble using tab autocompletion after the `make` command, then it is likely that a completion rule has been defined elsewhere for make. You can disable this by adding to the bottom of your `~/.bashrc` file

```
  complete -r make
```

which will restore the default autocompletion using the file system. If that still doesn't work, then you can try the command

```
  sudo mv /usr/share/bash-completion/completions/make /usr/share/bash-completion/completions/make_disabled
```

A complete tutorial of AlloSystem can be found at [AlloSystem User Guide](http://mantaraya36.gitbooks.io/allosystem-user-guide/content/)


## 4. Compilation Instructions

Compilation is done automatically when using the run script, but if you need AlloSystem as a library you can use these methods.


### 4.1 Building AlloSystem libraries (Using Make on Linux and OS X, and MSYS on Windows)

You need to use CMake to configure the build for your system. You can build AlloCore like this:

An alternative to building the AlloSystem libraries is using the application building and running facilities provided by the *run.sh* script, see below.

To build the AlloSystem libraries, you need to use CMake to configure the build for your system:

```
  ./distclean
  cmake .
  make
```

This will build all AlloSystem libraries in the `./build/lib` folder and the examples in the `./build/bin` folder.


### 4.2 Building AlloSystem (XCode project)

Do:

```
  ./distclean
  cmake . -GXcode
  open AlloSystem.xcodeprj
```

You will be able to run examples and debug from Xcode

### 4.3 Building Allosystem (Visual Studio project)

Coming soon...


### 4.4 Various CMake Options

If you want to build without examples:

```
  ./distclean
  cmake . -DBUILD_EXAMPLES=0
  make
```

To produce a debug build:

```
  ./distclean
  cmake . -DCMAKE_BUILD_TYPE=Debug
  make
```

The CMake build system for AlloSystem is setup to build everything it finds y default. When hard dependencies for a particular module are not met, the module will not be built. When optional dependencies are not found, the module will be built without support for that particular functionality. This will be reported in the console text, and if the functionality is used, this might result in a "header not found" error or a linker error.

You can also optionally force or disable building of external modules setting the following variables:

 * `BUILD_GAMMA`
 * `BUILD_GLV`

like:

```
  cmake . -DBUILD_GAMMA=0
```

Other variables that affect the build:

 * `BUILD_ROOT_DIR`: Sets where the build products will be put. This includes binaries, libraries and headers. By default it is set to the folder `build/` within the AlloSystem root folder.

## 5. Installing Allosystem

You can install the AlloSystem libraries and headers, which will allow CMake AlloSystem projects to use it instead of having to include all the AlloSystem sources in your project. The difference between the install target and setting `BUILD_ROOT_DIR` is that the install target only copies libraries and headers, not executables.

Using CMake configured for Makefiles, you will be able to install all of AlloSystem with headers by doing:

```
  sudo make install
```

You can specify a different install path by doing (e.g. to install in `/opt/local`):

```
  ./distclean
  cmake -DCMAKE_INSTALL_PREFIX:PATH=/opt/local .
  sudo make install
```

You can uninstall with:

```
  xargs rm < install_manifest.txt
```

## Unit tests

AlloSystem uses the CTest facilities from CMake to organize and launch unit testing.
To run the tests do:
```
cmake . -DCMAKE_BUILD_TYPE=Debug
make -j7
make test
```

To see verbose output do:
```
make test ARGS="-V"
```

# License

This project is licensed under the terms of the 3-clause BSD license.

Copyright (C) 2009-2015. AlloSphere Research Group, Media Arts & Technology, UCSB.

Copyright (C) 2009-2015. The Regents of the University of California.

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

