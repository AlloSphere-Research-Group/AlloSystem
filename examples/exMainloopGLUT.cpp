

#include <list>
#include <string>

#include "system/al_mainloop.h"
#include "graphics/al_Context.hpp"

namespace al {


namespace DisplayMode{
	enum t{
		SingleBuf	= 1<<0,		/**< Single-buffered */
		DoubleBuf	= 1<<1,		/**< Double-buffered */
		StereoBuf	= 1<<2,		/**< Do left-right stereo buffering */
		AccumBuf	= 1<<3,		/**< Use accumulation buffer */
		AlphaBuf	= 1<<4,		/**< Use alpha buffer */
		DepthBuf	= 1<<5,		/**< Use depth buffer */
		StencilBuf	= 1<<6,		/**< Use stencil buffer */
		Multisample = 1<<7,		/**< Multisampling support */
		DefaultBuf	= DoubleBuf|AlphaBuf|DepthBuf /**< Default display mode */
	};
	inline t operator| (const t& a, const t& b){ return t(int(a) | int(b)); }
	inline t operator& (const t& a, const t& b){ return t(int(a) & int(b)); }
}


class WindowGLUT : public Context {
public:


protected:
	friend class MainLoopGLUT;
	
	class MainLoopGLUT * mManager;	
	
	/// protected constructor - a WindowGLUT is managed by a MainLoopGLUT
	WindowGLUT();
	~WindowGLUT();
};


class MainLoopGLUT {
public:

	struct Dim{
		Dim(int w_, int h_, int l_=0, int t_=0): l(l_), t(t_), w(w_), h(h_){}
		Dim(int v=0): l(0), t(0), w(v), h(v){}
		int l,t,w,h;
	};

	MainLoopGLUT() {
		
	}

	WindowGLUT * newWindow(
		const Dim& dim,
		const std::string title,
		double fps=40,
		DisplayMode::t mode = DisplayMode::DefaultBuf
	) {
		return new WindowGLUT();
	}
	
	/// invalidates the win pointer
	void removeWindow(WindowGLUT * win) {
		mWindows.remove(win); 
		delete win;
	}
	
protected:
	std::list<WindowGLUT *> mWindows;
};




} // al::


/*

	TODO: Combine this mainloop stuff with the alWindowGLUT stuff

*/

#ifdef AL_OSX
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include "gl.h"
#include "glut.h"
#endif

#include <stdio.h>
#include <stdlib.h>

void ontick(al_nsec time, void * userdata) {
	al_sec t = time * al_time_ns2s;
	printf("%f\n", t);
	if (t > 3.0) {
		al_main_exit();
	}
}

void onquit(void * userdata) {
	exit(0);
}

void idle(void) 
{
	/* trigger mainloop here */
	al_main_tick();
   
	/* force redraw */
	glutPostRedisplay();
}

void display(void)
{
   // Clear frame buffer and depth buffer
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   // Set up viewing transformation, looking down -Z axis
   glLoadIdentity();
   gluLookAt(0, 0, -4, 0, 0, -1, 0, 1, 0);
   
   
   // Make sure changes appear onscreen
   glutSwapBuffers();
}

int main (int argc, char * argv[]) {
    
	// GLUT Window Initialization:
	glutInit (&argc, argv);
	glutInitWindowSize (400, 300);
	glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow ("GLUT example");

	// Register callbacks:
	glutDisplayFunc (display);
	glutIdleFunc (al_main_tick);
	
	// initialize mainloop code
	al_main_register(ontick, NULL, onquit);
	
	// Turn the flow of control over to GLUT
	printf("enter main loop\n");
	glutMainLoop();
	printf("done\n");
	al_main_exit();
	return 0;
}
