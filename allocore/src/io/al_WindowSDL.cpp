#include <stdio.h>		// snprintf
#include <stdlib.h>		// exit
#include <map>
#include "allocore/io/al_Window.hpp"
#include "allocore/system/al_Config.h"		// system defines
#include "allocore/system/al_MainLoop.hpp"	// start/stop loop, rendering
#include "allocore/system/al_Printing.hpp"	// warnings
#include "allocore/graphics/al_OpenGL.hpp"	// OpenGL headers

// SDL API:
// https://wiki.libsdl.org/APIByCategory
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

namespace al{

class WindowImpl{
public:

	typedef std::map<int, WindowImpl *> WindowsMap;

	WindowImpl(Window * w)
	:	mWindow(w)
	{
		resetState();
		Main::get(); // ensure that mainloop exists
	}

	~WindowImpl(){
		destroy();
	}

	void resetState(){
		mScheduled = false;
	}

	bool created() const { return mSDLWindow; }

	unsigned ID() const { return SDL_GetWindowID(mSDLWindow); }

	void makeMainWindow(){
		//glutSetWindow(id());
	}

	void destroy(){ //printf("destroy: %p\n", this);
		if(created()){
			SDL_GL_DeleteContext(mGLContext);
			mGLContext = NULL;
			SDL_DestroyWindow(mSDLWindow);
			mSDLWindow = NULL;
			windows().erase(ID());
			resetState();
		}
	}

/*
	// Returns the implementation of the currently selected window
	static WindowImpl * getWindowImpl(){ return getWindowImpl(glutGetWindow()); }

	static WindowImpl * getWindowImpl(int id){
		WindowsMap::iterator it = windows().find(id);
		if(windows().end() != it){
			return it->second;
		}
		return 0;
	}

	// Returns the currently selected window or 0 if invalid
	static Window * getWindow(){
		WindowImpl * w = getWindowImpl();
		return w ? w->mWindow : 0;
	}
*/
	void scheduleDraw(double deltaTime=0.){
		//if(!mScheduled){
		if(deltaTime == 0){
			scheduleDrawStatic(Main::get().now(), this);
		}
		else if(deltaTime > 0){
			mScheduled = true;
			Main::get().queue().send(Main::get().now()+deltaTime, scheduleDrawStatic, this);
		}
	}

private:
	static const char * errorString(bool verbose){
		GLenum err = glGetError();
		#define CS(GL_ERR, desc) case GL_ERR: return verbose ? #GL_ERR ", " desc : #GL_ERR;
		switch(err){
			case GL_NO_ERROR: return "";
			CS(GL_INVALID_ENUM, "An unacceptable value is specified for an enumerated argument.")
			CS(GL_INVALID_VALUE, "A numeric argument is out of range.")
			CS(GL_INVALID_OPERATION, "The specified operation is not allowed in the current state.")
		#ifdef GL_INVALID_FRAMEBUFFER_OPERATION
			CS(GL_INVALID_FRAMEBUFFER_OPERATION, "The framebuffer object is not complete.")
		#endif
			CS(GL_OUT_OF_MEMORY, "There is not enough memory left to execute the command.")
		#ifdef GL_STACK_OVERFLOW
			CS(GL_STACK_OVERFLOW, "This command would cause a stack overflow.")
		#endif
		#ifdef GL_STACK_UNDERFLOW
			CS(GL_STACK_UNDERFLOW, "This command would cause a stack underflow.")
		#endif
		#ifdef GL_TABLE_TOO_LARGE
			CS(GL_TABLE_TOO_LARGE, "The specified table exceeds the implementation's maximum supported table size.")
		#endif
			default: return "Unknown error code.";
		}
		#undef CS
	}

