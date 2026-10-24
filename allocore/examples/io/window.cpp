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

	// Window has several virtual functions that can be overridden. Here they are:

	bool onFrame() override {
		// We typically clear the color buffer first
		glClearColor(1,0,0,1);
		glClear(GL_COLOR_BUFFER_BIT);
		// This is where your drawing code goes ...
		return true;
	}

	bool onCreate() override { 					printf("onCreate\n"); return 1; }
	bool onDestroy() override {					printf("onDestroy\n"); return 1; }
	bool onResize(int w, int h) override {		printf("onResize     %d, %d\n", w, h); return 1; }
	bool onVisibility(bool v) override {		printf("onVisibility %s\n", v?"true":"false"); return 1; }

	bool onKeyDown(const Keyboard& k) override {printf("onKeyDown    "); printKey(); return 1; }
	bool onKeyUp(const Keyboard& k) override {	printf("onKeyUp      "); printKey(); return 1;}

	bool onMouseDown(const Mouse& m) override {	printf("onMouseDown  "); printMouse(); return 1;}
	bool onMouseUp(const Mouse& m) override {	printf("onMouseUp    "); printMouse(); return 1;}
	bool onMouseWheel(const Mouse& m) override {printf("onMouseWheel "); printMouse(); return 1; }
	bool onMouseDrag(const Mouse& m) override {	printf("onMouseDrag  "); printMouse(); return 1;}
	//bool onMouseMove(const Mouse& m) override {	printf("onMouseMove  "); printMouse(); return 1;}

	void printMouse() const {
		const auto& m = mouse();
		printf("x:%4d y:%4d b:%d,%d w:%d,%d\n", m.x(), m.y(), m.button(), m.down(), m.wheelX(), m.wheelY());
	}

	void printKey() const {
		const auto& k = keyboard();
		printf("k:%3d, %d s:%d c:%d a:%d\n", k.key(), k.down(), k.shift(), k.ctrl(), k.alt());
	}
};


int main(){

	// Construct window; note this does not actually create it
	MyWindow win;

	// Add some standard key controls for fullscreen, quitting, etc.
	win.append(*new StandardWindowKeyControls);

	// This creates the window
	win.create(
		{100, 0, 400,300},		// rect position and extent, in pixels
		"Window",				// title
		40,						// ideal frames/second; actual rate will vary
		Window::DEFAULT_BUF		// display mode
	);

	// Other options (can be set before or after creation):
	//win.keyRepeat(true);	// send repeated key events when key held down
	//win.asap(true);		// render frames as soon as possible
	//win.vsync(false);		// vertical sync: disable for better FPS, but possible tearing

	MainLoop::start();
}
