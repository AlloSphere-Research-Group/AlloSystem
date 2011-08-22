#include "allocore/al_Allocore.hpp"
#include "alloutil/al_ControlGLV.hpp"
#include "GLV/glv.h"

using namespace al;

int main(){

	glv::GLV topView(0,0);
	glv::Button btn(glv::Rect(100)); 

	btn.pos(glv::Place::CC).anchor(0.5, 0.5);

	topView << btn;

	Window win;
	win.add(*new StandardWindowKeyControls);
	win.add(*new GLVInputControl(topView));
	win.add(*new GLVWindowControl(topView));

	win.create(Window::Dim(600,400), "Simple GUI");

	MainLoop::start();

	return 0;
}