	void handleEvents(){
		if(!mWindow) return;
		auto * win = mWindow;

		SDL_Event ev;
		//ev.key.repeat == 0 // key was not repeated

		auto setModifiers = [this](SDL_Event ev){
			auto& k = mWindow->mKeyboard;
			k.alt  (ev.key.keysym.mod & KMOD_ALT);
			k.ctrl (ev.key.keysym.mod & KMOD_CTRL);
			k.shift(ev.key.keysym.mod & KMOD_SHIFT);
			//printf("a:%d c:%d s:%d\n", k.alt(), k.ctrl(), k.shift());
		};

		auto sdlToAlloKey = [](int sdlKey){
			int alloKey;
			switch(sdlKey){
			#define CS(v) case SDLK_##v: alloKey = Keyboard::v; break;
			CS(INSERT) CS(LEFT) CS(UP) CS(RIGHT) CS(DOWN) CS(END) CS(HOME)
			CS(F1) CS(F2) CS(F3) CS(F4) CS(F5) CS(F6) CS(F7) CS(F8) CS(F9) CS(F10) CS(F11) CS(F12)
			#undef CS
			case SDLK_PAGEDOWN: alloKey = Keyboard::PAGE_DOWN; break;
			case SDLK_PAGEUP: alloKey = Keyboard::PAGE_UP; break;
			default:; alloKey = sdlKey;
			}
			return alloKey;
		};

		// FIXME: this should be called only once per app as it applies to all windows
		// https://wiki.libsdl.org/SDL_Event
		// https://wiki.libsdl.org/SDL_EventType
		while(SDL_PollEvent(&ev)){

			switch(ev.type){

			case SDL_QUIT:
				//ctx->done = true;
				break;

			case SDL_WINDOWEVENT:
				if(ev.window.windowID == ID()){
					switch(ev.window.event){
					case SDL_WINDOWEVENT_SHOWN:
						//printf("Window %d shown\n", sdlWindowID);
						win->mVisible = true;
						win->callHandlersOnVisibility(win->mVisible);
						win->dimensions(win->dimensions());
						break;
					case SDL_WINDOWEVENT_HIDDEN:
						//printf("Window %d hidden\n", sdlWindowID);
						win->mVisible = false;
						win->callHandlersOnVisibility(win->mVisible);
						break;
					case SDL_WINDOWEVENT_EXPOSED:
						//printf("Window %d exposed\n", sdlWindowID);
						break;
					case SDL_WINDOWEVENT_MOVED:
						//printf("Window %d moved to %d,%d\n", sdlWindowID, ev.window.data1, ev.window.data2);
						win->mDim.l = ev.window.data1;
						win->mDim.t = ev.window.data2;
						break;
					case SDL_WINDOWEVENT_RESIZED:
						//printf("Window %d resized to %dx%d\n", sdlWindowID, ev.window.data1, ev.window.data2);
						win->mDim.w = ev.window.data1;
						win->mDim.h = ev.window.data2;
						win->callHandlersOnResize(win->mDim.w, win->mDim.h);
						break;
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						//printf("Window %d size changed to %dx%d\n",	sdlWindowID, ev.window.data1, ev.window.data2);
						win->mDim.w = ev.window.data1;
						win->mDim.h = ev.window.data2;
						win->callHandlersOnResize(win->mDim.w, win->mDim.h);
						break;
					}
				}
				break;

			case SDL_KEYDOWN:
			case SDL_KEYUP:
			{	bool keyDown = (ev.type == SDL_KEYDOWN);
				//printf("Key %s: %c\n", keyDown?"down":"up", ev.key.keysym.sym);
				win->mKeyboard.setKey(sdlToAlloKey(ev.key.keysym.sym), keyDown);
				setModifiers(ev);
				keyDown ? win->callHandlersOnKeyDown() : win->callHandlersOnKeyUp();
			}	break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{	bool buttonDown = (ev.type == SDL_MOUSEBUTTONDOWN);
				//printf("Mouse %s: %d (%d clicks)\n", buttonDown?"down":"up", ev.button.button, ev.button.clicks);
				auto btn = Mouse::EXTRA;
				switch(ev.button.button){
					case SDL_BUTTON_LEFT:	btn = Mouse::LEFT; break;
					case SDL_BUTTON_MIDDLE:	btn = Mouse::MIDDLE; break;
					case SDL_BUTTON_RIGHT:	btn = Mouse::RIGHT; break;
					//case SDL_BUTTON_X1:
					//case SDL_BUTTON_X2:
					default:; // unrecognized button
				}

				// update modifiers here for shift-mouse etc.
				setModifiers(ev);
				win->mMouse.button(btn, buttonDown);
				buttonDown ? win->callHandlersOnMouseDown() : win->callHandlersOnMouseUp();
			}	break;

			case SDL_MOUSEWHEEL: //printf("Mouse wheel: %d\n", ev.wheel.y);
				break;

			case SDL_MOUSEMOTION: //printf("Mouse motion: %d %d\n", ev.button.x, ev.button.y);
			{	win->mMouse.position(ev.button.x, ev.button.y);
				bool anyButtonDown = false;
				for(int i=0; i<AL_MOUSE_MAX_BUTTONS; ++i) anyButtonDown |= win->mMouse.down(i);
				if(anyButtonDown){
					win->callHandlersOnMouseDrag();
				}else{
					win->callHandlersOnMouseMove();
				}
			}	break;

			default:;
			}
		}
	}

