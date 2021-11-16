#include <algorithm> // find
#include <cstdio>
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

bool Mouse::any() const {
	for(auto b : mB){ if(b) return true; }
	return false;
}

void Mouse::button(int b, bool v){ mButton=b; mB[b]=v; if(v){ mBX[b]=mX; mBY[b]=mY; } }
void Mouse::position(int x, int y){ mDX=x-mX; mDY=y-mY; mX=x; mY=y; }


InputEventHandler::InputEventHandler()
:	mWindow(NULL)
{}

InputEventHandler::~InputEventHandler(){
	removeFromWindow();
}

void InputEventHandler::removeFromWindow(){
	if(attached()) window().remove(*this);
}



WindowEventHandler::WindowEventHandler()
:	mWindow(NULL)
{}

WindowEventHandler::~WindowEventHandler(){
	removeFromWindow();
}

void WindowEventHandler::removeFromWindow(){
	if(attached()){
		window().remove(*this); // Window::remove calls onResize
	}
}



Window::Window()
:	mDim(0,0,0,0), mDisplayMode(DEFAULT_BUF), mCursor(POINTER),
	mFPS(0), mFPSAvg(0), mFrameTime(0), mDeltaTime(0),
	mASAP(false), mCursorHide(false), mFullScreen(false),
	mVisible(false), mVSync(true)
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


void Window::updateFrameTime(){
	double timeNow = timeInSec();
	mDeltaTime = timeNow - mFrameTime;
	mFrameTime = timeNow;

	// Average frame-rate calculation:
	double fpsCurr = 1./mDeltaTime;
	mFPSAvg += 0.3 * (fpsCurr - mFPSAvg);
}


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
