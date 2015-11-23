#include "allocore/io/al_Window.hpp"
#include "allocore/system/al_Config.h"
#include "allocore/system/al_MainLoop.hpp"
#include "allocore/graphics/al_OpenGL.hpp"

#include <map>
#include <iostream>

namespace al {

class WindowImpl : public Main::Handler {
public:

	typedef std::map<GLFWwindow*, WindowImpl*> WindowsMap;

	WindowImpl(Window* w) : mWindow(w), mDimPrev(0) {
		resetState();
	}

	~WindowImpl() {
		destroy();
	}

	void resetState() {
		mGLFWwindow = nullptr;
		mIDGameMode = -1;
		mInGameMode = false;
	}

	GLFWwindow* glfwWindow() { return mGLFWwindow; }
	bool created() const { return mGLFWwindow != nullptr; }

	Window::Dim dimensionsGLFW() const {
		Window::Dim d(0,0,0,0);
		if(created()){
			// glutSetWindow(id());
			glfwMakeContextCurrent(mGLFWwindow);
			// d.l = glutGet(GLUT_WINDOW_X);
		}
		return d;
	}

	void makeMainWindow(){
		// glutSetWindow(id());
		glfwMakeContextCurrent(mGLFWwindow);
	}

	void destroy(){
		if(created()){
			glfwDestroyWindow(mGLFWwindow);
			windows().erase(mGLFWwindow);
			resetState();
		}
	}

	void gameMode(bool v) {
		// fullscreen
		if (v) {
			mInGameMode = true;
		}
		else {
			mInGameMode = false;
		}
	}

	static Window * getWindow(){
		WindowImpl * w = getWindowImpl();
		return w ? w->mWindow : 0;
	}

	static WindowImpl * getWindowImpl(){
		return getWindowImpl(glfwGetCurrentContext());
	}

	static WindowImpl * getWindowImpl(GLFWwindow* w){
		WindowsMap::iterator it = windows().find(w);
		if(windows().end() != it){
			return it->second;
		}
		return 0;
	}

	static void setModifiers(Keyboard& k){
		// int mod = glutGetModifiers();
		// k.alt  (mod & GLUT_ACTIVE_ALT);
		// k.ctrl (mod & GLUT_ACTIVE_CTRL);
		// k.shift(mod & GLUT_ACTIVE_SHIFT);
		//printf("a:%d c:%d s:%d\n", k.alt(), k.ctrl(), k.shift());
	}

	// An invalid key to indicate an error
	static const int INVALID_KEY = -1;

	static int remapKey(int key, bool special){
		// !?
		return key;
	}

	static void cbKeyboard(unsigned char key, int x, int y){
		Window * win = getWindow();
		if(win){
			key = remapKey(key, false);
			win->mKeyboard.setKey(key, true);
			setModifiers(win->mKeyboard);
			win->callHandlersOnKeyDown();
		}
	}

	static void cbKeyboardUp(unsigned char key, int x, int y){
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
		Window * win = getWindow();
		if(win){
			// switch(btn){
				// case GLUT_LEFT_BUTTON:		btn = Mouse::LEFT; break;
				// case GLUT_MIDDLE_BUTTON:	btn = Mouse::MIDDLE; break;
				// case GLUT_RIGHT_BUTTON:		btn = Mouse::RIGHT; break;
				// default:					btn = Mouse::EXTRA;		// unrecognized button
			// }

			// update modifiers here for shift-mouse etc.
			WindowImpl::setModifiers(win->mKeyboard);

			// Mouse& m = win->mMouse;
			// if(GLUT_DOWN == state){
			// 	m.position(ax, ay); m.button(btn, true);
			// 	win->callHandlersOnMouseDown();
			// }
			// else if(GLUT_UP == state){
			// 	m.position(ax, ay);	m.button(btn, false);
			// 	win->callHandlersOnMouseUp();
			// }
		}
	}

	static void cbMotion(int ax, int ay){
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


	static void cbReshape(int w, int h){
		Window * win = getWindow();
		if(win){
			Window::Dim& dimCurr = win->mDim;
			if(dimCurr.w != w || dimCurr.h != h){
				dimCurr.w = w;
				dimCurr.h = h;
				win->callHandlersOnResize(w, h);
				// needed to get title back after exiting full screen
				// win->title(win->title());
			}
		}
	}

	static void cbVisibility(int v){
		Window * win = getWindow();
		if(win){
			// win->mVisible = (v == GLUT_VISIBLE);
			win->callHandlersOnVisibility(win->mVisible);
		}
	}

	static void registerCBs(){
		// glutKeyboardFunc(cbKeyboard);
		// glutKeyboardUpFunc(cbKeyboardUp);
		// glutMouseFunc(cbMouse);
		// glutMotionFunc(cbMotion);
		// glutPassiveMotionFunc(cbPassiveMotion);
		// glutSpecialFunc(cbSpecial);
		// glutSpecialUpFunc(cbSpecialUp);
		// glutReshapeFunc(cbReshape);
		// glutVisibilityFunc(cbVisibility);
	}

	void onTick() {
		std::cout << "onTick, WindowImpl" << std::endl;
		if (glfwGetCurrentContext() != mGLFWwindow) {
			glfwMakeContextCurrent(mGLFWwindow);
		}
		onFrame();
		glfwPollEvents();
		glfwSwapBuffers(mGLFWwindow);
	}

	void onExit() {

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
		// const int winID = id();
		// const int current = glutGetWindow();
		// if(winID != current) glutSetWindow(winID);
		// mWindow->callHandlersOnFrame();
		// const char * err = errorString(true);
		// if(err[0]){
		// 	AL_WARN_ONCE("Error after rendering frame in window (id=%d): %s", winID, err);
		// }
		// glutSwapBuffers();
	}

	static WindowsMap& windows(){
		static WindowsMap* v = new WindowsMap;
		return *v;
	}

	Window * mWindow;
	GLFWwindow* mGLFWwindow;
	int mIDGameMode;
	Window::Dim mDimPrev;

	bool mInGameMode;

	friend class Window;
};

// -----------------------------------------------------------------------------

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
	Main::get().driver(Main::GLFW);

