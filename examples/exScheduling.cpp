#include <stdio.h>
#include <stdlib.h>
#include "allocore/al_Allocore.hpp"
using namespace al;

struct MyWindow : WindowGL{

	bool onCreate(){ 					printf("onCreate\n"); return true; }
	bool onDestroy(){					printf("onDestroy\n"); return true; }
	bool onResize(int w, int h){		printf("onResize     %d, %d\n", w, h); return true; }
	bool onVisibility(bool v){			printf("onVisibility %s\n", v?"true":"false"); return true; }
	
	void onKeyDown(const Keyboard& k){	printf("onKeyDown    "); printKey(); 
		switch(k.key()){
			case Key::Escape: fullScreenToggle(); break;
			case 'q': if(k.ctrl()) WindowGL::stopLoop(); break;
			case 'w': if(k.ctrl()) destroy(); break;
			case 'h': if(k.ctrl()) hide(); break;
			case 'm': if(k.ctrl()) iconify(); break;
			case 'c': if(k.ctrl()) cursorHideToggle(); break;
		}
	}
	void onKeyUp(const Keyboard& k){	printf("onKeyUp      "); printKey(); }
	
	void onMouseDown(const Mouse& m){	printf("onMouseDown  "); printMouse(); }
	void onMouseUp(const Mouse& m){		printf("onMouseUp    "); printMouse(); }
	void onMouseDrag(const Mouse& m){	printf("onMouseDrag  "); printMouse(); }
	//void onMouseMove(const Mouse& m){	printf("onMouseMove  "); printMouse(); }
	
	void printMouse(){
		const Mouse& m = mouse();
		printf("x:%4d y:%4d b:%d,%d\n", m.x(), m.y(), m.button(), m.down());
	}

	void printKey(){
		const Keyboard& k = keyboard();
		printf("k:%3d, %d s:%d c:%d a:%d\n", k.key(), k.down(), k.shift(), k.ctrl(), k.alt());
	}

	bool onFrame(){
		al_sec t = al_time();
		avg += 0.1*((t-last)-avg);
		last = t;
		
		gl.clear(gfx::AttributeBit::ColorBuffer | gfx::AttributeBit::DepthBuffer);
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
		return true;
	}
	
	void freqs(float v1, float v2){ freq1=v1; freq2=v2; }

	gfx::Graphics gl;
	float freq1, freq2;
	al_sec last;
	al_sec avg;
};

MyWindow win;

void set_freqs(al_sec t, double offset) {
	win.freqs(t, t+offset);
	printf("set_freqs @%f, fps %f\n", t, 1.0/win.avg);
	MainLoop::queue().send(t+0.1, set_freqs, offset);
}

int main (int argc, char * argv[]) {

	printf("%d\n", sizeof(Pose));

	/// Mainloop is created implicitly by the first reference to it; 
	/// e.g., the creation of a WindowGL, or directly via MainLoop:: calls.
	
	/// decreasing the interval may lead to more accurate timing, but more expensive
	/// reducing it to near the refresh rate may cause jerky animation
	//MainLoop::interval(0.01); 
	
	win.create(WindowGL::Dim(200,200,100), "Window 1", 40);
	win.freqs(1,2);
	win.avg = 0;

	MainLoop::queue().send(1.0, set_freqs, 1.);
	
	MainLoop::start();

	return 0;
}