	void onFrame(){
		mWindow->updateFrameTime(); // Compute actual frame interval
		handleEvents();
		mWindow->callHandlersOnFrame();
		const char * err = errorString(true);
		if(err[0]){
			AL_WARN_ONCE("Error after rendering frame in window (id=%d): %s", ID(), err);
		}
		SDL_GL_SwapWindow(mSDLWindow);
	}


	// schedule draws of a specific window
	static void scheduleDrawStatic(al_sec t, WindowImpl * winImpl){

		auto * sdlWin = winImpl->mSDLWindow;
		auto * win = winImpl->mWindow;

		if(sdlWin){
			// This calls the window's onFrame()
			winImpl->onFrame();

			if(win->fps() > 0){

				double waitSec = 0;

				if(!win->asap()){
					al_sec timeNow = win->mFrameTime;
					double FPS = win->fps();

					// Pre-render frame number
					double framePre = timeNow * FPS;

					// Next expected frame number
					double frameNext = (unsigned long long)(framePre + 1.5);
					// Next expected frame time
					double timeNext = frameNext / FPS;

					// Post-render frame time
					double timePost = Window::timeInSec();

					// Did rendering take less time than frame interval?
					// If so, compute wait time...
					if(timePost < timeNext){
						waitSec = timeNext - timePost;
						//printf("num=%lu, wait=%5.3f frames, %2u ms, dt=%5.3f, fps=%5.2f (avg=%5.2f)\n", (unsigned long)(timePost*FPS), wait*FPS, waitMsec, win->mDeltaTime, win->fpsActual(), win->fpsAvg());
					}
					else{
						//printf("dropped frame!\n");
					}
				}
				//printf("Scheduled next draw %g secs from now\n", waitSec);
				winImpl->scheduleDraw(waitSec);
				//SDL_Delay(waitSec*1000 + 0.5); winImpl->scheduleDraw(0);

				/* Crashes
				auto timerCB = [](Uint32 interval, void * user){
					scheduleDrawStatic(0, (WindowImpl *)(user));
					return interval;
				};
				SDL_AddTimer(waitSec*1000 + 0.5, timerCB, winImpl);//*/
			}
			else {
				winImpl->mScheduled = false;
			}
		}
		else {
			//printf("no window\n");
			winImpl->mScheduled = false;
		}
	}

	// Get ID -> created GLUT window map
	static WindowsMap& windows(){
		static WindowsMap* v = new WindowsMap;
		return *v;
	}

	Window * mWindow = NULL;
	SDL_Window * mSDLWindow = NULL;
	SDL_GLContext mGLContext = NULL;
	bool mScheduled = false;

	friend class Window;
};



//******************************************************************************

void Window::implCtor(){
	mImpl = new WindowImpl(this);
}

void Window::implDtor(){
	delete mImpl;
}

void Window::implDestroy(){
	mImpl->destroy();
}

bool Window::implCreate(){
	//printf("Window::create called (in al_WindowSDL.cpp)\n");

	// switch mainloop to default mode:
	Main::get().driver(Main::SLEEP);

	/*if(SDL_InitSubSystem(SDL_INIT_TIMER) < 0){
		printf("SDL ERROR: Failed to init SDL timer subsystem.\n");
		return false;
	}*/

	if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0){
		printf("SDL ERROR: Failed to init SDL video subsystem.\n");
		return false;
	}

