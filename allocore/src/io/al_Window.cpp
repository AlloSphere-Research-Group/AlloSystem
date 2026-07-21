#include <algorithm> // find
#include <cstdio> // printf
#include "allocore/io/al_Window.hpp"
#include "allocore/system/al_MainLoop.hpp"
#include "allocore/system/al_Time.h" // al_steady_time

namespace al{

Keyboard::Keyboard()
:	mKeycode(-1), mDown(false), mCaps(false), mModifiers(0)
{
}

int Keyboard::key() const { return mKeycode; }
int Keyboard::keyAsNumber() const { return key() - 48; }
bool Keyboard::down() const { return mDown; }
bool Keyboard::isNumber() const { return (key() >= '0') && (key() <= '9'); }
bool Keyboard::alt()   const { return mModifiers & ALT; }

bool Keyboard::ctrl()  const { return mModifiers & CTRL; }
bool Keyboard::meta()  const { return mModifiers & META; }
bool Keyboard::shift() const { return mModifiers & SHIFT; }
bool Keyboard::caps()  const { return mCaps; }
unsigned char Keyboard::modifiers() const { return mModifiers; }
bool Keyboard::key(int k) const { return mKeycode == k; }

void setBit(unsigned char& bits, unsigned char mask, bool val){
	if(val)	bits |=  mask;
	else	bits &= ~mask;
}
void Keyboard::alt  (bool state){ setBit(mModifiers, ALT  , state); }
void Keyboard::ctrl (bool state){ setBit(mModifiers, CTRL , state); }
void Keyboard::meta (bool state){ setBit(mModifiers, META , state); }
void Keyboard::shift(bool state){ setBit(mModifiers, SHIFT, state); }
void Keyboard::caps (bool state){ mCaps = state; }
void Keyboard::setKey(int k, bool v){
	mKeycode=k; mDown=v;
	mEvents.push_back({k,v});
	if(mEvents.size() >= 64) mEvents.pop_front();
}

void Keyboard::print() const {
	fprintf(stderr,
		"key=%3d (%c), alt=%i, ctrl=%i, meta=%i, shift=%i, caps=%i\n",
		key(),key(), alt(), ctrl(), meta(), shift(), caps()
	);
}


Mouse::Mouse(): mX(0), mY(0), mDX(0), mDY(0), mButton(LEFT), mWheelX(0), mWheelY(0){
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

bool Mouse::any() const {
	for(auto b : mB){ if(b) return true; }
	return false;
}

short Mouse::wheelX() const { return mWheelX; }
short Mouse::wheelY() const { return mWheelY; }

void Mouse::position(int x, int y){ mDX=x-mX; mDY=y-mY; mX=x; mY=y; }
void Mouse::button(int b, bool v){ mButton=b; mB[b]=v; if(v){ mBX[b]=mX; mBY[b]=mY; } }
void Mouse::wheel(short dx, short dy){ mWheelX=dx; mWheelY=dy; }



EventHandler::~EventHandler(){
	//removeFromWindow(); // no, no since it calls a virtual
}

void EventHandler::attach(Window& win){
	if(attached()){
		removeFromWindow();
	}
	mWindow = &win;
	if(active()) onAttach();
}

void EventHandler::detach(){
	if(attached()){
		if(active()) onDetach();
		mWindow = nullptr;
	}
}

void EventHandler::removeFromWindow(){
	/* Expected call stack:
		this->onRemoveFromWindow
			Window::remove
				this->detach
	*/
	if(attached()){
		onRemoveFromWindow();
	}
}


InputEventHandler::~InputEventHandler(){
	removeFromWindow();
}

void InputEventHandler::onRemoveFromWindow(){
	window().remove(*this);
}


WindowEventHandler::~WindowEventHandler(){
	removeFromWindow();
}

void WindowEventHandler::onAttach(){
	auto& win = window();
	// notify new handler of changes if the window already is created
	// otherwise, the window will call the proper handlers when created
	if(win.created()){
		//printf("onCreate for new addition to existing window\n");
		onCreate();
	}
	if(win.started()){
		onResize(win.width(), win.height());
		//printf("WindowEventHandler %p onResize(%d, %d)\n", this, win.width(), win.height());
	}
}

void WindowEventHandler::onDetach(){
	auto& win = window();
	if(win.started()){
		onResize(-win.width(), -win.height());
		//printf("WindowEventHandler %p onResize(%d, %d)\n", this, win.width(), win.height());
	}
	if(win.created()){
		onDestroy();
	}
}

void WindowEventHandler::onRemoveFromWindow(){
	window().remove(*this);
}



Window::Dim::Dim(int v):Window::Dim(v,v){}
Window::Dim::Dim(int w, int h): Window::Dim(0,0,w,h){}
Window::Dim::Dim(int l_, int t_, int w_, int h_): l(l_), t(t_), w(w_), h(h_){}

void Window::Dim::set(int l_, int t_, int w_, int h_){
	l=l_; t=t_; w=w_; h=h_;
}

float Window::Dim::aspect() const {
	return (w!=0 && h!=0) ? double(w)/h : 1;
}

void Window::Dim::print() const {
	printf("Dim: %4d x %4d @ (%4d, %4d)\n", w,h, l,t);
}


Window::Window(){
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

double Window::timeInSec(){
	return al_steady_time();
}

bool Window::create(
	const Dim& dim, const std::string& title, double fps, DisplayMode mode
){
	if(!created()){
		mDim = dim;
		mTitle = title;
		mFPS = fps;
		mDisplayMode = mode;
		mFrameTime = timeInSec();

		if(implCreate()){
			return true;
		}
	}

	return false;
}

void Window::destroy(){
	if(created()){
		callHandlersOnDestroy();
		implDestroy();
	}
}

bool Window::asap() const {
	return mASAP;
}

Window& Window::asap(bool v){
	mASAP=v;
	return *this;
}

double Window::aspect() const {
	return dimensions().aspect();
}

Window& Window::cursorHideToggle(){
	cursorHide(!cursorHide());
	return *this;
}

Window::Cursor Window::cursor() const {
	return mCursor;
}

Window& Window::cursor(Cursor v){
	mCursor = v;
	if(created()) implSetCursor();
	return *this;
}

bool Window::cursorHide() const {
	return mCursorHide;
}

Window& Window::cursorHide(bool v){
	mCursorHide = v;
	if(created()) implSetCursorHide();
	return *this;
}

bool Window::keyRepeat() const {
	return mKeyRepeat;
}

Window& Window::keyRepeat(bool v){
	mKeyRepeat = v;
	if(created()) implSetKeyRepeat();
	return *this;
}

Window::Dim Window::dimensions() const {
	return mDim;
}

Window& Window::dimensions(const Dim& v){
	mDim = v;
	if(created()) implSetDimensions();
	return *this;
}

Window::DisplayMode Window::displayMode() const {
	return mDisplayMode;
}

Window& Window::displayMode(DisplayMode v){
	if(mDisplayMode != v){
		if(created()){
			const auto cursor_ = cursor();
			const auto cursorHide_ = cursorHide();
			const auto dim_ = dimensions();
			const auto fullScreen_ = fullScreen();
			const auto fps_ = fps();
			const auto& title_ = title();

			destroy();
			create(dim_, title_, fps_, v);
			cursor(cursor_);
			cursorHide(cursorHide_);
			fullScreen(fullScreen_);
		}
		else{
			mDisplayMode = v;
		}
	}
	return *this;
}

double Window::fps() const {
	return mFPS;
}

Window& Window::fps(double v){
	if(v != mFPS && v > 0){
		mFPS = v;
		if(created()) implSetFPS();
	}
	return *this;
}

double Window::fpsActual() const {
	return 1./spfActual();
}

double Window::fpsAvg() const {
	return mFPSAvg;
}

bool Window::fullScreen() const {
	return mFullScreen;
}

Window& Window::fullScreen(bool v){
	if(v != mFullScreen){
		mFullScreen = v;
		if(created()) implSetFullScreen();
	}
	return *this;
}

Window& Window::fullScreenToggle(){
	fullScreen(!fullScreen());
	return *this;
}

const std::string& Window::title() const {
	return mTitle;
}

Window& Window::title(const std::string& v){
	mTitle = v;
	if(created()) implSetTitle();
	return *this;
}

double Window::spf() const {
	return 1./fps();
}

double Window::spfActual() const {
	return mDeltaTime;
}

//double Window::spfActual() const { return Main::get().intervalActual(); }

bool Window::visible() const {
	return mVisible;
}

bool Window::vsync() const {
	return mVSync;
}

Window& Window::vsync(bool v){
	mVSync = v;
	if(created()) implSetVSync();
	return *this;
}


bool Window::enabled(DisplayMode v) const {
	return mDisplayMode & v;
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

void Window::updateFrameTime(){
	double timeNow = timeInSec();
	mDeltaTime = timeNow - mFrameTime;
	mFrameTime = timeNow;

	// Average frame-rate calculation:
	double fpsCurr = 1./mDeltaTime;
	mFPSAvg += 0.3 * (fpsCurr - mFPSAvg);
}

template<> Window::EventHandlers<InputEventHandler>& Window::eventHandlers<InputEventHandler>(){
	return mInputEventHandlers;
}

template<> Window::EventHandlers<WindowEventHandler>& Window::eventHandlers<WindowEventHandler>(){
	return mWindowEventHandlers;
}

template <class TEventHandler>
Window& Window::removeT(TEventHandler& v){
	auto& H = eventHandlers<TEventHandler>();
	auto it = std::find(H.begin(), H.end(), &v);
	if(it != H.end()){
		H.erase(it);
		// the proper way to do it:
		//H.erase(std::remove(H.begin(), H.end(), &v), H.end());
		v.detach();
	}
	return *this;
}

Window& Window::remove(InputEventHandler& v){ return removeT(v); }
Window& Window::remove(WindowEventHandler& v){ return removeT(v); }

template <class TEventHandler>
Window& Window::insert(TEventHandler& v, int i){
	auto& H = eventHandlers<TEventHandler>();
	if(std::find(H.begin(), H.end(), &v) == H.end()){
		v.attach(*this);
		H.insert(H.begin()+i, &v);
	}
	return *this;
}

// Explicit template instantiations not needed since only used internally...
//template Window& Window::insert<InputEventHandler>(InputEventHandler&, int);
//template Window& Window::insert<WindowEventHandler>(WindowEventHandler&, int);

Window& Window::append(InputEventHandler& v){ return insert(v, mInputEventHandlers.size()); }
Window& Window::append(WindowEventHandler& v){ return insert(v, mWindowEventHandlers.size()); }
Window& Window::prepend(InputEventHandler& v){ return insert(v,0); }
Window& Window::prepend(WindowEventHandler& v){ return insert(v,0); }


#define CALL_HANDLERS(handlers, func){\
	for(auto * handler : handlers){\
		if(!handler->active()) continue;\
		if(false == handler->func) break;\
	}\
}

#define CALL(func) CALL_HANDLERS(mInputEventHandlers, func)
void Window::callHandlersOnMouseDown(){ CALL(onMouseDown(mMouse)); }
void Window::callHandlersOnMouseDrag(){ CALL(onMouseDrag(mMouse)); }
void Window::callHandlersOnMouseMove(){ CALL(onMouseMove(mMouse)); }
void Window::callHandlersOnMouseUp(){ CALL(onMouseUp(mMouse)); }
void Window::callHandlersOnMouseWheel(){ CALL(onMouseWheel(mMouse)); }
void Window::callHandlersOnKeyDown(){ CALL(onKeyDown(mKeyboard)); }
void Window::callHandlersOnKeyUp(){ CALL(onKeyUp(mKeyboard)); }
#undef CALL

#define CALL(func) CALL_HANDLERS(mWindowEventHandlers, func)
void Window::callHandlersOnFrame(){
	CALL(onFrame());
	mKeyboard.mEvents.clear();
	mMouse.wheel(0,0);
}
void Window::callHandlersOnCreate(){
	contextCreate();
	CALL(onCreate());
}
void Window::callHandlersOnDestroy(){
	CALL(onDestroy());
	contextDestroy();
}
void Window::callHandlersOnResize(int w, int h){ CALL(onResize(w, h)); }
void Window::callHandlersOnVisibility(bool v){ CALL(onVisibility(v)); }
void Window::callHandlersOnDrop(const std::vector<std::string>& paths){ CALL(onDrop(paths)); }
#undef CALL


bool StandardWindowKeyControls::onKeyDown(const Keyboard& k){
	if(k.ctrl()){
		switch(k.key()){
			case 'q': Window::stopLoop(); return false;
			//case 'w': window().destroy(); return false;
			case 'h': window().hide(); return false;
			case 'm': window().iconify(); return false;
			default:;
		}
	}
	else if(k.alt()){
		switch(k.key()){
			case Keyboard::F4: Window::stopLoop(); return false;
			default:;
		}
	}
	else{
		switch(k.key()){
			case Keyboard::ESCAPE: window().fullScreenToggle(); return false;
			default:;
		}
	}
	return true;
}

} // al::
