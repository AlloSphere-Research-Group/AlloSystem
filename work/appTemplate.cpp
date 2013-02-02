/*
Description:
This is a very stripped down App subclass that can be used as a starting 
template for projects. This is NOT an example of how to use the App class. 
See allocore/examples/io/simpleApp.cpp for an overview of the App class.

Author:
Lance Putnam, 9/2012, putnam.lance@gmail.com
*/

#include "allocore/io/al_App.hpp"

using namespace al;

class MyApp : public App{
public:

	MyApp(){
		//lens().near(0.1).far(25).fovy(45);
		//nav().pos(0,0,4);
		//nav().quat().fromAxisAngle(0, 0,0,1);
		//initWindow(Window::Dim(600,400), "", 40, Window::DEFAULT_BUF);
		//stereo().clearColor(HSV(0,0,1));
		//window().remove(navControl());
		//initAudio(44100, 128, 2,1);
	}

	// Audio callback
	virtual void onSound(AudioIOData& io){
		while(io()){
			//float in = io.in(0);
			float out1 = 0;
			float out2 = 0;

			io.out(0) = out1;
			io.out(1) = out2;
		}
	}

	// Graphics callbacks
	virtual void onAnimate(double dt){}
	virtual void onDraw(Graphics& g, const Viewpoint& v){}

	// Keyboard/mouse input callbacks
	virtual void onKeyDown(const ViewpointWindow& w, const Keyboard& k){
		switch(k.key()){
		case 'n': break;
		}
	}
	virtual void onKeyUp(const ViewpointWindow& w, const Keyboard& k){}
	virtual void onMouseDown(const ViewpointWindow& w, const Mouse& m){}
	virtual void onMouseUp(const ViewpointWindow& w, const Mouse& m){}
	virtual void onMouseDrag(const ViewpointWindow& w, const Mouse& m){}
	virtual void onMouseMove(const ViewpointWindow& w, const Mouse& m){}

	// Window-related callbacks
	virtual void onCreate(const ViewpointWindow& win){}
	virtual void onDestroy(const ViewpointWindow& win){}
	virtual void onResize(const ViewpointWindow& win, int dw, int dh){}
};


int main(){
	MyApp().start();
}
