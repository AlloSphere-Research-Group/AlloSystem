#include "utAllocore.h"
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/io/al_Window.hpp"
using namespace al;

static Graphics gl;

struct MyWindow : Window{

	int frameNum;
	int onCreateCalls;
	int onDestroyCalls;
	int onResizeCalls;

	static const Window::Dim& defaultDim(){
		static Window::Dim res(40,50, 400,300);
		return res;
	}

	static const std::string& defaultTitle(){
		static std::string res("Test Window");
		return res;
	}

	MyWindow(){
		onCreateCalls = onDestroyCalls = onResizeCalls = 0;
		frameNum = 0;
	}

	bool onCreate(){
		++onCreateCalls;
		return true;
	}

	bool onDestroy(){
		++onDestroyCalls;
		return true;
	}

	bool onResize(int dw, int dh){
		assert(onCreateCalls);	// at least one onCreate called
		if(onCreateCalls == 1){
			assert(dw == dimensions().w);
			assert(dh == dimensions().h);
		}
		++onResizeCalls;
		return true;
	}

	bool onVisibility(bool v){ return true; }

	bool onKeyDown(const Keyboard& k){ return true; }
	bool onKeyUp(const Keyboard& k){ return true; }
	bool onMouseDown(const Mouse& m){ return true; }
	bool onMouseUp(const Mouse& m){ return true; }
	bool onMouseDrag(const Mouse& m){ return true; }
	bool onMouseMove(const Mouse& m){ return true; }

	bool onFrame(){

		assert(onCreateCalls != 0);
		assert(onResizeCalls != 0);

		assert(created());

		assert(aspect() == defaultDim().aspect());

		assert(defaultTitle() == title());

		Window::Dim dim = dimensions();
		assert(defaultDim().l == dim.l);
		assert(defaultDim().t == dim.t);
		assert(defaultDim().w == dim.w);
		assert(defaultDim().h == dim.h);

		Window::stopLoop();
		return true;
	}
};


int utIOWindowGL(){

	// Do some basic testing of the event handlers
	{
		InputEventHandler ih[3];
		WindowEventHandler wh[3];

		Window win;

		win.remove(win.inputEventHandler());
		win.remove(win.windowEventHandler());

		win.append(ih[1]);
		win.append(ih[2]);
		win.prepend(ih[0]);
		win.append(wh[1]);
		win.append(wh[2]);
		win.prepend(wh[0]);

		for(int i=0; i<3; ++i){
			assert( ih[i].attached());
			assert(&ih[i].window() == &win);

			assert( wh[i].attached());
			assert(&wh[i].window() == &win);
		}

		for(int i=0; i<3; ++i){
			assert(win. inputEventHandlers()[i] == &ih[i]);
			assert(win.windowEventHandlers()[i] == &wh[i]);
		}

		/*Window::InputEventHandlers::const_iterator it = win.inputEventHandlers().begin();
		while(it){
			++it;
		}*/
	}


	MyWindow win;
	win.create(MyWindow::defaultDim(), MyWindow::defaultTitle(), 40);

	Window::startLoop();
	return 0;
}
