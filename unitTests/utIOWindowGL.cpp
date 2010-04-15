#include "utAllocore.h"

#include "protocol/al_Graphics.hpp"

// OpenGL platform-dependent includes
#if defined (__APPLE__) || defined (OSX)
	#define AL_GRAPHICS_USE_OPENGL

	#include <OpenGL/OpenGL.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>
	#include <OpenGL/glu.h>
	
#elif defined(__linux__)
	#define AL_GRAPHICS_USE_OPENGL

	#include <GL/glew.h>
	#include <GL/gl.h>
	#include <GL/glext.h>
	#include <GL/glu.h>
	#include <time.h>
	
#elif defined(WIN32)
	#define AL_GRAPHICS_USE_OPENGL

	#include <windows.h>
	#include <gl/gl.h>
	#include <gl/glu.h>
	#pragma comment( lib, "winmm.lib")
	#pragma comment( lib, "opengl32.lib" )
	#pragma comment( lib, "glu32.lib" )
	
#endif

al::Graphics gl;

struct MyWindow : WindowGL{

	void onCreate(){					printf("onCreate\n"); }
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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		
		gl.begin(GL_TRIANGLE_STRIP);
			for(int i=0; i<9; ++i){
				float p = i/8.;
				gl.color(1, p, p);
				p *= 6.2832;
				gl.vertex(cos(p), sin(p), 0);
			}
		gl.end();
	}
};



int utIOWindowGL(){

	MyWindow win, win2;
	
//	gl.setBackend(GraphicsBackend::None);
	
//	printf("setBackendOpenGL %d\n", setBackendOpenGL(&gl));

//	struct Func:TimedFunction{
//		void onExecute(){ printf("hello\n"); }
//	};
//
//	Func tf;
//	tf(1000);

	win.create(WindowGL::Dim(200,200,200), "Window 1", 40);
	win2.create(WindowGL::Dim(200,200,400), "Window 2", 40, SingleBuf);

	WindowGL::startLoop();
	return 0;
}
