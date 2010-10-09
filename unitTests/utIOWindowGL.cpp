#include "utAllocore.h"

static gfx::GraphicsBackendOpenGL backend;
static gfx::Graphics gl(&backend);

struct MyWindow : WindowGL{

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

	void onFrame(){
		gl.clear(gfx::COLOR_BUFFER_BIT | gfx::DEPTH_BUFFER_BIT);
		gl.loadIdentity();
		gl.viewport(0,0, dimensions().w, dimensions().h);
		
		gl.begin(gfx::LINE_STRIP);
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

	win.create(WindowGL::Dim(200,200,100), "Window 1", 40);
	win2.create(WindowGL::Dim(200,200,300), "Window 2", 40);
//	win2.create(WindowGL::Dim(200,200,300), "Window 2", 40, SingleBuf);

	win.freqs(1,2);
	win2.freqs(3,4);

//win.cursorHide(true);

	WindowGL::startLoop();
	return 0;
}
