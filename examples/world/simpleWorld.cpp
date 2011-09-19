/*
Allocore Example: Simple World

Description:
This example demonstrates how to use the App class to create a 3D space with
audio and graphics rendering callbacks.

Author:
Lance Putnam, 6/2011, putnam.lance@gmail.com
*/

#include "alloutil/al_App.hpp"

using namespace al;

class MyApp : public App{
public:

	virtual void onSound(AudioIOData& io){
		while(io()){
			float in = io.in(0);
			
			float out1 =-in*in*in;
			float out2 = out1;
			
			io.out(0) = out1;
			io.out(1) = out2;
		}
	}
	
	virtual void onAnimate(double dt){
	
	}

	virtual void onDraw(Graphics& g, const Viewpoint& v){
		
		// Note: we don't need to do all the normal graphics setup as this
		// is handled by the App's stereographic object. We can just draw
		// our geometry immediately!
		
		Mesh& m = g.mesh();
		
		m.reset();
		m.primitive(g.TRIANGLES);
		m.color(1,0,0);
		addSphere(m);

		g.draw(m);
	}

};

MyApp app;

int main(){
	app.camera().near(0.1).far(25);
	app.nav().pos(0,0,4);
	app.nav().quat().fromAxisAngle(0, 0,0,1);

	// Initialize a single window
	// Anything in App::onDraw will be rendered
	app.initWindow(Window::Dim(600,400));
	
	// Initialize audio so that App::onSound is called
	app.initAudio(
		44100,		// sample rate, in Hz
		128,		// block size
		2,			// number of output channels to open
		1			// number of input channels to open
	);

	app.start();
	return 0;
}
