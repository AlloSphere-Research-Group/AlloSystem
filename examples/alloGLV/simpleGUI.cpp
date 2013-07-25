/*
Allocore Example: Simple GUI

Description:
The example demonstrates how to add a GUI (using GLV) to your application.

Author:
Lance Putnam, 4/25/2011
*/

#include "allocore/al_Allocore.hpp"
#include "alloGLV/al_ControlGLV.hpp"
#include "GLV/glv.h"

using namespace al;

int main(){

	// Create the top-level GUI view
	glv::GLV topView;
	
	// Create a button
	glv::Button btn(glv::Rect(100)); 

	// Tell our button to keep itself centered in the GUI
	btn.pos(glv::Place::CC).anchor(0.5, 0.5);

	// Add the button to the GUI
	topView << btn;


	Window win;

	// Prepend GUI input handler so it receives events first
	win.prepend(*new GLVInputControl(topView));
	
	// Append GUI window control so the GUI is drawn last
	win.append(*new GLVWindowControl(topView));

	win.append(*new StandardWindowKeyControls);

	win.create(Window::Dim(600,400), "Simple GUI");

	MainLoop::start();
	return 0;
}
