#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>

#include "al_runloop.h"
#include "al_mainloop.h"

#import "AlOpenGLView.h"
#import "AlOpenGlWindow.h"

static GUI * gui = 0;
static NSAutoreleasePool * pool = 0;

void initGUI() {
	pool = [[NSAutoreleasePool alloc] init];
	
	gui = [[GUI alloc] init];
    [gui performSelectorOnMainThread:@selector(makeWindow)
                          withObject:nil
                       waitUntilDone:YES];
	//[gui makeWindow];
    [gui lock];
    [gui makeCurrent];	
	
	printf("made current\n");
}

void draw(al_sec t) {
	
	[gui makeCurrent];
	
	glViewport(0, 0, 200, 200);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	
	glColor3d(1, 1, 0);
	glBegin(GL_LINES);
	for (int i=-10; i<10; i++) {
		glVertex3d(cos(t+i), sin(i + t/10), 0);
	}
	glEnd();
	
	glFlush();
	
	[pool release];
	
	[gui flip];
    [gui unlock];
    
    pool = [[NSAutoreleasePool alloc] init];
    [gui lock];
}


void m_ontick(al_nsec time, void * userdata) {
	al_sec t = time * al_time_ns2s;
	printf("main %f\n", t);
	
	// simulate a half-second drop-out every 10 seconds:
	if (((int)t) % 10 == 0) {
		al_sleep(0.5);
	}
	
	if (t > 0.1)
		al_main_exit();
}

void m_onquit(void * userdata) {
	[[NSApplication sharedApplication] terminate: nil];
}

void r_ontick(al_nsec time, void * userdata) {
	al_sec t = time * al_time_ns2s;
	//printf("runl %f\n", t);
	
	// try drawing GL here
	if (gui == 0) {
		initGUI();
	} else {
		draw(t);
	}
	
	printf(".");
	
//	// simulate a half-second drop-out every 4 seconds:
//	if (((int)t) % 4 == 0) {
//		al_sleep(0.5);
//	}
}

void r_onterminate(void * userdata) {
	printf("onterminate\n");
}

int main(int argc, char *argv[])
{
	al_main_attach(0.1, m_ontick, NULL, m_onquit);
	runloop rl = al_runloop_create(0.01, r_ontick, NULL, r_onterminate);
	
	return NSApplicationMain(argc,  (const char **) argv);
}
