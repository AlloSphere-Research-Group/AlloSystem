/*
Allocore Example: Detachable GUI

Description:
This demonstrates how to create a window with a detachable GUI.

Author:
Lance Putnam, 2011, putnam.lance@gmail.com
*/

#include "allocore/al_Allocore.hpp"
#include "alloGLV/al_ControlGLV.hpp"
#include "GLV/glv.h"

using namespace al;

struct MyWindow : public Window{
	bool onFrame(){
		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return true;
	}
};

int main(){
	MyWindow win;
	GLVDetachable gui;
	glv::Table layout("><");
	
	// Create some random widgets for testing
	glv::Button btn;
	glv::Sliders sld(glv::Rect(100,10*4), 1,4);
	glv::Slider2D sld2;

	btn.colors().set(glv::Color(1,0,0));
	layout.enable(glv::DrawBack);
	//layout.enable(glv::Controllable | glv::HitTest);
	//layout.addHandler(glv::Event::MouseDrag, glv::Behavior::mouseMove);

	layout
		<< gui.detachedButton() << new glv::Label("detach")
		<< btn << new glv::Label("param 1")
		<< sld << new glv::Label("param 2")
		<< sld2<< new glv::Label("param 3")
	;

	gui << layout;	
	layout.arrange();
	
	// We must assign a parent window to the GUI
	gui.parentWindow(win);

	win.append(*new StandardWindowKeyControls);
	win.create();

	MainLoop::start();
}
