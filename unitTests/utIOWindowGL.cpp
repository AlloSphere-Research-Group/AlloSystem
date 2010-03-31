#include "utAllocore.h"

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
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//		glLoadIdentity();
//		
//		glBegin(GL_TRIANGLE_STRIP);
//			for(int i=0; i<9; ++i){
//				float p = i/8.;
//				glColor3f(1, p, p);
//				p *= 6.2832;
//				glVertex3f(cos(p), sin(p), 0);
//			}
//		glEnd();
	}
};



int utIOWindowGL(){

	MyWindow win, win2;

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
