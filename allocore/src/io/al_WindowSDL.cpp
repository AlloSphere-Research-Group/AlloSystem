#include <map>
#include "allocore/io/al_Window.hpp"
#include "allocore/system/al_Config.h"		// system defines
#include "allocore/system/al_MainLoop.hpp"	// start/stop loop, rendering
#include "allocore/system/al_Printing.hpp"	// warnings
#include "allocore/graphics/al_OpenGL.hpp"	// OpenGL headers
#include "al_SDL.hpp"

// SDL API:
// https://wiki.libsdl.org/APIByCategory

#ifdef AL_EMSCRIPTEN
	#include <emscripten.h>
	#include <emscripten/html5.h> // emscripten_request_fullscreen, emscripten_exit_fullscreen
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
			#ifdef AL_EMSCRIPTEN
			Main::get().remove(mMainHandler);
			#endif
			#ifdef USING_SDL3
				SDL_GL_DestroyContext(mGLContext);
			#elif defined USING_SDL2
				SDL_GL_DeleteContext(mGLContext);
			#endif
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
		#ifdef AL_EMSCRIPTEN
		// Main will call onFrame via handler's onTick
		Main::get().add(mMainHandler);
		Main::get().interval(deltaTime);
		#else
		//if(!mScheduled){
		if(deltaTime == 0){
			scheduleDrawStatic(Main::get().now(), this);
		}
		else if(deltaTime > 0){
			mScheduled = true;
			Main::get().queue().send(Main::get().now()+deltaTime, scheduleDrawStatic, this);
		}
		#endif
	}

