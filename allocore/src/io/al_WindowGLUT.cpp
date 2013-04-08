#include <stdio.h>		// snprintf
#include <stdlib.h>		// exit
#include <map>
#include "allocore/io/al_Window.hpp"
#include "allocore/system/al_Config.h"		// system defines
#include "allocore/system/al_MainLoop.hpp"	// start/stop loop, rendering
#include "allocore/system/al_Printing.hpp"	// warnings
#include "allocore/graphics/al_OpenGL.hpp"	// OpenGL headers

#ifdef AL_OSX
	#include <GLUT/glut.h>
#endif
#ifdef AL_LINUX
	#include <GL/glut.h>
#endif
#ifdef AL_WINDOWS
	#include <GL/glut.h>
#endif


namespace al{

//static void timerFunc(int value){
//	TimedFunction * f = (TimedFunction *)value;
//	f->onExecute();
//}

//void TimedFunction::operator()(float ms){
//	glutTimerFunc((unsigned int)ms, timerFunc, (int)this);
//}

class WindowImpl{
public:

	typedef std::map<int, WindowImpl *> WindowsMap;

	WindowImpl(Window * w)
	:	mWindow(w),
		mDimPrev(0), mDimCurr(0),
		mFPS(0), mTitle(""),
		mAvg(0.)
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
		mCursor = Window::POINTER;
		mVisible = true;
		mFullScreen = false;
		mCursorHide = false;
		mScheduled = false;
		mFrameTime = Main::get().realtime();
		mSPFActual = 0;
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



	// Setters
	void visible(bool v){ mVisible=v; }

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

			// ensure that current dims are set
			// GLUT will call the reshape CB upon entering game mode with fullscreen dims
			mDimCurr = dimensionsGLUT();

//			printf("l=%d, t=%d, w=%d, h=%d\n", mDimCurr.l, mDimCurr.t, mDimCurr.w, mDimCurr.h);

			windows().erase(mID);	// erase ID to stop draw scheduling...
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


	// incoming GLUT keys need to be remapped in certain cases...
	static unsigned int remapKey(unsigned int key, bool special){

		if(special){

			#define CS(k) case GLUT_KEY_##k: key = Keyboard::k; break;
			switch(key){
				CS(LEFT) CS(UP) CS(RIGHT) CS(DOWN)
				CS(PAGE_UP) CS(PAGE_DOWN)
				CS(HOME) CS(END) CS(INSERT)

				CS(F1) CS(F2) CS(F3) CS(F4)
				CS(F5) CS(F6) CS(F7) CS(F8)
				CS(F9) CS(F10)	CS(F11) CS(F12)
			}
			#undef CS
		}
		else{

			//printf("GLUT i: %3d %c\n", key, key);

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
				//Escape	=27glutPostRedisplay();

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

			//printf("GLUT o: %3d %c\n", key, key);
		}

		return key;
	}

	static void cbKeyboard(unsigned char key, int x, int y){
		Window * win = getWindow();
		if(win){
			key = remapKey(key, false);
			win->mKeyboard.setKey(key, true);
			setModifiers(win->mKeyboard);
			win->callHandlersOnKeyDown(win->mKeyboard);
		}
	}

	static void cbKeyboardUp(unsigned char key, int x, int y){
		Window * win = getWindow();
		if(win){
			key = remapKey(key, false);
			win->mKeyboard.setKey(key, false);
			setModifiers(win->mKeyboard);
			win->callHandlersOnKeyUp(win->mKeyboard);
		}
	}

	static void cbSpecial(int key, int x, int y){
		Window * win = getWindow();
		if(win){
			key = remapKey(key, true);
			win->mKeyboard.setKey(key, true);
			setModifiers(win->mKeyboard);
			win->callHandlersOnKeyDown(win->mKeyboard);
		}
	}

	static void cbSpecialUp(int key, int x, int y){
		Window * win = getWindow();
		if(win){
			key = remapKey(key, true);
			win->mKeyboard.setKey(key, false);
			setModifiers(win->mKeyboard);
			win->callHandlersOnKeyUp(win->mKeyboard);
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
				win->callHandlersOnMouseDown(m);
			}
			else if(GLUT_UP == state){
				m.position(ax, ay);	m.button(btn, false);
				win->callHandlersOnMouseUp(m);
			}
		}
	}

	static void cbMotion(int ax, int ay){
		//printf("GLUT: mouse drag x:%d y:%d\n", ax,ay);
		Window * win = getWindow();
		if(win){
			win->mMouse.position(ax,ay);
			win->callHandlersOnMouseDrag(win->mMouse);
		}
	}

	static void cbPassiveMotion(int ax, int ay){
		Window * win = getWindow();
		if(win){
			win->mMouse.position(ax,ay);
			win->callHandlersOnMouseMove(win->mMouse);
		}
	}


