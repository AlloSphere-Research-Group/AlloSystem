#include <stdio.h>		// snprintf
#include <stdlib.h>		// exit
#include <map>
#include "allocore/io/al_Window.hpp"
#include "allocore/system/al_Config.h"		// system defines
#include "allocore/system/al_MainLoop.hpp"	// start/stop loop, rendering
#include "allocore/system/al_Printing.hpp"	// warnings
#include "allocore/graphics/al_OpenGL.hpp"	// OpenGL headers

#if defined AL_OSX
	#include <GLUT/glut.h>

#elif defined AL_LINUX
	#include <GL/glew.h>
	#include <GL/glut.h>

#elif defined AL_WINDOWS
	#include <GL/wglew.h> // wglSwapInterval
	#include <GL/glut.h>
#endif


namespace al{

class WindowImpl{
public:

	typedef std::map<int, WindowImpl *> WindowsMap;

	WindowImpl(Window * w)
	:	mWindow(w), mDimPrev(0)
	{
		resetState();
		// ensure that GLUT and mainloop exist:
		Main::get();
	}

	~WindowImpl(){
		destroy();
	}

	void resetState(){
		mID = -1;
		mIDGameMode = -1;
		mInGameMode = false;
		mScheduled = false;
	}


	// Getters
	bool created() const { return mID > 0; }

	int id() const { return mInGameMode ? mIDGameMode : mID; }

	Window::Dim dimensionsGLUT() const {
		Window::Dim d(0,0,0,0);
		if(created()){
			glutSetWindow(id());
			d.l = glutGet(GLUT_WINDOW_X);
			d.t = glutGet(GLUT_WINDOW_Y);
			d.w = glutGet(GLUT_WINDOW_WIDTH);
			d.h = glutGet(GLUT_WINDOW_HEIGHT);
		}
		return d;
	}

	void makeMainWindow(){
		glutSetWindow(id());
	}

	void destroy(){ //printf("destroy: %p\n", this);
		if(created()){
			glutDestroyWindow(mID);
			windows().erase(id());
			resetState();
		}
	}

	void gameMode(bool v){

		/* GLUT reshape behavior:
			When entering game mode, the GLUT reshape callback gets called
			with the dimensions of the fullscreen window.
			When exiting game mode, the GLUT reshape callback is NOT called.
		*/

        mScheduled = false;
		// Go into game mode
		if(v){

	//		"width=1024 height=768 bpp=24 hertz=60"

	//	=       Equal.
	//	!=      Not equal.
	//	<       Less  than  and  preferring larger difference (the
	//			least is best).
	//	>       Greater than  and  preferring  larger  differences
	//			(the most is best).
	//	<=      Less  than  or equal and preferring larger differ-
	//			ence (the least is best).
	//	>=      Greater than or equal and preferring more  instead
	//			of less.  This comparator is useful for allocating
	//			resources like color precsion or depth buffer pre-
	//			cision  where  the  maximum  precison is generally
	//			preferred.  Contrast with the tilde (~) comprator.
	//	~       Greater  than or equal but preferring less instead
	//			of more.  This compartor is useful for  allocating
	//			resources  such as stencil bits or auxillary color
	//			buffers where you would rather not over  allocate.

	//		bpp     Bits per pixel for the frame buffer.
	//		height  Height of the screen in pixels.
	//		hertz   Video refresh rate of the screen in hertz.
	//		num     Number  of  the  window  system depenedent display
	//				mode configuration.
	//		width   Width of the screen in pixels.

	//		compact mode [ width "x" height ][ ":" bitsPerPixel ][ "@" videoRate ]

			// use screen resolution for fullscreen dimensions
			int sw = glutGet(GLUT_SCREEN_WIDTH);
			int sh = glutGet(GLUT_SCREEN_HEIGHT);

			// if invalid screen resolution, use sensible defaults
			if(!sw || !sh){
				sw = 1024;
				sh = 768;
			}

			// set full screen settings
			{
				char buf[32];
				snprintf(buf, sizeof(buf), "%dx%d:24", sw, sh);
				glutGameModeString(buf);
				//int refresh = glutGameModeGet(GLUT_GAME_MODE_REFRESH_RATE);
				//printf("%d\n", refresh);
			}

			windows().erase(mID);	// erase ID to stop draw scheduling...

			// GLUT will call the reshape CB upon entering game mode with fullscreen dims
			mIDGameMode = glutEnterGameMode();
			mInGameMode = true;
			windows()[mIDGameMode] = this;

			glutSetWindow(mIDGameMode);
			registerCBs();	// need to register callbacks since new window
			scheduleDraw();
		}

		// Exit game mode
		// All previously created windows are restored...
		else{
			windows().erase(mIDGameMode); // erase ID to stop draw scheduling...
			//glutSetWindow(mIDGameMode); // freeglut requires this before leaving game mode
			glutLeaveGameMode();
			mInGameMode = false;
			windows()[mID] = this;

			glutSetWindow(mID);
			//registerCBs();
			scheduleDraw();
		}
	}


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

