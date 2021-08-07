#include "allocore/io/al_Window.hpp"
#include "allocore/system/al_MainLoop.hpp"	// start/stop loop, rendering
#include "allocore/system/al_Printing.hpp"	// warnings
#include "allocore/graphics/al_OpenGL.hpp"	// OpenGL headers
#include <cstdio>

//#define GLFW_DLL // may be necessary on Windows
#include <GLFW/glfw3.h>

namespace al{

class WindowImpl{
public:

	GLFWwindow * mGLFWWindow = nullptr;
	Window * mWindow = nullptr;
	int lastKeyAction = GLFW_RELEASE;
	int mWindowedW = 100, mWindowedH = 100;

	WindowImpl(Window * win): mWindow(win){
		if(!glfwInit()){
			// something is wrong!
		} else {
			// Use native sleep functions for timing
			Main::get().driver(Main::SLEEP);
		}
	}

	// Call before making GL calls
	void makeContextCurrent(){
		glfwMakeContextCurrent(mGLFWWindow);
	}

	void setCallbacks(){
		glfwSetWindowUserPointer(mGLFWWindow, this);

		#define DECL_REFS(glfwWin)\
			auto& impl = *(WindowImpl*)glfwGetWindowUserPointer(glfwWin);\
			auto& win = *impl.mWindow;

		glfwSetWindowSizeCallback(mGLFWWindow,
			[](GLFWwindow * glfwWin, int w, int h){
				DECL_REFS(glfwWin)
				win.mDim.w = w;
				win.mDim.h = h;
				win.callHandlersOnResize(w, h);
			}
		);

		glfwSetWindowPosCallback(mGLFWWindow,
			[](GLFWwindow* glfwWin, int x, int y){
				DECL_REFS(glfwWin)
				win.mDim.l = x;
				win.mDim.t = y;
				// no al::Window callback
			}
		);

		glfwSetWindowIconifyCallback(mGLFWWindow,
			[](GLFWwindow* glfwWin, int iconified){
				DECL_REFS(glfwWin)
				win.callHandlersOnVisibility(GLFW_FALSE == iconified);
			}
		);

		glfwSetWindowCloseCallback(mGLFWWindow,
			[](GLFWwindow* glfwWin){
			}
		);

		glfwSetKeyCallback(mGLFWWindow,
			/*
			window		The window that received the event.
			key			The keyboard key that was pressed or released.
			scancode	The system-specific scancode of the key.
			action		GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
			mods		Bit field describing which modifier keys were held down.
			*/
			[](GLFWwindow* glfwWin, int key, int scancode, int action, int mods){
				//printf("KeyCallback key:%d scan:%d action:%d\n", key, scancode, action);
				DECL_REFS(glfwWin)

				// Characters map directly to ASCII codes of non-shifted keys on US keyboard:
				// https://www.glfw.org/docs/3.3/group__keys.html#gabf48fcc3afbe69349df432b470c96ef2
				static_assert(GLFW_KEY_A == 'A', "GLFW: mismatch on char code");

				int alloKey = key;
				bool unsupportedKey = false;
				switch(key){
				#define CS2(GLFWKEY, ALLO) case GLFW_KEY_##GLFWKEY: alloKey=ALLO; break;
				#define CS(KEY) CS2(KEY, Keyboard::KEY)
				CS(ENTER) CS(BACKSPACE) CS(TAB)
				//CS2(, Keyboard::RETURN) // none?
				CS(ESCAPE) CS(DELETE)
				CS(F1 ) CS(F2 ) CS(F3 ) CS(F4 ) CS(F5 ) CS(F6 )
				CS(F7 ) CS(F8 ) CS(F9 ) CS(F10) CS(F11) CS(F12)
				CS(INSERT)
				CS(LEFT) CS(RIGHT) CS(UP) CS(DOWN)
				CS(PAGE_DOWN) CS(PAGE_UP) CS(END) CS(HOME)
				default: unsupportedKey = true;
				#undef CS2
				#undef CS
				};

				win.mKeyboard.shift(mods & GLFW_MOD_SHIFT);
				win.mKeyboard.ctrl(mods & GLFW_MOD_CONTROL);
				win.mKeyboard.alt(mods & GLFW_MOD_ALT);
				win.mKeyboard.meta(mods & GLFW_MOD_SUPER);
				win.mKeyboard.caps(mods & GLFW_MOD_CAPS_LOCK);

				impl.lastKeyAction = action;

				if(GLFW_REPEAT != action){
					auto down = GLFW_PRESS == action;
					win.mKeyboard.setKey(alloKey, down);
					if(!unsupportedKey){
						if(down) win.callHandlersOnKeyDown();
						else win.callHandlersOnKeyUp();
					}
				}
			}
		);

		glfwSetCharCallback(mGLFWWindow,
			[](GLFWwindow* glfwWin, unsigned int codepoint){
				//printf("CharCallback codepoint:%d\n", codepoint);
				DECL_REFS(glfwWin)
				// Fortunately the key callback is called first which sets the button state (up or down). Thus, we can just retrieve it from the Keyboard.
				if(GLFW_REPEAT != impl.lastKeyAction){
					//auto down = win.mKeyboard.down();
					auto down = GLFW_PRESS == impl.lastKeyAction;
					win.mKeyboard.setKey(codepoint, down);
					if(down) win.callHandlersOnKeyDown();
					else win.callHandlersOnKeyUp();
				}
			}
		);

		glfwSetMouseButtonCallback(mGLFWWindow,
			[](GLFWwindow * glfwWin, int button, int action, int mods){
				//printf("mouse: %d %d %d\n", button, action, mods);
				DECL_REFS(glfwWin)
				Mouse& m = win.mMouse;
				int b = Mouse::EXTRA;
				if(GLFW_MOUSE_BUTTON_LEFT == button) b = Mouse::LEFT;
				else if(GLFW_MOUSE_BUTTON_RIGHT == button) b = Mouse::RIGHT;
				else if(GLFW_MOUSE_BUTTON_MIDDLE == button) b = Mouse::MIDDLE;
				if(GLFW_PRESS == action){
					m.button(b, true);
					win.callHandlersOnMouseDown();
				}
				else if(GLFW_RELEASE == action){
					m.button(b, false);
					win.callHandlersOnMouseUp();
				}

			}
		);

		glfwSetCursorPosCallback(mGLFWWindow,
			[](GLFWwindow * glfwWin, double x, double y){
				DECL_REFS(glfwWin)
				Mouse& m = win.mMouse;
				m.position(x, y);
				if(m.any())	win.callHandlersOnMouseDrag();
				else		win.callHandlersOnMouseMove();
			}
		);

		glfwSetDropCallback(mGLFWWindow,
			[](GLFWwindow* glfwWin, int count, const char** paths){
				//for(int i=0; i<count; ++i) printf("Dropped file %s\n", paths[i]);
			}
		);
	}