	// NOTE: This only gets called if glutMainLoop() has been called.
	static void cbReshape(int w, int h){
		//printf("GLUT reshape: w = %4d, h = %4d\n", w,h);
		Window * win = getWindow();
		if(win){
			win->makeCurrent();

			Window::Dim& dimPrev = win->mImpl->mDimPrev;
			Window::Dim& dimCurr = win->mImpl->mDimCurr;

			dimPrev = dimCurr;
			//dimCurr = win->dimensionsGLUT(); // BUG: not always new dimensions
			dimCurr.w = w;
			dimCurr.h = h;

			int dw = w - dimPrev.w;
			int dh = h - dimPrev.h;

			if(dw || dh){
				//printf("Window: onResize(%d, %d)\n", dw, dh);
				win->callHandlersOnResize(dw, dh);
				win->title(win->title());	// TODO: need this hack to get title
											// back after exiting full screen
			}
		}
	}

	static void cbVisibility(int v){
		Window * win = getWindow();
		if(win){
			win->mImpl->visible(v == GLUT_VISIBLE);
			win->callHandlersOnVisibility(v == GLUT_VISIBLE);
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
		}
	}

private:
	friend class Main;

	// schedule draws of a specific window
	static void scheduleDrawStatic(al_sec t, int winID) {
		Main& M = Main::get();
		WindowImpl *impl = getWindowImpl(winID);
		// If there is a valid implementation, then draw and schedule next draw...
		if(impl){
			Window * win = impl->mWindow;
			if(win){
				//al_sec prert = MainLoop::realtime();	// what time it really is now (before render)

				{	// compute actual frame interval
					al_sec timeNow = M.realtime();
					impl->mSPFActual = timeNow - impl->mFrameTime;
					impl->mFrameTime = timeNow;
				}
				
				// this calls the window's onFrame()
				win->implOnFrame();

				if(win->fps() > 0) {
					al_sec next;
					al_sec rt = M.realtime();	// what time it really is now (after render)
					if (win->asap()) {
						next = rt;
					} else {
						//al_sec dt = rt - prert;
						al_sec projected = t+1.0/win->fps();	// what time next render should be
						// calculate time of next frame; if it has already passed, do it immediately:
						next = projected;
						if (rt > projected) next = rt;	// next = MAX(rt,projected)
					}
					M.queue().send(next, scheduleDrawStatic, winID);
					
					// frame-rate calculation:
					al_sec per = 1./(next - t);
					impl->mAvg += 0.3 * (per - impl->mAvg);

				} else {
					impl->mScheduled = false;
				}
			} else {
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
	//Window::Dim mWinDim;
	Window::Dim mDimPrev, mDimCurr;
	double mFPS; // requested FPS
	al_sec mAvg;
	al_sec mFrameTime, mSPFActual;
	std::string mTitle;
	Window::Cursor mCursor;

	bool mInGameMode;
	bool mVisible;
	bool mFullScreen;
	bool mCursorHide;
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

const char * errorString(bool verbose){
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

void Window::implOnFrame(){
	const int winID = mImpl->id();
	const int current = glutGetWindow();
	if(winID != current) glutSetWindow(winID);
	callHandlersOnFrame();
	const char * err = errorString(true);
	if(err[0]){
		AL_WARN_ONCE("Error after rendering frame in window (id=%d): %s", winID, err);
	}
	glutSwapBuffers();
}


bool Window::create(
	const Dim& dim, const std::string& title, double fps, DisplayMode mode
)
{
	//printf("Window::create called (in al_WindowGLUT.cpp)\n");

	if(created()) return true;

	// switch mainloop to GLUT mode:
	Main::get().driver(Main::GLUT);

	mImpl->mDimPrev.set(0,0,0,0);
	mImpl->mDimCurr.set(dim.l,dim.t,0,0);
	mImpl->mTitle = title;
	mImpl->mFPS = fps;
	mDisplayMode = mode;

	glutInitWindowSize(dim.w, dim.h);
	glutInitWindowPosition(dim.l, dim.t);

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

	mImpl->mID = glutCreateWindow(mImpl->mTitle.c_str());

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
	WindowImpl::cbReshape(dim.w, dim.h);

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

bool Window::created() const { return mImpl->created(); }
Window::Cursor Window::cursor() const { return mImpl->mCursor; }
bool Window::cursorHide() const { return mImpl->mCursorHide; }

//Window::Dim Window::dimensions() const { return mImpl->dimensions(); }
Window::Dim Window::dimensions() const { return mImpl->mDimCurr; }
double Window::fps() const { return mImpl->mFPS; }
double Window::fpsAvg() const { return mImpl->mAvg; }
double Window::spfActual() const { return mImpl->mSPFActual; }
bool Window::fullScreen() const { return mImpl->mFullScreen; }
const std::string& Window::title() const { return mImpl->mTitle; }
bool Window::visible() const { return mImpl->mVisible; }

Window& Window::cursor(Cursor v){
	mImpl->mCursor = v;

	if(!mImpl->mCursorHide){
		switch(v){
			case CROSSHAIR:	if(makeCurrent()) glutSetCursor(GLUT_CURSOR_CROSSHAIR); break;
			case POINTER:	if(makeCurrent()) glutSetCursor(GLUT_CURSOR_INHERIT); break;
			default:;
		}
	}
	return *this;
}

Window& Window::cursorHide(bool v){
	mImpl->mCursorHide = v;
	if(makeCurrent()){
		if(v)	glutSetCursor(GLUT_CURSOR_NONE);
		else	cursor(mImpl->mCursor);
	}
	return *this;
}

Window& Window::dimensions(const Dim& v){
	//printf("Window::dimensions(const Dim&)\n");
	if(makeCurrent()){
		glutPositionWindow(v.l, v.t);
		mImpl->mDimPrev.l = mImpl->mDimCurr.l;
		mImpl->mDimPrev.t = mImpl->mDimCurr.t;
		mImpl->mDimCurr.l = v.l;
		mImpl->mDimCurr.t = v.t;
		glutReshapeWindow(v.w, v.h);
	}
	else{
		//mImpl->mDimPrev = mImpl->mDimCurr;
		mImpl->mDimPrev.set(0,0,0,0);
		mImpl->mDimCurr = v;
	}
	return *this;
}

Window& Window::fps(double v){
	if(created()){
		// Note: we can safely call scheduleDraw multiple times...
		if(v > 0) mImpl->scheduleDraw();
		//if(mImpl->mFPS <= 0 && v > 0) mImpl->scheduleDraw();
	}
	mImpl->mFPS = v;
	return *this;
}

Window& Window::fullScreen(bool v){

	// exit full screen
	if(mImpl->mFullScreen && !v){
		#ifdef AL_LINUX
			callHandlersOnDestroy();
			// Must manually set dims since exiting game mode does NOT 
			// automatically call GLUT reshape callback.
			mImpl->mDimPrev = mImpl->dimensionsGLUT();
			mImpl->gameMode(false);
			mImpl->mDimCurr = mImpl->dimensionsGLUT();
			callHandlersOnCreate();
			callHandlersOnResize(
				mImpl->mDimCurr.w - mImpl->mDimPrev.w,
				mImpl->mDimCurr.h - mImpl->mDimPrev.h
			);
			//callHandlersOnResize(mImpl->mWinDim.w, mImpl->mWinDim.h);
			hide(); show(); // need to force focus to get key callbacks to work
		#else
			dimensions(mImpl->mDimPrev);	// calls glutReshapeWindow which leaves full screen
		#endif
	}

	// enter full screen
	else if(!mImpl->mFullScreen && v){

		//mImpl->mWinDim = dimensionsGLUT();

		#ifdef AL_LINUX
			callHandlersOnDestroy();
			// TODO: current dims now set in gameMode().
			// GLUT automatically calls reshape CB upon entering game mode...
//			mImpl->mDimPrev = dimensionsGLUT();		// dims of windowed window
			mImpl->gameMode(true);
//			mImpl->mDimCurr = dimensionsGLUT();		// dims of fullscreen window

//			printf("l=%d, t=%d, w=%d, h=%d\n", mImpl->mDimPrev.l, mImpl->mDimPrev.t, mImpl->mDimPrev.w, mImpl->mDimPrev.h);
//			printf("l=%d, t=%d, w=%d, h=%d\n", mImpl->mDimCurr.l, mImpl->mDimCurr.t, mImpl->mDimCurr.w, mImpl->mDimCurr.h);
			callHandlersOnCreate();
			cursorHide(cursorHide());
		#else
			glutSetWindow(mImpl->mID);
			mImpl->mDimCurr = mImpl->dimensionsGLUT();
			glutFullScreen(); // calls glutReshapeWindow
		#endif
	}

	mImpl->mFullScreen = v;
	return *this;
}

Window& Window::hide(){ if(makeCurrent()) glutHideWindow(); return *this; }

Window& Window::iconify(){ if(makeCurrent()) glutIconifyWindow(); return *this; }

bool Window::makeCurrent() const {
	if(created()){
		glutSetWindow(mImpl->id());
		return true;
	}
	return false;
}

Window& Window::show(){ if(makeCurrent()) glutShowWindow(); return *this; }

Window& Window::title(const std::string& v){
	mImpl->mTitle = v;
	if(makeCurrent()){
		glutSetWindowTitle(mImpl->mTitle.c_str());
		//printf("Window::title(%s)\n", mImpl->mTitle.c_str());
	}
	return *this;
}

Window& Window::vsync(bool v){
	if(makeCurrent()){
		mVSync = v;
		#if defined AL_OSX
			GLint VBL = v ? 1 : 0;
			CGLContextObj ctx = CGLGetCurrentContext();
			CGLSetParameter(ctx, kCGLCPSwapInterval, &VBL);
		#elif defined AL_LINUX
		#elif defined AL_WINDOWS
		#endif
	}
	return *this;
}

} // al::

