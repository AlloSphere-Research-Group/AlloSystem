#include "utAllocore.h"

static GraphicsBackendOpenGL backend;
static Graphics gl(&backend);

struct MyWindow : Window{

	bool onCreate(){ 					printf("onCreate\n"); return true; }
	bool onDestroy(){					printf("onDestroy\n"); return true; }
	bool onResize(int w, int h){		printf("onResize     %d, %d\n", w, h); return true; }
	bool onVisibility(bool v){			printf("onVisibility %s\n", v?"true":"false"); return true; }
	
	bool onKeyDown(const Keyboard& k){	printf("onKeyDown    "); printKey(); return true; }
	bool onKeyUp(const Keyboard& k){	printf("onKeyUp      "); printKey(); return true; }
	
	bool onMouseDown(const Mouse& m){	printf("onMouseDown  "); printMouse(); return true; }
	bool onMouseUp(const Mouse& m){		printf("onMouseUp    "); printMouse(); return true; }
	bool onMouseDrag(const Mouse& m){	printf("onMouseDrag  "); printMouse(); return true; }
	//void onMouseMove(const Mouse& m){	printf("onMouseMove  "); printMouse(); return true; }
	
	void printMouse(){
		const Mouse& m = mouse();
		printf("x:%4d y:%4d b:%d,%d\n", m.x(), m.y(), m.button(), m.down());
	}

	void printKey(){
		const Keyboard& k = keyboard();
		printf("k:%3d, %d s:%d c:%d a:%d\n", k.key(), k.down(), k.shift(), k.ctrl(), k.alt());
	}

	bool onFrame(){
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
		return true;
	}
	
	void freqs(float v1, float v2){ freq1=v1; freq2=v2; }

	float freq1, freq2;
};


int utIOWindowGL(){

	MyWindow win;
	MyWindow win2;
	
//	gl.setBackend(GraphicsBackend::None);
	
//	printf("setBackendOpenGL %d\n", setBackendOpenGL(&gl));

//	struct Func:TimedFunction{
//		void onExecute(){ printf("hello\n"); }
//	};
//
//	Func tf;
//	tf(1000);

	win.create(Window::Dim(100,0, 200,200), "Window 1", 40);
	win2.create(Window::Dim(300,0, 200,200), "Window 2", 40);
//	win2.create(Window::Dim(200,200,300), "Window 2", 40, SingleBuf);

	win.freqs(1,2);
	win2.freqs(3,4);

//win.cursorHide(true);

	Window::startLoop();
	return 0;
}
