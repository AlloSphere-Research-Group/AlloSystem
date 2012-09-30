#include "allocore/io/al_Window.hpp"
#include "allocore/system/al_MainLoop.hpp"
#include <stdio.h>

namespace al{

Keyboard::Keyboard()
:	mKeycode(-1), mDown(false)
{
	for(int i=0; i<5; ++i) mModifiers[i] = false;
}

int Keyboard::key() const { return mKeycode; }
int Keyboard::keyAsNumber() const { return key() - 48; }
bool Keyboard::down() const { return mDown; }
bool Keyboard::isNumber() const { return (key() >= '0') && (key() <= '9'); }
bool Keyboard::alt()   const { return mModifiers[1]; }
bool Keyboard::caps()  const { return mModifiers[3]; }
bool Keyboard::ctrl()  const { return mModifiers[2]; }
bool Keyboard::meta()  const { return mModifiers[4]; }
bool Keyboard::shift() const { return mModifiers[0]; }
bool Keyboard::key(int k) const { return mKeycode == k; }
void Keyboard::alt  (bool state){mModifiers[1] = state;}
void Keyboard::caps (bool state){mModifiers[3] = state;}
void Keyboard::ctrl (bool state){mModifiers[2] = state;}
void Keyboard::meta (bool state){mModifiers[4] = state;}
void Keyboard::shift(bool state){mModifiers[0] = state;}
void Keyboard::setKey(int k, bool v){ mKeycode=k; mDown=v; }



Mouse::Mouse(): mX(0), mY(0), mButton(LEFT){
	for(int i=0; i<AL_MOUSE_MAX_BUTTONS; ++i){
		mBX[i] = mBY[i] = 0; mB[i] = false;
	}
}

int Mouse::x() const { return mX; }
int Mouse::y() const { return mY; }
int Mouse::dx() const { return mDX; }
int Mouse::dy() const { return mDY; }

int Mouse::button() const { return mButton; }
bool Mouse::down() const { return down(mButton); }
bool Mouse::down(int button) const { return mB[button]; }
bool Mouse::left() const { return mB[LEFT]; }
bool Mouse::middle() const { return mB[MIDDLE]; }
bool Mouse::right() const { return mB[RIGHT]; }

void Mouse::button(int b, bool v){ mButton=b; mB[b]=v; if(v){ mBX[b]=mX; mBY[b]=mY; } }
void Mouse::position(int x, int y){ mDX=x-mX; mDY=y-mY; mX=x; mY=y; }


InputEventHandler::InputEventHandler()
:	mWindow(NULL)
{}

InputEventHandler::~InputEventHandler(){
	removeFromWindow();
}

void InputEventHandler::removeFromWindow(){
	if(attached()) window().remove(this);
}



WindowEventHandler::WindowEventHandler()
:	mWindow(NULL)
{}

WindowEventHandler::~WindowEventHandler(){
	removeFromWindow();
}

void WindowEventHandler::removeFromWindow(){
	if(attached()){
		window().remove(this); // Window::remove calls onResize
	}
}



Window::Window()
:	mDisplayMode(DEFAULT_BUF), mASAP(false), mVSync(true)
{
	implCtor(); // must call first!
	dimensions(Dim(800,600));
	fps(40);
	append(inputEventHandler());
	append(windowEventHandler());
}

Window::~Window(){
	destroy();
	implDtor();
}

void Window::destroy(){
	if(created()){
		callHandlersOnDestroy();
		implDestroy();
	}
}

//void Window::init(){
//	// Window has its own built-in handlers (which may be overridden in subclasses)
//	// they are added explicitly here so that the order of handlers can be user controled
//	append(inputEventHandler());
//	append(windowEventHandler());
//	mDisplayMode = DEFAULT_BUF;
//	mASAP = false;
//}

Window& Window::cursorHideToggle(){
	cursorHide(!cursorHide());
	return *this;
}

Window::DisplayMode Window::displayMode() const { return mDisplayMode; }

Window& Window::displayMode(DisplayMode v){
	if(created()){
		const Cursor cursor_ = cursor();
		const bool cursorHide_ = cursorHide();
		const Dim dim_ = dimensions();
		const bool fullScreen_ = fullScreen();
		const double fps_ = fps();
		const std::string& title_ = title();

		destroy();
		create(dim_, title_, fps_, v);
		cursor(cursor_);
		cursorHide(cursorHide_);
		fullScreen(fullScreen_);
	}
	else{
		mDisplayMode = v;
	}
	return *this;
}

bool Window::enabled(DisplayMode v) const { return mDisplayMode & v; }

Window& Window::fullScreenToggle(){
	fullScreen(!fullScreen());
	return *this;
}

//double Window::spfActual() const { return Main::get().intervalActual(); }


Window& Window::insert(InputEventHandler& v, int i){
	InputEventHandlers& H = mInputEventHandlers;
	if(std::find(H.begin(), H.end(), &v) == H.end()){
		v.removeFromWindow();
		H.insert(H.begin()+i, &(v.window(this)));
	}
	return *this;
}

Window& Window::insert(WindowEventHandler& v, int i){
	WindowEventHandlers& H = mWindowEventHandlers;
	if(std::find(H.begin(), H.end(), &v) == H.end()){
		v.removeFromWindow();
		H.insert(H.begin()+i, &(v.window(this)));
		
		// notify new handler of changes if the window already is created
		// otherwise, the window will call the proper handlers when created
		if(created()){
			//printf("onCreate for new addition to existing window\n");
			v.onCreate();
		}
		if(started()){
			v.onResize(width(), height());
			//printf("WindowEventHandler %p onResize(%d, %d)\n", &v, width(), height());
		}
	}
	return *this;
}

Window& Window::append(InputEventHandler& v){ return insert(v, mInputEventHandlers.size()); }
Window& Window::append(WindowEventHandler& v){ return insert(v, mWindowEventHandlers.size()); }
Window& Window::prepend(InputEventHandler& v){ return insert(v,0); }
Window& Window::prepend(WindowEventHandler& v){ return insert(v,0); }


Window& Window::remove(InputEventHandler& v){
	InputEventHandlers& H = mInputEventHandlers;
	InputEventHandlers::iterator it = std::find(H.begin(), H.end(), &v);

	if(it != H.end()){
		H.erase(it);

		// the proper way to do it:
		//H.erase(std::remove(H.begin(), H.end(), &v), H.end());
		v.mWindow = NULL;
	}
	return *this;
}

Window& Window::remove(WindowEventHandler& v){
	WindowEventHandlers& H = mWindowEventHandlers;
	WindowEventHandlers::iterator it = std::find(H.begin(), H.end(), &v);

	if(it != H.end()){

		H.erase(it);

		// the proper way to do it:
		//H.erase(std::remove(H.begin(), H.end(), &v), H.end());

		//printf("removed window event handler (%p) from window (%p)\n", &v, this);
		//assert(std::find(H.begin(), H.end(), &v) == H.end());
		
		if(started()){
			v.onResize(-width(), -height());
			//printf("WindowEventHandler %p onResize(%d, %d)\n", &v, width(), height());
		}
		if(created()){
			v.onDestroy();
		}
		v.mWindow = NULL;
	}
	return *this;
}


bool Window::started(){
	return Main::get().isRunning();
}

void Window::startLoop(){
	Main::get().start();
}

void Window::stopLoop(){
	Window::destroyAll();
	Main::get().stop();
}


} // al::