	static void setModifiers(Keyboard& k){
		int mod = glutGetModifiers();
		k.alt  (mod & GLUT_ACTIVE_ALT);
		k.ctrl (mod & GLUT_ACTIVE_CTRL);
		k.shift(mod & GLUT_ACTIVE_SHIFT);
		//printf("a:%d c:%d s:%d\n", k.alt(), k.ctrl(), k.shift());
	}

	// An invalid key to indicate an error
	static const int INVALID_KEY = -1;

	// incoming GLUT keys need to be remapped in certain cases...
	static int remapKey(int key, bool special){

		//printf("GLUT i: %3d %c\n", key, key);

		if(special){

			#define CS(k) case GLUT_KEY_##k: key = Keyboard::k; break;
			switch(key){
				CS(LEFT) CS(UP) CS(RIGHT) CS(DOWN)
				CS(PAGE_UP) CS(PAGE_DOWN)
				CS(HOME) CS(END) CS(INSERT)

				CS(F1) CS(F2) CS(F3) CS(F4)
				CS(F5) CS(F6) CS(F7) CS(F8)
				CS(F9) CS(F10)	CS(F11) CS(F12)

				// Sometimes GLUT will pass an unspecified key code to the 
				// special function. The callee should consider this an error...
				default: key = INVALID_KEY;
			}
			#undef CS
		}
		else{

			#define MAP(i,o) case i: key=o; break

//			bool shft = glutGetModifiers() & GLUT_ACTIVE_SHIFT;
//			if(shft && (key>32 && key<127)){
//				const char * QWERTYunshifted =
//					" 1\'3457\'908=,-./0123456789;;,=./"
//					"2abcdefghijklmnopqrstuvwxyz[\\]6-"
//					"`abcdefghijklmnopqrstuvwxyz[\\]`"
//				;
//				key = QWERTYunshifted[key-32];
//			}

			// Reassign keycodes when CTRL is down
			//#ifdef AL_OSX

			bool ctrl = glutGetModifiers() & GLUT_ACTIVE_CTRL;

			if(ctrl){
				// All alphabetical keys get dropped to lower ASCII range.
				// Some will conflict with standard non-printable characters.
				// There is no way to detect this, since the control modified
				// keycode gets sent to the GLUT callback. We will assume that
				// ctrl-key events are the printable character keys.

				//Enter		=3
				//BackSpace	=8
				//Tab		=9
				//Return	=13
				//Escape	=27

				if(key <= 26){ key += 96; }

				// only some non-alphabetic keys are wrong...
				else{

					switch(key){
						MAP(27, '[');
						MAP(28, '\\');
						MAP(29, ']');
						MAP(31, '-');
					};

				}
			}

			//#endif
			#undef MAP

			#ifdef AL_OSX

			// swap backspace and delete
			if(8 == key){ key = 127; }
			else if(127 == key){ key = 8; }

			#endif
		}

		//printf("GLUT o: %3d %c\n", key, key);

		return key;
	}

	static void cbKeyboard(unsigned char key, int x, int y){
		//printf("GLUT: key down %d (%c)\n", key,key);
		Window * win = getWindow();
		if(win){
			key = remapKey(key, false);
			win->mKeyboard.setKey(key, true);
			setModifiers(win->mKeyboard);
			win->callHandlersOnKeyDown();
		}
	}

	static void cbKeyboardUp(unsigned char key, int x, int y){
		//printf("GLUT: key up %d (%c)\n", key,key);
		Window * win = getWindow();
		if(win){
			key = remapKey(key, false);
			win->mKeyboard.setKey(key, false);
			setModifiers(win->mKeyboard);
			win->callHandlersOnKeyUp();
		}
	}

	static void cbSpecial(int key, int x, int y){
		Window * win = getWindow();
		if(win){
			key = remapKey(key, true);
			if(INVALID_KEY != key){
				win->mKeyboard.setKey(key, true);
				setModifiers(win->mKeyboard);
				win->callHandlersOnKeyDown();
			}
		}
	}

