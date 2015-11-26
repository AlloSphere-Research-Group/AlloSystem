#include "allocore/io/al_Window.hpp"
#include "allocore/system/al_Config.h"
#include "allocore/system/al_MainLoop.hpp"
#include "allocore/graphics/al_OpenGL.hpp"

#include <map>
#include <iostream>
#include <cmath>

using namespace std;

namespace al {

class WindowImpl : public Main::Handler {
public:

	typedef std::map<GLFWwindow*, WindowImpl*> WindowsMap;
	typedef std::map<int, int> KeyMap;


	WindowImpl(Window* w) : mWindow(w), mDimPrev(0) {
		resetState();
		initKeymap(); // set static keymapping map
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
			glfwMakeContextCurrent(mGLFWwindow);
		}
		return d;
	}

	void makeMainWindow(){
		glfwMakeContextCurrent(mGLFWwindow);
	}

	void destroy(){
		if(created()){
			windows().erase(mGLFWwindow);
			glfwDestroyWindow(mGLFWwindow);
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
		return nullptr;
	}

	static int remapKey(int key){
		auto search = keymap().find(key);
    if(search != keymap().end()) return search->second;
		return 0;
	}

	static void cbKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods){
		Window* w = getWindow();
		if (!w) return;

		// first set modifiers
		Keyboard& k = w->mKeyboard;
		k.alt(mods & GLFW_MOD_ALT);
		k.ctrl(mods & GLFW_MOD_CONTROL);
		k.shift(mods & GLFW_MOD_SHIFT);

		switch(action) {
			case GLFW_PRESS:
				k.setKey(remapKey(key), true);
				w->callHandlersOnKeyDown();
				break;
			case GLFW_REPEAT:	break;
			case GLFW_RELEASE:
				k.setKey(remapKey(key), false);
				w->callHandlersOnKeyUp();
				break;
		}
	}

	static void cbMouse(GLFWwindow* window, int button, int action, int mods){
		Window * w = getWindow();
		if(!w) return;

		Mouse& m = w->mMouse;

		switch(button){
			case GLFW_MOUSE_BUTTON_LEFT: button = Mouse::LEFT; break;
			case GLFW_MOUSE_BUTTON_RIGHT: button = Mouse::MIDDLE; break;
			case GLFW_MOUSE_BUTTON_MIDDLE: button = Mouse::RIGHT; break;
			default: button = Mouse::EXTRA;		// unrecognized button
		}

		Keyboard& k = w->mKeyboard;
		k.alt(mods & GLFW_MOD_ALT);
		k.ctrl(mods & GLFW_MOD_CONTROL);
		k.shift(mods & GLFW_MOD_SHIFT);

		if (GLFW_PRESS == action) {
			double xpos, ypos;
			glfwGetCursorPos (window, &xpos, &ypos);
			m.position((int)round(xpos), (int)round(ypos));
			m.button(button, true);
			w->callHandlersOnMouseDown();
		}
		if (GLFW_RELEASE == action) {
			double xpos, ypos;
			glfwGetCursorPos (window, &xpos, &ypos);
			m.position((int)round(xpos), (int)round(ypos));
			m.button(button, false);
			w->callHandlersOnMouseUp();
		}
	}

	static void cbMotion(GLFWwindow* window, double mx, double my) {
		Window * win = getWindow();
		if(!win) return;

		win->mMouse.position((int)round(mx), (int)round(my));

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS ||
		    glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS ||
		    glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS)
		{
			win->callHandlersOnMouseDrag();
			return;
		}

		win->callHandlersOnMouseMove();
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

