#include "allocore/io/al_Window.hpp"
#include "allocore/system/al_MainLoop.hpp"

#include <stdio.h>
#include <iostream>

using namespace al;
using namespace std;

struct MyWindow : Window {

	bool onCreate() {
		printf("onCreate\n");
		return 1;
	}

	bool onFrame(){
		// This is where your drawing code goes ...

		return true;
	}

	bool onDestroy() {
		printf("onDestroy\n");
		return 1;
	}

	bool onResize(int w, int h) {
		printf("onResize     %d, %d\n", w, h);
		return 1;
	}

	bool onVisibility(bool v) {
		printf("onVisibility %s\n", v?"true":"false");
		return 1;
	}

	bool onKeyDown(const Keyboard& k) {
		printf("onKeyDown    ");
		printKey();
		return 1;
	}

	bool onKeyUp(const Keyboard& k) {
		printf("onKeyUp      ");
		printKey();
		return 1;
	}

	bool onMouseDown(const Mouse& m) {
		printf("onMouseDown  ");
		printMouse();
		return 1;
	}

	bool onMouseUp(const Mouse& m) {
		printf("onMouseUp    ");
		printMouse();
		return 1;
	}

	bool onMouseDrag(const Mouse& m) {
		printf("onMouseDrag  ");
		printMouse();
		return 1;
	}

	bool onMouseMove(const Mouse& m) {
		printf("onMouseMove  ");
		printMouse();
		return 1;
	}

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
	MyWindow win;
	win.append(*new StandardWindowKeyControls);
	win.create(
		Window::Dim(100, 0, 400,300),	// dimensions, in pixels
		"Window",						// title
		5,								// ideal frames/second; actual rate will vary
		Window::DEFAULT_BUF				// display mode
	);
	MainLoop::start();
	return 0;
}