	static void cbSpecialUp(int key, int x, int y){
		Window * win = getWindow();
		if(win){
			key = remapKey(key, true);
			if(INVALID_KEY != key){
				win->mKeyboard.setKey(key, false);
				setModifiers(win->mKeyboard);
				win->callHandlersOnKeyUp();
			}
		}
	}

	static void cbMouse(int btn, int state, int ax, int ay){
		//printf("GLUT: mouse click x:%d y:%d bt:#%d,%d\n", ax,ay, btn, state==GLUT_DOWN);
		Window * win = getWindow();
		if(win){
			switch(btn){
				case GLUT_LEFT_BUTTON:		btn = Mouse::LEFT; break;
				case GLUT_MIDDLE_BUTTON:	btn = Mouse::MIDDLE; break;
				case GLUT_RIGHT_BUTTON:		btn = Mouse::RIGHT; break;
				default:					btn = Mouse::EXTRA;		// unrecognized button
			}

			// update modifiers here for shift-mouse etc.
			WindowImpl::setModifiers(win->mKeyboard);

			Mouse& m = win->mMouse;
			if(GLUT_DOWN == state){
				m.position(ax, ay); m.button(btn, true);
				win->callHandlersOnMouseDown();
			}
			else if(GLUT_UP == state){
				m.position(ax, ay);	m.button(btn, false);
				win->callHandlersOnMouseUp();
			}
		}
	}

	static void cbMotion(int ax, int ay){
		//printf("GLUT: mouse drag x:%d y:%d\n", ax,ay);
		Window * win = getWindow();
		if(win){
			win->mMouse.position(ax,ay);
			win->callHandlersOnMouseDrag();
		}
	}

	static void cbPassiveMotion(int ax, int ay){
		Window * win = getWindow();
		if(win){
			win->mMouse.position(ax,ay);
			win->callHandlersOnMouseMove();
		}
	}


	// NOTE: This only gets called if glutMainLoop() has been called.
	static void cbReshape(int w, int h){
		//printf("GLUT reshape: w = %4d, h = %4d\n", w,h);
		Window * win = getWindow();
		if(win){
			Window::Dim& dimCurr = win->mDim;
			//printf("mDim: w = %4d, h = %4d\n", dimCurr.w, dimCurr.h);
			if(dimCurr.w != w || dimCurr.h != h){
				dimCurr.w = w;
				dimCurr.h = h;
				//printf("trigger resize with %d %d\n", w,h);
				win->callHandlersOnResize(w, h);

				// needed to get title back after exiting full screen
				win->title(win->title());
			}
		}
	}

	static void cbVisibility(int v){
		Window * win = getWindow();
		if(win){
			win->mVisible = (v == GLUT_VISIBLE);
			win->callHandlersOnVisibility(win->mVisible);
		}
	}

	// This is triggered upon a call to glutPostRedisplay or implicitly as the
	// result of window damage reported by the window system.
	static void cbDisplay(){
		//printf("GLUT display for window %d\n", glutGetWindow());
		// this is empty because we are using a periodic timer for drawing
	}

	static void registerCBs(){
		glutKeyboardFunc(cbKeyboard);
		glutKeyboardUpFunc(cbKeyboardUp);
		glutMouseFunc(cbMouse);
		glutMotionFunc(cbMotion);
		glutPassiveMotionFunc(cbPassiveMotion);
		glutSpecialFunc(cbSpecial);
		glutSpecialUpFunc(cbSpecialUp);
		glutReshapeFunc(cbReshape);
		glutVisibilityFunc(cbVisibility);
		glutDisplayFunc(cbDisplay);
	}


