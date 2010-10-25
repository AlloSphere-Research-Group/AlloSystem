/*
Description:
This demonstrates how to construct a moving object in space that generates its 
own sound and graphics.

*/

#include "al_Allocore.hpp"

struct Agent : public Nav{

	void generateAudio(AudioIOData& io){
	
	}

};

#define NUM_FRAMES 256
AudioScene scene(3, 1, NUM_FRAMES);
Reverb<float> reverb;
Listener * listener;
SoundSource src;


void audioCB(AudioIOData& io){
	int numFrames = io.framesPerBuffer();

	for(int i=0; i<numFrames; ++i){
		float s = io.in(0,i);

		float sl, sr;
		reverb(s, sl, sr);
		sl = 0.2*sl;
		sr = 0.2*sr;

		io.out(0,i) = sl;
		io.out(1,i) = sr;
	}
	
	for(int i=0; i<numFrames; i++){
		src.writeSample(rnd::uniform(1.,-1.)*0.2);
	}
	
	scene.encode(numFrames, io.framesPerSecond());
	scene.render(&io.out(0,0), numFrames);
}


struct MyWindow : Window{
	
	MyWindow(): gl(new gfx::GraphicsBackendOpenGL)
	{}
	
	bool onKeyDown(const Keyboard& k){
		switch(k.key()){
			default: return true;
		}
	}

	bool onFrame(){
		gl.clear(gfx::COLOR_BUFFER_BIT | gfx::DEPTH_BUFFER_BIT);
		gl.loadIdentity();
		gl.viewport(0,0, dimensions().w, dimensions().h);
		return true;
	}

	gfx::Graphics gl;
};


int main (int argc, char * argv[]){

	listener = &scene.createListener(2);
	scene.addSource(src);

	MyWindow win;
	win.add(new StandardWindowKeyControls);
	win.create(Window::Dim(200,200,100), "Window 1", 40);

	AudioIO audioIO(NUM_FRAMES, 44100, audioCB, 0, 2, 1);
	audioIO.start();

	MainLoop::start();
	return 0;
}
