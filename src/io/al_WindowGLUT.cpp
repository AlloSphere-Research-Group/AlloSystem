#include <stdio.h>		// snprintf
#include <stdlib.h>		// exit
#include <map>

#include "io/al_WindowGL.hpp"

#include "system/al_Config.h"		// system defines
#include "system/al_MainLoop.hpp"	// start/stop loop, rendering
#include "graphics/al_Config.h"		// OpenGL headers

#ifdef AL_OSX
	#include <GLUT/glut.h>
#endif
#ifdef AL_LINUX
	#include <GL/glut.h>
#endif
#ifdef AL_WIN32
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

	WindowImpl(WindowGL * w)
	:	mWindow(w), mID(-1), mIDGameMode(-1),
		mWinDim(0), mFPS(0), mTitle(""),
		mMode(DisplayMode::DefaultBuf),
		mCursor(Cursor::Pointer),
		mAvg(0.),
		mInGameMode(false), mVisible(true), mFullScreen(false), mCursorHide(false),
		mScheduled(false)
	{
		// ensure that GLUT and mainloop exist:
		MainLoop::get();
		//
	}

	~WindowImpl(){
		destroy();
	}

	bool created(){ return mID >= 0; }

	void destroy(){ //printf("destroy\n");
		if(created()){
			glutDestroyWindow(mID);
			windows().erase(id());
			mID = -1;
			mIDGameMode = -1;
			mInGameMode = false;
			mScheduled = false;
		}
	}

	void gameMode(bool v){

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

			// get current screen resolution
			int sw = glutGet(GLUT_SCREEN_WIDTH);
			int sh = glutGet(GLUT_SCREEN_HEIGHT);

			// use current resolution and refresh rate
			if(sw && sh){
				char buf[32];
				snprintf(buf, sizeof(buf), "%dx%d:24", sw, sh);
				glutGameModeString(buf);

				//int refresh = glutGameModeGet(GLUT_GAME_MODE_REFRESH_RATE);
				//printf("%d\n", refresh);
			}

			// otherwise, use sensible defaults
			else{
				glutGameModeString("1024x768:24");
			}

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

	int id(){ return mInGameMode ? mIDGameMode : mID; }

	void scheduleDraw(){
		if (!mScheduled) {
			mScheduled = true;
			//printf("window id: %d\n", id());
			MainLoop::queue().send(0, scheduleDrawStatic, id());
		}
	}

	void visible(bool v){ mVisible=v; }

	// Returns the implementation of the currently selected window
	static WindowImpl * getWindowImpl(){ return getWindowImpl(glutGetWindow()); }


	static WindowImpl * getWindowImpl(int id){
		if(windows().count(id) > 0){
			return windows()[id];
		}
		return 0;
	}

	// Returns the currently selected window or 0 if invalid
	static WindowGL * getWindow(){
		WindowImpl * w = getWindowImpl();
		return w ? w->mWindow : 0;
	}

	static void setModifiers(Keyboard& k){
		int mod = glutGetModifiers();
		k.alt  (mod & GLUT_ACTIVE_ALT);
		k.ctrl (mod & GLUT_ACTIVE_CTRL);
		k.shift(mod & GLUT_ACTIVE_SHIFT);
		//printf("a:%d c:%d s:%d\n", g->keyboard.alt(), g->keyboard.ctrl(), g->keyboard.shift());
	}

	static void cbDisplay(){
		// this is empty because we are using a periodic timer for drawing
	}

	// incoming GLUT keys need to be remapped in certain cases...
	static unsigned int remapKey(unsigned int key, bool special){

		if(special){

			#define CS(glut, io) case GLUT_KEY_##glut: key = Key::io; break;
			switch(key){
				CS(LEFT, Left) CS(UP, Up) CS(RIGHT, Right) CS(DOWN, Down)
				CS(PAGE_UP, PageUp) CS(PAGE_DOWN, PageDown)
				CS(HOME, Home) CS(END, End) CS(INSERT, Insert)

				CS(F1, F1) CS(F2, F2) CS(F3, F3) CS(F4, F4)
				CS(F5, F5) CS(F6, F6) CS(F7, F7) CS(F8, F8)
				CS(F9, F9) CS(F10, F10)	CS(F11, F11) CS(F12, F12)
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

			//printf("GLUT o: %3d %c\n", key, key);
		}

		return key;
	}

	static void cbKeyboard(unsigned char key, int x, int y){
//printf("GLUT key down:\n");
		WindowGL * win = getWindow();
		if(win){
			key = remapKey(key, false);
			win->mKeyboard.setKey(key, true);
			setModifiers(win->mKeyboard);
			win->doKeyDown(win->mKeyboard);
		}
	}

	static void cbKeyboardUp(unsigned char key, int x, int y){
		WindowGL * win = getWindow();
		if(win){
			key = remapKey(key, false);
			win->mKeyboard.setKey(key, false);
			setModifiers(win->mKeyboard);
			win->doKeyUp(win->mKeyboard);
		}
	}

	static void cbSpecial(int key, int x, int y){
		WindowGL * win = getWindow();
		if(win){
			key = remapKey(key, true);
			win->mKeyboard.setKey(key, true);
			setModifiers(win->mKeyboard);
			win->doKeyDown(win->mKeyboard);
		}
	}

	static void cbSpecialUp(int key, int x, int y){
		WindowGL * win = getWindow();
		if(win){
			key = remapKey(key, true);
			win->mKeyboard.setKey(key, false);
			setModifiers(win->mKeyboard);
			win->doKeyUp(win->mKeyboard);
		}
	}


	static void cbMouse(int btn, int state, int ax, int ay){
		//printf("GLUT: mouse event x:%d y:%d bt:#%d,%d\n", ax,ay, btn, state==GLUT_DOWN);
		WindowGL * win = getWindow();
		if(win){
			switch(btn){
				case GLUT_LEFT_BUTTON:		btn = Mouse::Left; break;
				case GLUT_MIDDLE_BUTTON:	btn = Mouse::Middle; break;
				case GLUT_RIGHT_BUTTON:		btn = Mouse::Right; break;
				default:					btn = Mouse::Extra;		// unrecognized button
			}

			Mouse& m = win->mMouse;
			if(GLUT_DOWN == state){
				m.position(ax, ay); m.button(btn, true);
				win->doMouseDown(m);
			}
			else if(GLUT_UP == state){
				m.position(ax, ay);	m.button(btn, false);
				win->doMouseUp(m);
			}
		}
	}

	static void cbMotion(int ax, int ay){
		WindowGL * win = getWindow();
		if(win){
			win->mMouse.position(ax,ay);
			win->doMouseDrag(win->mMouse);
		}
	}

	static void cbPassiveMotion(int ax, int ay){
		WindowGL * win = getWindow();
		if(win){
			win->mMouse.position(ax,ay);
			win->doMouseMove(win->mMouse);
		}
	}


	static void cbReshape(int w, int h){
		WindowGL * win = getWindow();
		if(win){
			win->makeActive();
			win->onResize(w, h);
			win->title(win->title());	// TODO: need this hack to get title back
										// after exiting full screen
		}
	}

	static void cbVisibility(int v){
		WindowGL * win = getWindow();
		if(win){
			win->mImpl->visible(v == GLUT_VISIBLE);
			win->onVisibility(v == GLUT_VISIBLE);

		}
	}

	static void registerCBs(){
		glutDisplayFunc(cbDisplay);
		glutKeyboardFunc(cbKeyboard);
		glutKeyboardUpFunc(cbKeyboardUp);
		glutMouseFunc(cbMouse);
		glutMotionFunc(cbMotion);
		glutPassiveMotionFunc(cbPassiveMotion);
		glutReshapeFunc(cbReshape);
		glutSpecialFunc(cbSpecial);
		glutSpecialUpFunc(cbSpecialUp);
		glutVisibilityFunc(cbVisibility);
	}

private:
	friend class MainLoop;

	// schedule draws of a specific window
	static void scheduleDrawStatic(al_sec t, int winID) {
		WindowImpl *impl = getWindowImpl(winID);

		// If there is a valid implementation, then draw and schedule next draw...
		if(impl){
			WindowGL * win = impl->mWindow;
			if(win){
				win->doFrame();
				if(win->fps() > 0) {
					al_sec projected = t+1.0/win->fps();	// what time next render should be
					al_sec rt = MainLoop::realtime();	// what time it really is now (after render)
					al_sec next = projected;
					if (rt > projected) next = rt;	// next = MAX(rt,projected)

					MainLoop::queue().send(next, scheduleDrawStatic, winID);
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
//		else {
//			printf("no window impl\n");
//		}
	}

	// Map of windows constructed on first use to avoid static intialization
	// order problems.
	static WindowsMap& windows(){
		static WindowsMap* windowsmap = new WindowsMap;
		return *windowsmap;
	}

	WindowGL * mWindow;
	int mID;
	int mIDGameMode;
	WindowGL::Dim mWinDim;
	double mFPS;
	std::string mTitle;
	DisplayMode::t mMode;
	Cursor::t mCursor;
	al_sec mAvg;

	bool mInGameMode;
	bool mVisible;
	bool mFullScreen;
	bool mCursorHide;
	bool mScheduled;

	friend class WindowGL;
};



WindowGL::WindowGL(): mImpl(new WindowImpl(this)){}

WindowGL::~WindowGL(){
	destroy();
	delete mImpl;
}


void WindowGL::create(
	const Dim& dim, const std::string title, double fps, DisplayMode::t mode
)
{
	if(mImpl->created()) return;

	mImpl->mWinDim = dim;
	mImpl->mTitle = title;
	mImpl->mFPS = fps;
	mImpl->mMode = mode;

	glutInitWindowSize(mImpl->mWinDim.w, mImpl->mWinDim.h);
	glutInitWindowPosition(mImpl->mWinDim.l, mImpl->mWinDim.t);

	using namespace DisplayMode;
    int bits =
        (enabled(SingleBuf )	? GLUT_SINGLE		:0) |
        (enabled(DoubleBuf )	? GLUT_DOUBLE		:0) |
        (enabled(AccumBuf  )	? GLUT_ACCUM		:0) |
        (enabled(AlphaBuf  )	? GLUT_ALPHA		:0) |
        (enabled(DepthBuf  )	? GLUT_DEPTH		:0) |
        (enabled(StencilBuf)	? GLUT_STENCIL		:0) |
        (enabled(StereoBuf )	? GLUT_STEREO		:0) |
		(enabled(Multisample)	? GLUT_MULTISAMPLE	:0);

	glutInitDisplayMode(GLUT_RGBA | bits);

//	int stat = glutGet(GLUT_DISPLAY_MODE_POSSIBLE);
//	printf("%d\n", stat);

	mImpl->mID = glutCreateWindow(mImpl->mTitle.c_str());

	glutSetWindow(mImpl->mID);
	glutIgnoreKeyRepeat(1);
	WindowImpl::registerCBs();
	WindowImpl::windows()[mImpl->mID] = mImpl;

	AL_GRAPHICS_INIT_CONTEXT;
	onCreate();
	mImpl->scheduleDraw();
}

void WindowGL::destroy(){
	onDestroy();
	mImpl->destroy();
}

void WindowGL::destroyAll(){
	WindowImpl::WindowsMap::iterator it = WindowImpl::windows().begin();
	for(; it != WindowImpl::windows().end(); it++){
		if(it->second && it->second->mWindow){
			it->second->mWindow->destroy();
		}
	}
}

bool WindowGL::cursorHide() const { return mImpl->mCursorHide; }

WindowGL::Dim WindowGL::dimensions() const {
	Dim d;
	glutSetWindow(mImpl->id());
	d.l = glutGet(GLUT_WINDOW_X);
	d.t = glutGet(GLUT_WINDOW_Y);
	d.w = glutGet(GLUT_WINDOW_WIDTH);
	d.h = glutGet(GLUT_WINDOW_HEIGHT);
	return d;
}

bool WindowGL::enabled(DisplayMode::t v) const { return mImpl->mMode & v; }
double WindowGL::fps() const { return mImpl->mFPS; }
double WindowGL::avgFps() const { return mImpl->mAvg; }
bool WindowGL::fullScreen() const { return mImpl->mFullScreen; }
const std::string& WindowGL::title() const { return mImpl->mTitle; }
bool WindowGL::visible() const { return mImpl->mVisible; }

WindowGL& WindowGL::cursor(Cursor::t v){
	mImpl->mCursor = v;

	if(mImpl->created() && !mImpl->mCursorHide){
		switch(v){
			case Cursor::CrossHair:	makeActive(); glutSetCursor(GLUT_CURSOR_CROSSHAIR);
			case Cursor::Pointer:	makeActive(); glutSetCursor(GLUT_CURSOR_INHERIT);
			default:;
		}
	}
	return *this;
}

WindowGL& WindowGL::cursorHide(bool v){
	mImpl->mCursorHide = v;
	makeActive();
	if(mImpl->created() && v)	glutSetCursor(GLUT_CURSOR_NONE);
	else						cursor(mImpl->mCursor);
	return *this;
}

WindowGL& WindowGL::dimensions(const Dim& v){
	mImpl->mWinDim = v;
	if(mImpl->created()){
		glutSetWindow(mImpl->mID);
		glutPositionWindow(v.l, v.t);
		glutReshapeWindow(v.w, v.h);
	}
	return *this;
}

void WindowGL::doFrame(){
	const int winID = mImpl->id();
	const int current = glutGetWindow();
	if(winID != current) glutSetWindow(winID);
//	glutPostRedisplay();
	//glEnable(GL_DEPTH_TEST);

	onFrame();

	glutSwapBuffers();
//	if(current > 0 && current != winID) glutSetWindow(current);
}

WindowGL& WindowGL::fps(double v){
	if(mImpl->mFPS <= 0 && v > 0) mImpl->scheduleDraw();
	mImpl->mFPS = v;
	return *this;
}

WindowGL& WindowGL::fullScreen(bool v){

	// exit full screen
	if(mImpl->mFullScreen && !v){
		#ifdef AL_LINUX
			onDestroy();
			mImpl->gameMode(false);
			onCreate();
			onResize(mImpl->mWinDim.w, mImpl->mWinDim.h);
			hide(); show(); // need to force focus to get key callbacks to work
		#else
			dimensions(mImpl->mWinDim);	// glutReshapeWindow leaves full screen
		#endif
	}

	// enter full screen
	else if(!mImpl->mFullScreen && v){

		mImpl->mWinDim = dimensions();

		#ifdef AL_LINUX
			onDestroy();
			mImpl->gameMode(true);
			onCreate();
			cursorHide(cursorHide());
		#else
			glutSetWindow(mImpl->mID);
			glutFullScreen();
		#endif
	}

	mImpl->mFullScreen = v;
	return *this;
}

WindowGL& WindowGL::hide(){ makeActive(); glutHideWindow(); return *this; }
WindowGL& WindowGL::iconify(){ makeActive(); glutIconifyWindow(); return *this; }
WindowGL& WindowGL::makeActive(){ glutSetWindow(mImpl->id()); return *this; }
WindowGL& WindowGL::show(){ makeActive(); glutShowWindow(); return *this; }

WindowGL& WindowGL::title(const std::string& v){
	mImpl->mTitle = v;
	glutSetWindow(mImpl->mID);
	glutSetWindowTitle(mImpl->mTitle.c_str());
	//printf("WindowGL::title(%s)\n", mImpl->mTitle.c_str());
	return *this;
}

void WindowGL::startLoop(){
	MainLoop::get().start();
//	glutIdleFunc(al_main_tick);
//	glutMainLoop();
}

void WindowGL::stopLoop(){
	WindowGL::destroyAll();
	MainLoop::get().stop();
}

} // al::