	void scheduleDraw(){
		if (!mScheduled) {
			mScheduled = true;
			//printf("window id: %d\n", id());
			Main::get().queue().send(0, scheduleDrawStatic, id());
			//scheduleDrawStaticGLUT(id());
		}
	}

private:
	friend class Main;

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
			CS(GL_STACK_OVERFLOW, "This command would cause a stack overflow.")
			CS(GL_STACK_UNDERFLOW, "This command would cause a stack underflow.")
		#ifdef GL_TABLE_TOO_LARGE
			CS(GL_TABLE_TOO_LARGE, "The specified table exceeds the implementation's maximum supported table size.")
		#endif
			default: return "Unknown error code.";
		}
		#undef CS
	}

	void onFrame(){
		const int winID = id();
		const int current = glutGetWindow();
		if(winID != current) glutSetWindow(winID);
		mWindow->callHandlersOnFrame();
		const char * err = errorString(true);
		if(err[0]){
			AL_WARN_ONCE("Error after rendering frame in window (id=%d): %s", winID, err);
		}
		glutSwapBuffers();
	}


	// schedule draws of a specific window
	static void scheduleDrawStatic(al_sec t, int winID) {
		/* Note: This function used to use the Main scheduler queue, however,
		the Main scheduler uses a fixed-interval polling mechanism with too
		course of a timing granularity to obtain a precise enough frame rate
		for smooth animation. Instead, we call glutTimerFunc directly using an
		estimated delta time. */
		scheduleDrawStaticGLUT(winID);
	}

	static void scheduleDrawStaticGLUT(int winID){

		WindowImpl *impl = getWindowImpl(winID);

		// If there is a valid implementation, then draw and schedule next draw...
		if(impl){
			Window * win = impl->mWindow;
			if(win){

				// Compute actual frame interval
				win->updateFrameTime();

				// This calls the window's onFrame()
				impl->onFrame();

				if(win->fps() > 0){

					unsigned waitMsec = 0;

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
							double wait = timeNext - timePost;
							waitMsec = unsigned(wait * 1000. + 0.5);
							//printf("num=%lu, wait=%5.3f frames, %2u ms, dt=%5.3f, fps=%5.2f (avg=%5.2f)\n", (unsigned long)(timePost*FPS), wait*FPS, waitMsec, win->mDeltaTime, win->fpsActual(), win->fpsAvg());
						}
						else{
							//printf("dropped frame!\n");
						}
					}
					#ifdef AL_WINDOWS
						// Passing 0 ms wait to glutTimerFunc may cause lockup on Windows
						if(0==waitMsec) waitMsec=1;
					#endif
					glutTimerFunc(waitMsec, scheduleDrawStaticGLUT, winID);
				}
				else {
					impl->mScheduled = false;
				}
			}
			else {
				//printf("no window\n");
				impl->mScheduled = false;
			}
		}
	}

	// Get ID -> created GLUT window map
	static WindowsMap& windows(){
		static WindowsMap* v = new WindowsMap;
		return *v;
	}

	Window * mWindow;
	int mID;
	int mIDGameMode;
	Window::Dim mDimPrev;

	bool mInGameMode;
	bool mScheduled;

	friend class Window;
};



//******************************************************************************

void Window::implCtor(){
	mImpl = new WindowImpl(this);
}

void Window::implDtor(){
	//printf("implDtor\n");
	delete mImpl;
}

void Window::implDestroy(){
	mImpl->destroy();
}

bool Window::implCreate(){
	//printf("Window::create called (in al_WindowGLUT.cpp)\n");

	// switch mainloop to GLUT mode:
	Main::get().driver(Main::GLUT);

	mImpl->mDimPrev.set(0,0,0,0);

	// We must zero extent of Dim member so resize callback detects a change
	int w = mDim.w;
	int h = mDim.h;
	mDim.w = 0;
	mDim.h = 0;

	glutInitWindowSize(w, h);
	glutInitWindowPosition(mDim.l, mDim.t);

    int bits =
        (enabled(SINGLE_BUF )	? GLUT_SINGLE		:0) |
        (enabled(DOUBLE_BUF )	? GLUT_DOUBLE		:0) |
        (enabled(ACCUM_BUF  )	? GLUT_ACCUM		:0) |
        (enabled(ALPHA_BUF  )	? GLUT_ALPHA		:0) |
        (enabled(DEPTH_BUF  )	? GLUT_DEPTH		:0) |
        (enabled(STENCIL_BUF)	? GLUT_STENCIL		:0) |
        (enabled(STEREO_BUF )	? GLUT_STEREO		:0) |
		(enabled(MULTISAMPLE)	? GLUT_MULTISAMPLE	:0);

	glutInitDisplayMode(GLUT_RGBA | bits);

//	int stat = glutGet(GLUT_DISPLAY_MODE_POSSIBLE);
//	printf("%d\n", stat);

	mImpl->mID = glutCreateWindow(mTitle.c_str());

	// check that the window was created...
	if(!(mImpl->mID > 0)){
		return false;
	}
	//printf("GLUT created window %d\n",mImpl->mID);

	glutSetWindow(mImpl->mID);
	glutIgnoreKeyRepeat(1);
	WindowImpl::registerCBs();
	WindowImpl::windows()[mImpl->mID] = mImpl;

	AL_GRAPHICS_INIT_CONTEXT;
	vsync(mVSync);

	callHandlersOnCreate();

	// We need to manually call this because GLUT will not call its reshape
	// callback until the main loop is started. By forcing it here, we ensure
	// that the requested window width and height get properly stored.
	WindowImpl::cbReshape(w, h);

	// Set fullscreen according to mFullScreen member
	{	bool fs = fullScreen();
		mFullScreen = false;
		fullScreen(fs);
	}

	mImpl->scheduleDraw();

	return true;
}

