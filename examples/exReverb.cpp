#include <stdio.h>
#include <stdlib.h>
#include "io/al_AudioIO.hpp"
#include "io/al_WindowGL.hpp"
#include "protocol/al_Graphics.hpp"
#include "sound/al_Reverb.hpp"

using namespace al;

Reverb<float> reverb;


void audioCB(AudioIOData& io){
	for(int i=0; i<io.framesPerBuffer(); ++i){
		float s = io.in(0)[i];

		float sl, sr;
		reverb(s, sl, sr);
		sl = s + 0.2*sl;
		sr = s + 0.2*sr;

		io.out(0)[i] = sl;
		io.out(1)[i] = sr;
	}
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
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.loadIdentity();
		gl.viewport(0,0, dimensions().w, dimensions().h);

	}

	gfx::Graphics gl;
};


int main (int argc, char * argv[]){

	MyWindow win;
	win.create(WindowGL::Dim(200,200,100), "Window 1", 40);

	AudioIO audioIO(256, 44100, audioCB, 0, 2, 1);
	audioIO.start();

	WindowGL::startLoop();
	return 0;
}
