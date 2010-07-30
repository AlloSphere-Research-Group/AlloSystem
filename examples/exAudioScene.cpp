#include <stdio.h>
#include <stdlib.h>
#include "io/al_AudioIO.hpp"
#include "io/al_WindowGL.hpp"
#include "math/al_Random.hpp"
#include "protocol/al_Graphics.hpp"
#include "sound/al_Reverb.hpp"
#include "sound/al_AudioScene.hpp"

using namespace al;

#define NUM_FRAMES 256

Reverb<float> reverb;

AudioScene scene(3, 1, NUM_FRAMES);

Listener * listener;

rnd::Random<> randy;
SoundSource src;

void audioCB(AudioIOData& io){
	int numFrames = io.framesPerBuffer();
	
	

	for(int i=0; i<io.framesPerBuffer(); ++i){
		float s = io.in(0)[i];

		float sl, sr;
		reverb(s, sl, sr);
		sl = 0.2*sl;
		sr = 0.2*sr;

		io.out(0)[i] = sl;
		io.out(1)[i] = sr;
	}
	
	for (int i=0; i<numFrames; i++) {
		src.writeSample(randy.uniformS());
	}
	
	scene.encode(numFrames, io.framesPerSecond());
	scene.render(io.out(0), numFrames);
}


struct MyWindow : WindowGL{
	
	void onKeyDown(const Keyboard& k){
		switch(k.key()){
			case Key::Escape: fullScreenToggle(); break;
			case 'q': if(k.ctrl()) WindowGL::stopLoop(); break;
			case 'w': if(k.ctrl()) destroy(); break;
			case 'h': if(k.ctrl()) hide(); break;
			case 'm': if(k.ctrl()) iconify(); break;
			case 'c': if(k.ctrl()) cursorHideToggle(); break;
		}
	}

	void onMouseDown(const Mouse& m){}
	void onMouseUp(const Mouse& m){}
	void onMouseDrag(const Mouse& m){}

	void onFrame(){
		gl.clear(gfx::AttributeBit::ColorBuffer | gfx::AttributeBit::DepthBuffer);
		gl.loadIdentity();
		gl.viewport(0,0, dimensions().w, dimensions().h);

	}

	gfx::Graphics gl;
};


int main (int argc, char * argv[]){

	listener = &scene.createListener(2);
	scene.addSource(src);

	MyWindow win;
	win.create(WindowGL::Dim(200,200,100), "Window 1", 40);

	AudioIO audioIO(NUM_FRAMES, 44100, audioCB, 0, 2, 1);
	audioIO.start();

	WindowGL::startLoop();
	return 0;
}
