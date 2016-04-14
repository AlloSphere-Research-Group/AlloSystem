/*
AlloSystem Example: Multi-file application

Description:
This demonstrates how to write an application that uses multiple files. It can be run from
the AlloSystem root directory using:

./run.sh examples/multifile

Author(s):
Andres Cabrera 17/4/2014

Based on an allocore example by Lance Putnam
*/

#include "allocore/al_Allocore.hpp"
using namespace al;

// The window class is defined in a separate header and the mywindow.cpp class
// When passing a directory to the run script, it will look for all .cpp files
// in the directory and link them all together into a single application
// Make sure only one of the files has a main() function!

#include "mywindow.hpp"

MyWindow win1;

int main(){
	win1.add(new StandardWindowKeyControls);
	win1.create(Window::Dim(800, 600));

	MainLoop::start();
	return 0;
}