void Window::destroyAll(){ //printf("Window::destroyAll\n");
	WindowImpl::WindowsMap::iterator it = WindowImpl::windows().begin();
	while(it != WindowImpl::windows().end()){
		if(it->second && it->second->mWindow){
			(it++)->second->mWindow->destroy();
		}
		else{
			++it;
		}
	}
}

bool Window::created() const {
	return mImpl->created();
}

void Window::implSetDimensions(){
	mImpl->makeMainWindow();
	glutPositionWindow(mDim.l, mDim.t);

	// Set mDim extent to actual window extent so reshape callback triggers
	// handlers.
	int w = mDim.w;
	int h = mDim.h;
	mDim.w = glutGet(GLUT_WINDOW_WIDTH);
	mDim.h = glutGet(GLUT_WINDOW_HEIGHT);
	glutReshapeWindow(w, h);
}

void Window::implSetCursor(){
	if(!mCursorHide){
		mImpl->makeMainWindow();
		switch(mCursor){
			case CROSSHAIR:	glutSetCursor(GLUT_CURSOR_CROSSHAIR); break;
			case POINTER:	glutSetCursor(GLUT_CURSOR_INHERIT); break;
			default:;
		}
	}
}

void Window::implSetCursorHide(){
	mImpl->makeMainWindow();
	if(mCursorHide)	glutSetCursor(GLUT_CURSOR_NONE);
	else			cursor(mCursor);
}

void Window::implSetFPS(){
	// Nothing to do here except ensure the scheduler has started.
	// Note: we can safely call scheduleDraw multiple times...
	mImpl->scheduleDraw();
}

void Window::implSetFullScreen(){

	// Note that on Linux, we must use GLUT's "game mode" to get a borderless
	// fullscreen window.

	// enter full screen
	if(mFullScreen){
		#ifdef AL_LINUX
			callHandlersOnDestroy();
			// GLUT automatically calls reshape CB upon entering game mode...
			mImpl->gameMode(true);
			callHandlersOnCreate();
			cursorHide(cursorHide());
		#else
			glutSetWindow(mImpl->mID);
			mImpl->mDimPrev = mImpl->dimensionsGLUT();
			glutFullScreen(); // calls glutReshapeWindow
		#endif
	}

	// exit full screen
	else {
		#ifdef AL_LINUX
			callHandlersOnDestroy();
			mImpl->gameMode(false);
			mDim = mImpl->dimensionsGLUT();
			callHandlersOnCreate();
			callHandlersOnResize(mDim.w, mDim.h);
			hide(); show(); // need to force focus to get key callbacks to work
		#else
			// Calls glutReshapeWindow which exits from a glutFullScreen call.
			// This also calls our reshape callback which sets the mDim member.
			dimensions(mImpl->mDimPrev);
		#endif
	}
}

void Window::implSetTitle(){
	mImpl->makeMainWindow();
	glutSetWindowTitle(mTitle.c_str());
	//printf("Window::title(%s)\n", mTitle.c_str());
}

// See: https://www.opengl.org/wiki/Swap_Interval
void Window::implSetVSync(){
	mImpl->makeMainWindow();
	#if defined AL_OSX
		GLint VBL = GLint(mVSync);
		CGLContextObj ctx = CGLGetCurrentContext();
		CGLSetParameter(ctx, kCGLCPSwapInterval, &VBL);
	#elif defined AL_LINUX
	#elif defined AL_WINDOWS
		wglSwapIntervalEXT(int(mVSync));
	#endif
}

Window& Window::hide(){
	mImpl->makeMainWindow();
	glutHideWindow();
	return *this;
}

Window& Window::iconify(){
	mImpl->makeMainWindow();
	glutIconifyWindow();
	return *this;
}

Window& Window::show(){
	mImpl->makeMainWindow();
	glutShowWindow();
	return *this;
}

} // al::

