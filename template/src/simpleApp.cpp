/*
Allocore Example: Simple App

Description:
This example demonstrates how to use the App class to create a 3D space with
audio and graphics rendering callbacks.

The App class (and any subclass thereof) has some default keyboard and mouse
controls assigned automatically. These are:

	Keyboard		Action
	'w'				move forward
	'x'				move back
	'a'				move left
	'd'				move right
	'e'				move up
	'c'				move down

	'q'				roll counter-clockwise
	'z'				roll clockwise
	left arrow		turn left
	right arrow		turn right
	up arrow		turn up
	down arrow		turn down

	Mouse
	drag left		turn left
	drag right		turn right
	drag up			turn up
	drag down		turn down

For the camera, the coordinate conventions are:

	-x is left
	+x is right
	-y is down
	+y is up
	-z is forward 
	+z is backward

Author:
Lance Putnam, 6/2011, putnam.lance@gmail.com
*/

#include "allocore/io/al_App.hpp"

using namespace al;

// We inherit from App to create our own custom application
class MyApp : public App{
public:

	double phase;

	// This constructor is where we initialize the application
	MyApp(): phase(0){

		// Configure the camera lens
		lens().near(0.1).far(25).fovy(45);

		// Set navigation position and orientation
		nav().pos(0,0,4);
		nav().quat().fromAxisAngle(0.*M_2PI, 0,1,0);

		// Initialize a single window; anything in App::onDraw will be rendered
		// Arguments: position/dimensions, title, frames/second
		initWindow(Window::Dim(0,0, 600,400), "Untitled", 40);
		
		// Uncomment this to disable the default navigation keyboard/mouse controls
		//window().remove(navControl());

		// Set background color
		//stereo().clearColor(HSV(0,0,1));

		// Initialize audio so that App::onSound is called
		// Arguments: sample rate (Hz), block size, output channels, input channels
		initAudio(44100, 128, 2, 1);
	}


	// This is the audio callback
	virtual void onSound(AudioIOData& io){
	
		// Things here occur at block rate...
	
		// This is the sample loop
		while(io()){
			//float in = io.in(0);
			
			float out1 = 0;
			float out2 = 0;
			
			io.out(0) = out1;
			io.out(1) = out2;
		}
	}


	// This is the application's (graphical) model update.
	// This is called once for each frame of graphics. Typically, you will
	// update your application's geometry, physics, etc. here.
	virtual void onAnimate(double dt){
		// The phase will ramp from 0 to 1 over 1 second. We will use it to
		// animate the color of a sphere.
		phase += dt;
		if(phase >= 1.) phase -= 1.;
	}


	// This is the application's view update.
	// This is called one or more times per frame, for each window, viewport,
	// and eye (for stereoscopic). Typically, this is where you instruct the
	// GPU to render something.
	virtual void onDraw(Graphics& g, const Viewpoint& v){
		
		// Note: we don't need to do all the normal graphics setup as this
		// is handled by the App's stereographic object. We can just draw
		// our geometry immediately!
		
		// Graphics has a Mesh for temporary use
		Mesh& m = g.mesh();
		
		// We must clear the Mesh each frame because we are regenerating its
		// vertices each frame.
		m.reset();
		
		// Set drawing primitive of Mesh
		m.primitive(g.TRIANGLES);

		// Add new vertices to the Mesh in the shape of a sphere.
		// The return value is the number of vertices added.
		int N = addSphere(m, 1, 32, 32);

		// We add new colors for each vertex generated in the call above.
		for(int i=0; i<N; ++i){
			m.color(HSV(0.1, 0.5, al::fold(phase + i*0.5/N, 0.5)+0.5));
		}

		// Finally, command Graphics to draw the Mesh
		g.draw(m);
	}


	// This is called whenever a key is pressed.
	virtual void onKeyDown(const ViewpointWindow& w, const Keyboard& k){
	
		// Use a switch to do something when a particular key is pressed
		switch(k.key()){

		// For printable keys, we just use its character symbol:
		case '1': printf("Pressed 1.\n"); break;
		case 'y': printf("Pressed y.\n"); break;
		case 'n': printf("Pressed n.\n"); break;
		case '.': printf("Pressed period.\n"); break;
		case ' ': printf("Pressed space bar.\n"); break;
		
		// For non-printable keys, we have to use the enums described in the
		// Keyboard class:
		case Keyboard::RETURN: printf("Pressed return.\n"); break;
		case Keyboard::DELETE: printf("Pressed delete.\n"); break;
		case Keyboard::F1: printf("Pressed F1.\n"); break;
		}
	}

	// This is called whenever a mouse button is pressed.
	virtual void onMouseDown(const ViewpointWindow& w, const Mouse& m){
		switch(m.button()){
		case Mouse::LEFT: printf("Pressed left mouse button.\n"); break;
		case Mouse::RIGHT: printf("Pressed right mouse button.\n"); break;
		case Mouse::MIDDLE: printf("Pressed middle mouse button.\n"); break;
		}
	}
	
	// This is called whenever the mouse is dragged.
	virtual void onMouseDrag(const ViewpointWindow& w, const Mouse& m){
		// Get mouse coordinates, in pixels, relative to top-left corner of window
		int x = m.x();
		int y = m.y();
		printf("Mouse dragged: %3d, %3d\n", x,y);
	}
	
	// *****************************************************
	// NOTE: check the App class for more callback functions
};


int main(){
	MyApp().start();
}