	// key mapping function, original version by Donghao Ren
	// 
	static void initKeymap() {
		static bool initd = false;
		if (initd) return;
		initd = false;

		KeyMap& km = keymap();

		km[GLFW_KEY_SPACE] = ' ';
		km[GLFW_KEY_APOSTROPHE] = '\'';
		km[GLFW_KEY_COMMA] = ',';
		km[GLFW_KEY_MINUS] = '-';
		km[GLFW_KEY_PERIOD] = '.';
		km[GLFW_KEY_SLASH] = '/';
		km[GLFW_KEY_0] = '0';
		km[GLFW_KEY_1] = '1';
		km[GLFW_KEY_2] = '2';
		km[GLFW_KEY_3] = '3';
		km[GLFW_KEY_4] = '4';
		km[GLFW_KEY_5] = '5';
		km[GLFW_KEY_6] = '6';
		km[GLFW_KEY_7] = '7';
		km[GLFW_KEY_8] = '8';
		km[GLFW_KEY_9] = '9';
		km[GLFW_KEY_SEMICOLON] = ';';
		km[GLFW_KEY_EQUAL] = '=';
		km[GLFW_KEY_A] = 'a';
		km[GLFW_KEY_B] = 'b';
		km[GLFW_KEY_C] = 'c';
		km[GLFW_KEY_D] = 'd';
		km[GLFW_KEY_E] = 'e';
		km[GLFW_KEY_F] = 'f';
		km[GLFW_KEY_G] = 'g';
		km[GLFW_KEY_H] = 'h';
		km[GLFW_KEY_I] = 'i';
		km[GLFW_KEY_J] = 'j';
		km[GLFW_KEY_K] = 'k';
		km[GLFW_KEY_L] = 'l';
		km[GLFW_KEY_M] = 'm';
		km[GLFW_KEY_N] = 'n';
		km[GLFW_KEY_O] = 'o';
		km[GLFW_KEY_P] = 'p';
		km[GLFW_KEY_Q] = 'q';
		km[GLFW_KEY_R] = 'r';
		km[GLFW_KEY_S] = 's';
		km[GLFW_KEY_T] = 't';
		km[GLFW_KEY_U] = 'u';
		km[GLFW_KEY_V] = 'v';
		km[GLFW_KEY_W] = 'w';
		km[GLFW_KEY_X] = 'x';
		km[GLFW_KEY_Y] = 'y';
		km[GLFW_KEY_Z] = 'z';
		km[GLFW_KEY_LEFT_BRACKET] = '[';
		km[GLFW_KEY_BACKSLASH] = '\\';
		km[GLFW_KEY_RIGHT_BRACKET] = ']';
		km[GLFW_KEY_GRAVE_ACCENT] = '`';
		km[GLFW_KEY_ESCAPE] = Keyboard::ESCAPE;
		km[GLFW_KEY_ENTER] = Keyboard::ENTER;
		km[GLFW_KEY_TAB] = Keyboard::TAB;
		km[GLFW_KEY_BACKSPACE] = Keyboard::BACKSPACE;
		km[GLFW_KEY_INSERT] = Keyboard::INSERT;
		km[GLFW_KEY_DELETE] = Keyboard::DELETE;
		km[GLFW_KEY_RIGHT] = Keyboard::RIGHT;
		km[GLFW_KEY_LEFT] = Keyboard::LEFT;
		km[GLFW_KEY_DOWN] = Keyboard::DOWN;
		km[GLFW_KEY_UP] = Keyboard::UP;
		km[GLFW_KEY_PAGE_UP] = Keyboard::PAGE_UP;
		km[GLFW_KEY_PAGE_DOWN] = Keyboard::PAGE_DOWN;
		km[GLFW_KEY_HOME] = Keyboard::HOME;
		km[GLFW_KEY_END] = Keyboard::END;
		km[GLFW_KEY_F1] = Keyboard::F1;
		km[GLFW_KEY_F2] = Keyboard::F2;
		km[GLFW_KEY_F3] = Keyboard::F3;
		km[GLFW_KEY_F4] = Keyboard::F4;
		km[GLFW_KEY_F5] = Keyboard::F5;
		km[GLFW_KEY_F6] = Keyboard::F6;
		km[GLFW_KEY_F7] = Keyboard::F7;
		km[GLFW_KEY_F8] = Keyboard::F8;
		km[GLFW_KEY_F9] = Keyboard::F9;
		km[GLFW_KEY_F10] = Keyboard::F10;
		km[GLFW_KEY_F11] = Keyboard::F11;
		km[GLFW_KEY_F12] = Keyboard::F12;
  }

	void registerCBs(){
		// glutKeyboardFunc(cbKeyboard);
		// glutKeyboardUpFunc(cbKeyboardUp);
		// glutMouseFunc(cbMouse);
		// glutMotionFunc(cbMotion);
		// glutPassiveMotionFunc(cbPassiveMotion);
		// glutSpecialFunc(cbSpecial);
		// glutSpecialUpFunc(cbSpecialUp);
		// glutReshapeFunc(cbReshape);
		// glutVisibilityFunc(cbVisibility);

		// glfwSetWindowUserPointer(window, this);
		glfwSetKeyCallback(mGLFWwindow, cbKeyboard);
		// glfwSetWindowPosCallback(window, cb_windowpos);
		// glfwSetWindowSizeCallback(mGLFWwindow, cbReshape);
		// glfwSetFramebufferSizeCallback(window, cb_framebuffersize);
		// glfwSetWindowCloseCallback(window, cb_windowclose);
		// glfwSetWindowRefreshCallback(window, cb_windowrefresh);
		// glfwSetWindowFocusCallback(window, cb_windowfocus);
	  // glfwSetErrorCallback(errorCallback);
	  glfwSetMouseButtonCallback(mGLFWwindow, cbMouse);
	  glfwSetCursorPosCallback(mGLFWwindow, cbMotion);
	}

	void onTick() {
		// std::cout << "onTick, WindowImpl" << std::endl;
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

	static KeyMap& keymap() {
		static KeyMap* k = new KeyMap;
		return *k;
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

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	mImpl->mGLFWwindow = glfwCreateWindow(w, h, "title", NULL, NULL);
	if (!mImpl->created()) {
		return false;
	}

	glfwMakeContextCurrent(mImpl->mGLFWwindow);

	std::cout << "checking versions..." << std::endl;
	const GLubyte* renderer = glGetString(GL_RENDERER);
  std::cout << "renderer: " << renderer << std::endl;
  int mj = glfwGetWindowAttrib(mImpl->mGLFWwindow, GLFW_CONTEXT_VERSION_MAJOR);
  int mn = glfwGetWindowAttrib(mImpl->mGLFWwindow, GLFW_CONTEXT_VERSION_MINOR);
  std::cout << "opengl window version: " << mj << "." << mn << std::endl;
  char* glsl_version = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
  std::cout << "glsl version: " << glsl_version << std::endl;

	Main::get().add(*mImpl);

	mImpl->registerCBs();
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
		// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

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