	int w = mDim.w;
	int h = mDim.h;
	mDim.w = 0;
	mDim.h = 0;
	mImpl->mDimPrev.set(0,0,0,0);

	mImpl->mGLFWwindow = glfwCreateWindow(w, h, "title", NULL, NULL);
	if (!mImpl->created()) {
		return false;
	}
	Main::get().add(*mImpl);

	WindowImpl::registerCBs();
	WindowImpl::windows()[mImpl->mGLFWwindow] = mImpl;

	AL_GRAPHICS_INIT_CONTEXT; // init glew
	vsync(mVSync);

	callHandlersOnCreate();

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
	// glutPositionWindow(mDim.l, mDim.t);

	// Set mDim extent to actual window extent so reshape callback triggers
	// handlers.
	int w = mDim.w;
	int h = mDim.h;
	// mDim.w = glutGet(GLUT_WINDOW_WIDTH);
	// mDim.h = glutGet(GLUT_WINDOW_HEIGHT);
	// glutReshapeWindow(w, h);
}

void Window::implSetCursor(){
	if(!mCursorHide){
		mImpl->makeMainWindow();
		// switch(mCursor){
			// case CROSSHAIR:	glutSetCursor(GLUT_CURSOR_CROSSHAIR); break;
			// case POINTER:	glutSetCursor(GLUT_CURSOR_INHERIT); break;
			// default:;
		// }
	}
}

void Window::implSetCursorHide(){
	mImpl->makeMainWindow();
	// if(mCursorHide)	glutSetCursor(GLUT_CURSOR_NONE);
	// else			cursor(mCursor);
}

void Window::implSetFPS(){
	// Nothing to do here except ensure the scheduler has started.
	// Note: we can safely call scheduleDraw multiple times...
	// mImpl->scheduleDraw();
}

void Window::implSetFullScreen(){

	// Note that on Linux, we must use GLUT's "game mode" to get a borderless
	// fullscreen window.

	// enter full screen
	// if(mFullScreen){
	// 	#ifdef AL_LINUX
	// 		callHandlersOnDestroy();
	// 		// GLUT automatically calls reshape CB upon entering game mode...
	// 		mImpl->gameMode(true);
	// 		callHandlersOnCreate();
	// 		cursorHide(cursorHide());
	// 	#else
	// 		glutSetWindow(mImpl->mID);
	// 		mImpl->mDimPrev = mImpl->dimensionsGLUT();
	// 		glutFullScreen(); // calls glutReshapeWindow
	// 	#endif
	// }

	// // exit full screen
	// else {
	// 	#ifdef AL_LINUX
	// 		callHandlersOnDestroy();
	// 		mImpl->gameMode(false);
	// 		mDim = mImpl->dimensionsGLUT();
	// 		callHandlersOnCreate();
	// 		callHandlersOnResize(mDim.w, mDim.h);
	// 		hide(); show(); // need to force focus to get key callbacks to work
	// 	#else
	// 		// Calls glutReshapeWindow which exits from a glutFullScreen call.
	// 		// This also calls our reshape callback which sets the mDim member.
	// 		dimensions(mImpl->mDimPrev);
	// 	#endif
	// }
}

void Window::implSetTitle(){
	mImpl->makeMainWindow();
	// glutSetWindowTitle(mTitle.c_str());
	//printf("Window::title(%s)\n", mTitle.c_str());
}

// See: https://www.opengl.org/wiki/Swap_Interval
void Window::implSetVSync(){
	mImpl->makeMainWindow();
	// #if defined AL_OSX
	// 	GLint VBL = GLint(mVSync);
	// 	CGLContextObj ctx = CGLGetCurrentContext();
	// 	CGLSetParameter(ctx, kCGLCPSwapInterval, &VBL);
	// #elif defined AL_LINUX
	// #elif defined AL_WINDOWS
	// #endif
}

Window& Window::hide(){
	mImpl->makeMainWindow();
	// glutHideWindow();
	return *this;
}

Window& Window::iconify(){
	mImpl->makeMainWindow();
	// glutIconifyWindow();
	return *this;
}

Window& Window::show(){
	mImpl->makeMainWindow();
	// glutShowWindow();
	return *this;
}

}