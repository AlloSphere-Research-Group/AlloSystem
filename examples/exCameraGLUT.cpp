#include "system/al_mainloop.h"

#include "protocol/al_Graphics.hpp"
#include "types/al_Camera.hpp"
#include "types/al_MsgTube.hpp"

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


al::Graphics gl;
al::MsgTube inbox, outbox;

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
   
   glColor3d(1, 1, 1);
   
   gl.begin(GL_POINTS);
   for (double i=-1; i<1.; i += 0.1) {
		gl.vertex(i, i, i);
	}
   gl.end();
   
   glBegin(GL_LINES);
    for (double i=-1; i<1.; i += 0.1) {
		glVertex3d(i, i, i);
	}
   
   glEnd();
   
   // Make sure changes appear onscreen
   glutSwapBuffers();   
}

int main (int argc, char * argv[]) {

	printf("setBackendOpenGL %d\n", setBackendOpenGL(&gl));
	 
	// GLUT Window Initialization:
	glutInit (&argc, argv);
	glutInitWindowSize (400, 300);
	glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow ("GLUT example");

	// Register callbacks:
	glutDisplayFunc (display);
	glutIdleFunc (idle);
	
	// initialize mainloop code
	al_main_register(ontick, NULL, onquit);
	
	// Turn the flow of control over to GLUT
	printf("enter main loop\n");
	glutMainLoop();
	printf("done\n");
	al_main_exit();
	return 0;
}
