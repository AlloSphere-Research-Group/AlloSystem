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

	WindowImpl(Window * win): mWindow(win){
		if(!glfwInit()){
			// something is wrong!
		} else {
			// Use native sleep functions for timing
			Main::get().driver(Main::SLEEP);
		}
	}

	void handleEvents(){
	}

	void onFrame(){
		mWindow->updateFrameTime(); // Compute actual frame interval
		handleEvents();
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
	//glfwWindowHint(GLFW_RED_BITS, GLFW_DONT_CARE);
	//glfwWindowHint(GLFW_GREEN_BITS, GLFW_DONT_CARE);
	//glfwWindowHint(GLFW_BLUE_BITS, GLFW_DONT_CARE);
	glfwWindowHint(GLFW_ALPHA_BITS, enabled(ALPHA_BUF) ? GLFW_DONT_CARE : 0);
	glfwWindowHint(GLFW_DEPTH_BITS, enabled(DEPTH_BUF) ? GLFW_DONT_CARE : 0);
	glfwWindowHint(GLFW_STENCIL_BITS, enabled(STENCIL_BUF) ? GLFW_DONT_CARE : 0);
	glfwWindowHint(GLFW_SAMPLES, enabled(MULTISAMPLE) ? 4 : 0);
	glfwWindowHint(GLFW_STEREO, enabled(STEREO_BUF) ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_FALSE);

	mImpl->mGLFWWindow = glfwCreateWindow(800, 600, "Hello World", NULL, NULL);

	scheduleDraw(Main::get().now(), 0, mImpl);

	return true;
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
	glfwSetWindowPos(mImpl->mGLFWWindow, mDim.l, mDim.t);
	glfwSetWindowSize(mImpl->mGLFWWindow, mDim.w, mDim.h);
}

void Window::implSetTitle(){
	glfwSetWindowTitle(mImpl->mGLFWWindow, mTitle.c_str());
}

void Window::implSetVSync(){
	glfwSwapInterval(int(mVSync));
}


/*
bool Window::makeCurrent() const {
	glfwMakeContextCurrent(mImpl->mWin);
	return true;
}
*/

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
