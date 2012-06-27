/*
Allocore Example: Simple App

Description:
This example demonstrates how to use the App class to create a 3D space with
audio and graphics rendering callbacks.

Author:
Lance Putnam, 6/2011, putnam.lance@gmail.com
*/

#include "alloutil/al_App.hpp"

using namespace al;

// We inherit from App to create our own custom application
class MyApp : public App{
public:

	double phase;

	// Ths constructor is where we initialize the application
	MyApp(): phase(0){
		// Configure our camera lens
		lens().near(0.1).far(25).fovy(45);

		// Set navigation position and orientation
		nav().pos(0,0,4);
		nav().quat().fromAxisAngle(0, 0,0,1);

		// Initialize a single window
		// Anything in App::onDraw will be rendered
		initWindow(Window::Dim(600,400));
		
		// Initialize audio so that App::onSound is called
		initAudio(
			44100,		// sample rate, in Hz
			128,		// block size
			2,			// number of output channels to open
			1			// number of input channels to open
		);
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
	// and eye (for stereoscopic). Typically, this is where you send drawing
	// commands to the GPU.
	virtual void onDraw(Graphics& g, const Viewpoint& v){
		
		// Note: we don't need to do all the normal graphics setup as this
		// is handled by the App's stereographic object. We can just draw
		// our geometry immediately!
		
		Mesh& m = g.mesh();
		
		m.reset();
		m.primitive(g.TRIANGLES);

		int N = addSphere(m, 1, 32, 32);

		for(int i=0; i<N; ++i){
			m.color(HSV(0.1, 0.5, al::fold(phase + i*0.5/N, 0.5)+0.5));
		}

		g.draw(m);
	}
	
	// This is the application's key down controller.
	virtual void onKeyDown(const ViewpointWindow& w, const Keyboard& k){
		printf("Pressed the %c key.\n", k.key());
	}
};


MyApp app;

int main(){
	app.start();
}