	auto sdlWin = SDL_CreateWindow(mTitle.c_str(), mDim.l,mDim.t, mDim.w,mDim.h, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if(!sdlWin){
		printf("SDL ERROR: Could not create window.\n");
		return false;
	}
	mImpl->mSDLWindow = sdlWin;
	WindowImpl::windows()[mImpl->ID()] = mImpl;

	auto glContext = SDL_GL_CreateContext(sdlWin);
	if(!glContext){
		printf("SDL ERROR: Could not create GL context.\n");
	}
	mImpl->mGLContext = glContext;

	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, enabled(DOUBLE_BUF));
	SDL_GL_SetAttribute(SDL_GL_STEREO, enabled(STEREO_BUF));
	if(enabled(MULTISAMPLE)){
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	}

	AL_GRAPHICS_INIT_CONTEXT;
	vsync(mVSync);

	callHandlersOnCreate();

	/*{ // SDL doesn't trigger a resize on window creation, so do it here
		SDL_Event ev;
		ev.type = SDL_WINDOWEVENT_SIZE_CHANGED;
		ev.window.windowID = SDL_GetWindowID(sdlWin);
		ev.window.data1 = mDim.w;
		ev.window.data2 = mDim.h;
		SDL_PushEvent(&ev);
	}*/

	// Set fullscreen according to mFullScreen member
	{	bool fs = fullScreen();
		mFullScreen = false;
		fullScreen(fs);
	}

	#ifdef __EMSCRIPTEN__
	{
		int infiniteLoop = 1;
		int emFPS = asap() ? -1 : fps();
		//void loop_handler(void *arg)
		emscripten_set_main_loop_arg(
			[](void * user){ decltype(mImpl)(user)->onFrame(); }, mImpl, emFPS, infiniteLoop
		);
	}
	#else
		mImpl->scheduleDraw();
	#endif

	return true;
}

void Window::destroyAll(){ //printf("Window::destroyAll\n");
	//Main::get().queue().clear();
	auto it = WindowImpl::windows().begin();
	while(it != WindowImpl::windows().end()){
		if(it->second && it->second->mWindow){
			(it++)->second->mWindow->destroy();
		}
		else{
			++it;
		}
	}
	#ifdef EMSCRIPTEN
		emscripten_cancel_main_loop();
	#endif
	//SDL_Quit();
}

bool Window::created() const {
	return mImpl->created();
}

void Window::implSetDimensions(){
	SDL_SetWindowPosition(mImpl->mSDLWindow, mDim.l, mDim.t);
	SDL_SetWindowSize(mImpl->mSDLWindow, mDim.w, mDim.h);
}

void Window::implSetCursor(){
	/*if(!mCursorHide){
		mImpl->makeMainWindow();
		switch(mCursor){
			case CROSSHAIR:	glutSetCursor(GLUT_CURSOR_CROSSHAIR); break;
			case POINTER:	glutSetCursor(GLUT_CURSOR_INHERIT); break;
			default:;
		}
	}*/
}

void Window::implSetCursorHide(){
	/*mImpl->makeMainWindow();
	if(mCursorHide)	glutSetCursor(GLUT_CURSOR_NONE);
	else			cursor(mCursor);*/
}

void Window::implSetFPS(){
	// Nothing to do here except ensure the scheduler has started.
	// Note: we can safely call scheduleDraw multiple times...
	mImpl->scheduleDraw();
}

void Window::implSetFullScreen(){
	auto err = SDL_SetWindowFullscreen(
		mImpl->mSDLWindow,
		mFullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0
	);
	if(err<0){
		//printf("Error %s fullscreen (err = %d)\n", mFullScreen ? "entering" : "exiting", err);
	}
}

void Window::implSetTitle(){
	SDL_SetWindowTitle(mImpl->mSDLWindow, mTitle.c_str());
}

void Window::implSetVSync(){
	SDL_GL_SetSwapInterval(int(mVSync));
}

Window& Window::hide(){
	SDL_HideWindow(mImpl->mSDLWindow);
	return *this;
}

Window& Window::iconify(){
	SDL_MinimizeWindow(mImpl->mSDLWindow);
	return *this;
}

Window& Window::show(){
	SDL_ShowWindow(mImpl->mSDLWindow);
	return *this;
}

} // al::