	void onFrame(){
		mWindow->updateFrameTime(); // Compute actual frame interval
		makeContextCurrent();
		mWindow->callHandlersOnFrame();
		/*const char * err = errorString(true);
		if(err[0]){
			AL_WARN_ONCE("Error after rendering frame in window (id=%d): %s", ID(), err);
		}*/
		glfwSwapBuffers(mGLFWWindow);
		glfwPollEvents();
	}
};


static void scheduleDraw(al_sec t, double deltaTime, WindowImpl * winImpl){
	winImpl->onFrame();
	Main::get().queue().send(Main::get().now()+deltaTime, scheduleDraw, 0.1, winImpl);
}


void Window::implCtor(){
	mImpl = new WindowImpl(this);
}

void Window::implDtor(){
	delete mImpl;
}

void Window::implDestroy(){
	glfwDestroyWindow(mImpl->mGLFWWindow);
}

bool Window::implCreate(){

	glfwWindowHint(GLFW_DOUBLEBUFFER, enabled(DOUBLE_BUF) ? GLFW_TRUE : GLFW_FALSE);
	//glfwWindowHint(GLFW_RED_BITS, 8);
	//glfwWindowHint(GLFW_GREEN_BITS, 8);
	//glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, enabled(ALPHA_BUF) ? 8 : 0);
	glfwWindowHint(GLFW_DEPTH_BITS, enabled(DEPTH_BUF) ? 24 : 0);
	glfwWindowHint(GLFW_STENCIL_BITS, enabled(STENCIL_BUF) ? GLFW_DONT_CARE : 0);
	glfwWindowHint(GLFW_SAMPLES, enabled(MULTISAMPLE) ? 4 : 0);
	glfwWindowHint(GLFW_STEREO, enabled(STEREO_BUF) ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_FALSE);

	// Set 1x1 so that implSetDimensions() triggers onResize (0x0 does not work!)
	auto * glfwWin = glfwCreateWindow(1, 1, mTitle.c_str(), NULL, NULL);

	if(glfwWin){
		mImpl->mGLFWWindow = glfwWin;
		mImpl->setCallbacks();
		mImpl->mWindowedW = mDim.w;
		mImpl->mWindowedH = mDim.h;
		implSetDimensions();
		scheduleDraw(Main::get().now(), 0, mImpl);
		return true;
	}

	return false;
}

void Window::destroyAll(){ //printf("Window::destroyAll\n");
	/*WindowImpl::WindowsMap::iterator it = WindowImpl::windows().begin();
	while(it != WindowImpl::windows().end()){
		if(it->second && it->second->mWindow){
			(it++)->second->mWindow->destroy();
		}
		else{
			++it;
		}
	}*/
}

bool Window::created() const {
	return mImpl->mGLFWWindow;
}

void Window::implSetDimensions(){
	//printf("implSetDimensions l:%d t:%d w:%d h:%d\n", mDim.l, mDim.t, mDim.w, mDim.h);
	glfwSetWindowPos(mImpl->mGLFWWindow, mDim.l, mDim.t);
	glfwSetWindowSize(mImpl->mGLFWWindow, mDim.w, mDim.h);
}

void Window::implSetCursor(){
}

void Window::implSetCursorHide(){
	glfwSetInputMode(mImpl->mGLFWWindow, GLFW_CURSOR,
		mCursorHide ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL
	);
}

void Window::implSetFPS(){
}

void Window::implSetFullScreen(){
	if(mFullScreen){
		//auto * mon = glfwGetPrimaryMonitor();
		auto * mon = glfwGetWindowMonitor(mImpl->mGLFWWindow);
		const auto * mode = glfwGetVideoMode(mon);
		mImpl->mWindowedW = mDim.w;
		mImpl->mWindowedH = mDim.h;
		glfwSetWindowMonitor(mImpl->mGLFWWindow, mon, 0, 0, mode->width, mode->height, mode->refreshRate);
		vsync(mVSync);
	} else {
		glfwSetWindowMonitor(mImpl->mGLFWWindow, NULL, mDim.l, mDim.t, mImpl->mWindowedW, mImpl->mWindowedH, GLFW_DONT_CARE);
		vsync(mVSync);
	}
}

void Window::implSetTitle(){
	glfwSetWindowTitle(mImpl->mGLFWWindow, mTitle.c_str());
}

void Window::implSetVSync(){
	mImpl->makeContextCurrent(); // must do this
	glfwSwapInterval(int(mVSync));
}

Window& Window::hide(){
	glfwHideWindow(mImpl->mGLFWWindow);
	return *this;
}

Window& Window::iconify(){
	glfwIconifyWindow(mImpl->mGLFWWindow);
	return *this;
}

Window& Window::show(){
	glfwShowWindow(mImpl->mGLFWWindow);
	return *this;
}


} // al::