private:

	void handleEvents(){
		if(!mWindow) return;
		auto& win = *mWindow;

		SDL_Event ev;
		//ev.key.repeat == 0 // key was not repeated

		std::vector<std::string> dropPaths;

		auto sdlToAlloKey = [](int sdlKey) -> int {
			switch(sdlKey){
			#define CS(v) case SDLK_##v: return Keyboard::v;
			CS(INSERT) CS(LEFT) CS(UP) CS(RIGHT) CS(DOWN) CS(END) CS(HOME)
			CS(F1) CS(F2) CS(F3) CS(F4) CS(F5) CS(F6) CS(F7) CS(F8) CS(F9) CS(F10) CS(F11) CS(F12)
			#undef CS
			#define CS(KEY,allo) case SDLK_##KEY: return Keyboard::allo;
			CS(PAGEDOWN, PAGE_DOWN) CS(PAGEUP, PAGE_UP)
			#undef CS
			default: return sdlKey;
			}
		};

		// FIXME: this should be called only once per app as it applies to all windows
		// https://wiki.libsdl.org/SDL_Event
		// https://wiki.libsdl.org/SDL_EventType
		while(SDL_PollEvent(&ev)){

			//printf("SDL event %d\n", ev.type);

			// https://github.com/libsdl-org/SDL/blob/SDL2/include/SDL_events.h
			// https://github.com/libsdl-org/SDL/blob/main/include/SDL3/SDL_events.h
			#ifdef USING_SDL3
				#define EV_QUIT 			SDL_EVENT_QUIT
				#define EV_WIN_CLOSE		SDL_EVENT_WINDOW_CLOSE_REQUESTED
				#define EV_WIN_SHOW			SDL_EVENT_WINDOW_SHOWN
				#define EV_WIN_HIDE			SDL_EVENT_WINDOW_HIDDEN
				#define EV_WIN_EXPOSE		SDL_EVENT_WINDOW_EXPOSED
				#define EV_WIN_MOVE			SDL_EVENT_WINDOW_MOVED
				#define EV_WIN_RESIZE		SDL_EVENT_WINDOW_RESIZED
				#define EV_WIN_SIZE_CHANGE	SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED
				#define EV_DROP_FILE		SDL_EVENT_DROP_FILE
				#define EV_KEY_DOWN			SDL_EVENT_KEY_DOWN
				#define EV_KEY_UP			SDL_EVENT_KEY_UP
				#define EV_TEXT_INPUT		SDL_EVENT_TEXT_INPUT
				#define EV_MOUSE_DOWN		SDL_EVENT_MOUSE_BUTTON_DOWN
				#define EV_MOUSE_UP			SDL_EVENT_MOUSE_BUTTON_UP
				#define EV_MOUSE_WHEEL		SDL_EVENT_MOUSE_WHEEL
				#define EV_MOUSE_MOTION		SDL_EVENT_MOUSE_MOTION
				#define EV_FINGER_DOWN		SDL_EVENT_FINGER_DOWN
				#define EV_FINGER_UP		SDL_EVENT_FINGER_UP
				#define EV_FINGER_MOTION	SDL_EVENT_FINGER_MOTION
				#define K_ALT				SDL_KMOD_ALT
				#define K_CTL				SDL_KMOD_CTRL
				#define K_SHF				SDL_KMOD_SHIFT

			#elif defined USING_SDL2
				#define EV_QUIT				SDL_QUIT
				#define EV_WIN_CLOSE		SDL_WINDOWEVENT_CLOSE
				#define EV_WIN_SHOW			SDL_WINDOWEVENT_SHOWN
				#define EV_WIN_HIDE			SDL_WINDOWEVENT_HIDDEN
				#define EV_WIN_EXPOSE		SDL_WINDOWEVENT_EXPOSED
				#define EV_WIN_MOVE			SDL_WINDOWEVENT_MOVED
				#define EV_WIN_RESIZE		SDL_WINDOWEVENT_RESIZED
				#define EV_WIN_SIZE_CHANGE	SDL_WINDOWEVENT_SIZE_CHANGED
				#define EV_DROP_FILE		SDL_DROPFILE
				#define EV_KEY_DOWN			SDL_KEYDOWN
				#define EV_KEY_UP			SDL_KEYUP
				#define EV_TEXT_INPUT		SDL_TEXTINPUT
				#define EV_MOUSE_DOWN		SDL_MOUSEBUTTONDOWN
				#define EV_MOUSE_UP			SDL_MOUSEBUTTONUP
				#define EV_MOUSE_WHEEL		SDL_MOUSEWHEEL
				#define EV_MOUSE_MOTION		SDL_MOUSEMOTION
				#define EV_FINGER_DOWN		SDL_FINGERDOWN
				#define EV_FINGER_UP		SDL_FINGERUP
				#define EV_FINGER_MOTION	SDL_FINGERMOTION
				#define K_ALT				KMOD_ALT
				#define K_CTL				KMOD_CTRL
				#define K_SHF				KMOD_SHIFT
			#endif

			switch(ev.type){

			case EV_QUIT:
				//ctx->done = true;
				break;

			case EV_DROP_FILE:{ // called multiple times per frame if multi-file drop
				#ifdef USING_SDL3
					dropPaths.emplace_back(ev.drop.data);
				#elif defined USING_SDL2
					dropPaths.emplace_back(ev.drop.file);
					SDL_free(ev.drop.file);	
				#endif
				} break;

			#ifdef USING_SDL2
			// SDL2 window events use two different enums and SDL3 only one.
			case SDL_WINDOWEVENT:
				if(ev.window.windowID == ID()){
					switch(ev.window.event){
			#endif
					case EV_WIN_CLOSE:
						win.destroyAll();
						Main::get().stop();
						return; // exit run loop
					case EV_WIN_SHOW:
						//printf("Window %d shown\n", ID());
						win.mVisible = true;
						win.callHandlersOnVisibility(win.mVisible);
						win.dimensions(win.dimensions());
						break;
					case EV_WIN_HIDE:
						//printf("Window %d hidden\n", ID());
						win.mVisible = false;
						win.callHandlersOnVisibility(win.mVisible);
						break;
					case EV_WIN_EXPOSE:
						//printf("Window %d exposed\n", ID());
						break;
					case EV_WIN_MOVE:
						//printf("Window %d moved to %d,%d\n", ID(), ev.window.data1, ev.window.data2);
						win.mDim.l = ev.window.data1;
						win.mDim.t = ev.window.data2;
						break;
					case EV_WIN_RESIZE:
					case EV_WIN_SIZE_CHANGE:
						//printf("Window %d resized to %dx%d\n", ID(), ev.window.data1, ev.window.data2);
						win.mDim.w = ev.window.data1;
						win.mDim.h = ev.window.data2;
						win.callHandlersOnResize(win.mDim.w, win.mDim.h);
						break;
			#ifdef USING_SDL2
					}
				}
				break;
			#endif

			// A "scancode" represents a physical location on a keyboard according to a QWERTY layout.

			case EV_KEY_DOWN:
			case EV_KEY_UP:
			if(ev.key.repeat == 0){ //&& !SDL_IsTextInputActive()
				bool keyDown = EV_KEY_DOWN == ev.type;
				auto& kb = win.mKeyboard;

				#ifdef USING_SDL3
					const auto& key = ev.key;
					int keyCode = SDL_GetKeyFromScancode(key.scancode, key.mod, false);
				#elif defined USING_SDL2
					// keysym gives us an SDL_Keysym struct with .scancode (physical code), .sym (virtual code) and .mod (modifier).
					const auto& key = ev.key.keysym;
					int keyCode = SDL_GetKeyFromScancode(key.scancode);
					//printf("Key %s: %s (sym:%#x scancode:%d keycode:%d)\n", keyDown?"down":"up", SDL_GetKeyName(key.sym), key.sym, key.scancode, SDL_GetKeyFromScancode(key.scancode));
				#endif

				kb.setKey(sdlToAlloKey(keyCode), keyDown);
				kb.alt  (key.mod & K_ALT);
				kb.ctrl (key.mod & K_CTL);
				kb.shift(key.mod & K_SHF);
				//printf("a:%d c:%d s:%d\n", kb.alt(), kb.ctrl(), kb.shift());

				keyDown ? win.callHandlersOnKeyDown() : win.callHandlersOnKeyUp();
			}	break;

			// In SDL2, this is the only way to get unicode (shifted) values.
			// Unfortunately, this event is only sent on key down.
			// These events are only active if SDL_StartTextInput was called.
			case EV_TEXT_INPUT:{
				const char * t = ev.text.text;
				//printf("Text input: %s\n", t);
			}	break;

			case EV_MOUSE_DOWN:
			case EV_MOUSE_UP:
			{	bool buttonDown = EV_MOUSE_DOWN == ev.type;
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

				if(!mUsingTouch){ // always 0,0 on touch devices!
					win.mMouse.position(ev.button.x, ev.button.y);
				}
				win.mMouse.button(btn, buttonDown);
				buttonDown ? win.callHandlersOnMouseDown() : win.callHandlersOnMouseUp();
			}	break;

			case EV_MOUSE_WHEEL: //printf("Mouse wheel: %d\n", ev.wheel.y);
				break;

			case EV_MOUSE_MOTION: //printf("Mouse motion: %d %d\n", ev.motion.x, ev.motion.y);
			{	if(!mUsingTouch) win.mMouse.position(ev.motion.x, ev.motion.y);
				if(win.mMouse.any())	win.callHandlersOnMouseDrag();
				else					win.callHandlersOnMouseMove();
			}	break;

			// For a single finger swipe, we get: SDL_MOUSEBUTTONDOWN, SDL_FINGERDOWN, SDL_FINGERMOTION, SDL_MOUSEBUTTONUP, SDL_FINGERUP. For multi-gesture, we get SDL_FINGERMOTION followed by SDL_MULTIGESTURE for each finger.

			case EV_FINGER_DOWN:
			{	mUsingTouch = true;
				++mFingersDown;
				if(1 == mFingersDown){ // prevent multiple triggers from different fingers
					win.mMouse.position(ev.tfinger.x*win.width(), ev.tfinger.y*win.height());
				}
			}	break;

			case EV_FINGER_UP:
			{	--mFingersDown;
				if(mFingersDown<0) mFingersDown=0; // just in case...
			}	break;

			case EV_FINGER_MOTION:
			{	if(1 == mFingersDown){ // prevent multiple triggers from different fingers
					win.mMouse.position(ev.tfinger.x*win.width(), ev.tfinger.y*win.height());
					win.callHandlersOnMouseDrag();
				}
			}	break;
			/*
			case SDL_MULTIGESTURE:
			{	switch(ev.mgesture.numFingers){
				case 2: win.mMouse.button(Mouse::RIGHT, true);
				case 3: win.mMouse.button(Mouse::MIDDLE, true);
				default:;
				}
				win.mMouse.position(ev.mgesture.x*win.width(), ev.mgesture.y*win.height());
				win.callHandlersOnMouseDrag();
			}	break;

			case SDL_FINGERUP:
			{	for(int i=0; i<AL_MOUSE_MAX_BUTTONS; ++i) win.mMouse.button(i, false);
			}	break;
			*/
			default:;
			}
		}

		if(dropPaths.size())
			win.callHandlersOnDrop(dropPaths);
	}

	void onFrame(){
		mWindow->updateFrameTime(); // Compute actual frame interval
		handleEvents();
		if(created()){
			mWindow->callHandlersOnFrame();
			const char * err = glGetErrorString();
			if(err[0]){
				AL_WARN_ONCE("Error after rendering frame in window (id=%d): %s", ID(), err);
			}
			SDL_GL_SwapWindow(mSDLWindow);
		}
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
	bool mUsingTouch = false;
	int mFingersDown = 0;

	#ifdef AL_EMSCRIPTEN
	struct MainHandler : public Main::Handler {
		MainHandler(WindowImpl& impl): outer(impl){}
		WindowImpl& outer;
		void onTick() override {
			outer.onFrame();
		}
	} mMainHandler{*this};
	#endif

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

	/*if(SDL_INIT_ERROR(SDL_InitSubSystem(SDL_INIT_TIMER))){
		printf("SDL ERROR: Failed to init SDL timer subsystem.\n");
		return false;
	}*/

	if(SDL_INIT_ERROR(SDL_InitSubSystem(SDL_INIT_VIDEO))){
		printf("SDL ERROR: Failed to init SDL video subsystem.\n");
		return false;
	}

	//SDL_EnableUNICODE(1);

	// SDL setup code: https://wiki.libsdl.org/SDL_GL_SetAttribute

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

	auto sdlWinFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

	#ifdef USING_SDL3
		auto sdlWin = SDL_CreateWindow(mTitle.c_str(), mDim.w,mDim.h, sdlWinFlags);
		if(sdlWin) SDL_SetWindowPosition(sdlWin, mDim.l,mDim.t);
	#elif defined USING_SDL2
		auto sdlWin = SDL_CreateWindow(mTitle.c_str(), mDim.l,mDim.t, mDim.w,mDim.h, sdlWinFlags);
	#endif

	if(!sdlWin){
		printf("SDL ERROR: Could not create window.\n");
		return false;
	}

	int top, left, bottom, right;
	if(SDL_GetWindowBordersSize(sdlWin, &top, &left, &bottom, &right) == 0) {
		mDim.l += left;
		mDim.t += top;
	}

	// Needed to get shifted keys in SDL2, although never got working...
	#ifdef USING_SDL2
		//SDL_StartTextInput();
	#endif

	mImpl->mSDLWindow = sdlWin;
	WindowImpl::windows()[mImpl->ID()] = mImpl;

	auto glContext = SDL_GL_CreateContext(sdlWin);
	if(!glContext){
		printf("SDL ERROR: Could not create GL context.\n");
	}
	mImpl->mGLContext = glContext;

	AL_GRAPHICS_INIT_CONTEXT;
	vsync(mVSync);

	callHandlersOnCreate();

	// SDL doesn't trigger a resize on window creation, so do it here
	callHandlersOnResize(mDim.w, mDim.h);

	// Set fullscreen according to mFullScreen member
	{	bool fs = fullScreen();
		mFullScreen = false;
		fullScreen(fs);
	}

	#ifdef AL_EMSCRIPTEN
		float period = asap() ? -1 : 1./fps();
		mImpl->scheduleDraw(period);
		// use requestAnimationFrame
		//emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);
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
	#ifdef AL_EMSCRIPTEN
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
	#ifdef AL_EMSCRIPTEN
		if(mFullScreen){
			EmscriptenFullscreenStrategy s;
			s.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT;
			s.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
			s.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
			s.canvasResizedCallback = nullptr;
			s.canvasResizedCallbackUserData = nullptr;
			emscripten_request_fullscreen_strategy("#canvas", true, &s);
		}
		else emscripten_exit_fullscreen();

	#else
		auto err = SDL_SetWindowFullscreen(
			mImpl->mSDLWindow,
			mFullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0
		);
		if(err<0){
			//printf("Error %s fullscreen (err = %d)\n", mFullScreen ? "entering" : "exiting", err);
		}
	#endif
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

