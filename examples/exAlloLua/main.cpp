#include <iostream>
#include <vector>

#include "io/al_WindowGL.hpp"
#include "protocol/al_Graphics.hpp"

using namespace al;

struct MyWindow : WindowGL {

	static std::vector<MyWindow *> windows;

	void onCreate(){ 					printf("onCreate\n"); }
	void onDestroy(){					printf("onDestroy\n"); }
	void onResize(int w, int h){		printf("onResize     %d, %d\n", w, h); }
	void onVisibility(bool v){			printf("onVisibility %s\n", v?"true":"false"); }
	
	void onKeyDown(const Keyboard& k){	printf("onKeyDown    "); printKey(); 
		switch(k.key()){
			case Key::Escape: fullScreenToggle(); break;
			case 'q': if(k.ctrl()) WindowGL::stopLoop(); break;
			case 'w': if(k.ctrl()) destroy(); break;
			case 'h': if(k.ctrl()) hide(); break;
			case 'm': if(k.ctrl()) iconify(); break;
			case 'c': if(k.ctrl()) cursorHideToggle(); break;
			case 'n': 
				windows.push_back(new MyWindow);
				windows[windows.size()-1]->create(WindowGL::Dim(200,200,400), "Window X", 40);
				break;
		}
	}
	void onKeyUp(const Keyboard& k){	printf("onKeyUp      "); printKey(); }
	
	void onMouseDown(const Mouse& m){	printf("onMouseDown  "); printMouse(); }
	void onMouseUp(const Mouse& m){		printf("onMouseUp    "); printMouse(); }
	void onMouseDrag(const Mouse& m){	printf("onMouseDrag  "); printMouse(); printf("%p\n", this); }
	//void onMouseMove(const Mouse& m){	printf("onMouseMove  "); printMouse(); }
	
	void printMouse(){
		const Mouse& m = mouse();
		printf("x:%4d y:%4d b:%d,%d\n", m.x(), m.y(), m.button(), m.down());
	}

	void printKey(){
		const Keyboard& k = keyboard();
		printf("k:%3d, %d s:%d c:%d a:%d\n", k.key(), k.down(), k.shift(), k.ctrl(), k.alt());
	}

	void onFrame(){
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.loadIdentity();
		gl.viewport(0,0, dimensions().w, dimensions().h);
		
		gl.begin(gl.LINE_STRIP);
			static float limit = 120;
			for (float i = 0; i<limit; i++) {
				float p = i / limit;
				gl.color(1, p, 1-p);
				p *= 6.283185308;
				gl.vertex(cos(p*freq1), sin(p*freq2), 0);
			}
		gl.end();
//printf("%p: %d x %d\n", this, dimensions().w, dimensions().h);
	}
	
	void freqs(float v1, float v2){ freq1=v1; freq2=v2; }

	gfx::Graphics gl;
	float freq1, freq2;
};

std::vector<MyWindow *> MyWindow::windows;

int main (int argc, char * const argv[]) {

	MyWindow win;
	win.create(WindowGL::Dim(200,200,100), "Window 1", 40);
	
	WindowGL::startLoop();	/* GLUT never returns. */
    return 0;
}
