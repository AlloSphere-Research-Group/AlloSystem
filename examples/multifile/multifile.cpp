/*
AlloSystem Example: Multi-file application

Description:
This demonstrates how to write an application that uses multiple files. It can be run from
the AlloSystem root directory using:

./run.sh examples/multifile

Author(s):
Lance Putnam, 4/25/2011
Andres Cabrera 17/4/2014
*/

#include "allocore/al_Allocore.hpp"
using namespace al;

#include "mywindow.hpp"

MyWindow win1;

int main(){
	win1.add(new StandardWindowKeyControls);
	win1.create(Window::Dim(800, 600));

	MainLoop::start();
	return 0;
}
