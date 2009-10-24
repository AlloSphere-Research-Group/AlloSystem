#include "al_mainloop.h"

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

void tick(al_nsec time, void * userdata) {
	
	al_sec t = time * al_time_ns2s;
	printf("%f\n", al_main_time_sec());
	if (t > 3.0) {
		al_main_exit();
	}
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
	glutIdleFunc (idle);
	
	// initialize mainloop code
	al_main_register(tick, NULL);
	
	// Turn the flow of control over to GLUT
	printf("enter main loop\n");
	glutMainLoop();
	printf("done\n");
	return al_main_quit();
}
