#include "allocore/io/al_Window.hpp"

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



Mouse::Mouse(): mX(0), mY(0), mButton(Left){
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
bool Mouse::left() const { return mB[Left]; }
bool Mouse::middle() const { return mB[Middle]; }
bool Mouse::right() const { return mB[Right]; }

void Mouse::button(int b, bool v){ mButton=b; mB[b]=v; if(v){ mBX[b]=mX; mBY[b]=mY; } }
void Mouse::position(int x, int y){ mDX=x-mX; mDY=y-mY; mX=x; mY=y; }



InputEventHandler :: ~InputEventHandler() {
	if (mWindow) mWindow->remove(this);
}

WindowEventHandler :: ~WindowEventHandler() {
	if (mWindow) mWindow->remove(this);
}



void Window::init(){

}

Window& Window::cursorHideToggle(){
	cursorHide(!cursorHide());
	return *this;
}

Window& Window::fullScreenToggle(){
	fullScreen(!fullScreen());
	return *this;
}

Window& Window::add(InputEventHandler * v){
	mInputEventHandlers.push_back(&(v->window(this)));
	return *this;
}

Window& Window::add(WindowEventHandler * v){
	mWindowEventHandlers.push_back(&(v->window(this)));
	return *this;
}

Window& Window::prepend(InputEventHandler * v){
	mInputEventHandlers.insert(mInputEventHandlers.begin(), &(v->window(this)));
	return *this;
}

Window& Window::prepend(WindowEventHandler * v){
	mWindowEventHandlers.insert(mWindowEventHandlers.begin(), &(v->window(this)));
	return *this;
}

Window& Window::remove(InputEventHandler * v){
	// the proper way to do it:
	mInputEventHandlers.erase(std::remove(mInputEventHandlers.begin(), mInputEventHandlers.end(), v), mInputEventHandlers.end());
	v->mWindow = NULL;
	return *this;
}

Window& Window::remove(WindowEventHandler * v){
	// the proper way to do it:
	mWindowEventHandlers.erase(std::remove(mWindowEventHandlers.begin(), mWindowEventHandlers.end(), v), mWindowEventHandlers.end());
	v->mWindow = NULL;
	return *this;
}


} // al::
