/*
Allocore Example: Window

Description:
The example demonstrates how to create a window with custom event callbacks.

Author:
Lance Putnam, 4/25/2011
*/

#include <stdio.h>
#include "allocore/io/al_Window.hpp"
#include "allocore/system/al_MainLoop.hpp"
#include "allocore/graphics/al_OpenGL.hpp"
using namespace al;

struct MyWindow : Window {

	bool onFrame(){
		// This is where your drawing code goes ...
		glClearColor(1,0,0,1);
		glClear(GL_COLOR_BUFFER_BIT);

		return true;
	}

	bool onCreate(){ 					printTitle(); printf("onCreate\n"); return 1; }
	bool onDestroy(){					printTitle(); printf("onDestroy\n"); return 1; }
	bool onResize(int w, int h){		printTitle(); printf("onResize     %d, %d\n", w, h); return 1; }
	bool onVisibility(bool v){			printTitle(); printf("onVisibility %s\n", v?"true":"false"); return 1; }

	bool onKeyDown(const Keyboard& k){	printTitle(); printf("onKeyDown    "); printKey(); return 1; }
	bool onKeyUp(const Keyboard& k){	printTitle(); printf("onKeyUp      "); printKey(); return 1;}

	bool onMouseDown(const Mouse& m){	printTitle(); printf("onMouseDown  "); printMouse(); return 1;}
	bool onMouseUp(const Mouse& m){		printTitle(); printf("onMouseUp    "); printMouse(); return 1;}
	bool onMouseDrag(const Mouse& m){	printTitle(); printf("onMouseDrag  "); printMouse(); return 1;}
	//bool onMouseMove(const Mouse& m){	printTitle(); printf("onMouseMove  "); printMouse(); return 1;}

	void printMouse(){
		const Mouse& m = mouse();
		printf("x:%4d y:%4d b:%d,%d\n", m.x(), m.y(), m.button(), m.down());
	}

	void printKey(){
		const Keyboard& k = keyboard();
		printf("k:%3d, %d s:%d c:%d a:%d\n", k.key(), k.down(), k.shift(), k.ctrl(), k.alt());
	}

	void printTitle(){ printf("%s: ", &title()[0]); }
};


int main(){

	// Construct window; note this does not actually create it
	MyWindow win;

	// Add some standard key controls for fullscreen, quitting, etc.
	win.append(*new StandardWindowKeyControls);

	// This creates the window
	win.create(
		Window::Dim(100, 0, 400,300),	// dimensions, in pixels
		"Window",						// title
		40,								// ideal frames/second; actual rate will vary
		Window::DEFAULT_BUF				// display mode
	);

	MainLoop::start();
	return 0;
}
