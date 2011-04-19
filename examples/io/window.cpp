#include <stdio.h>
#include "allocore/al_Allocore.hpp"
using namespace al;

struct MyWindow : Window {

	bool onCreate(){ 					printTitle(); printf("onCreate\n"); return 1; }
	bool onDestroy(){					printTitle(); printf("onDestroy\n"); return 1; }
	bool onResize(int dw, int dh){		printTitle(); printf("onResize     %d, %d\n", dw, dh); return 1; }
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

	bool onFrame(){
		return true;
	}
	
	void printTitle(){ printf("%s: ", &title()[0]); }
};


int main(){

	MyWindow win1, win2;

	win1.add(new StandardWindowKeyControls);
	win2.add(new StandardWindowKeyControls);

	win1.create(Window::Dim(100, 0, 400,300), "Window 1");
	//win2.create(Window::Dim(500, 0, 400,300), "Window 2");
	
	MainLoop::start();
	return 0;
}
