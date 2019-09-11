/*
Allocore Example: VCR

Description:
The example demonstrates how to record images and sounds to disk.

Press 's' to start recording
Press 'q' to stop

Author:
Graham Wakefield, Apr 2012
*/

#include <stdio.h>
#include "allocore/al_Allocore.hpp"
#include "alloutil/al_VCR.hpp"

using namespace al;

rnd::Random<> rng;

VCR vcr;

struct MyApp : Window {

	MyApp()
	:	audio(256, 44100, audioCB, this, 2, 2)
	{
		image.format(3, AlloUInt8Ty, 2048, 2048);

		append(*new StandardWindowKeyControls);
		create(Window::Dim(100, 0, 400,300), "App", 30);

		audio.start();
	}

	static void filler(unsigned char * c, double normx, double normy) {
		int frame = vcr.frame();

		c[0] = normx * frame * 255.;
		c[1] = normy * frame * 255.;
		c[2] = rng.uniform() * 255.;
	}

	bool onFrame(){
		// for now just fill image with random data.
		image.fill(filler);

		printf("frame %d\n", vcr.frame());

		// write:
		vcr.image(image);

		return true;
	}

	bool onKeyDown(const Keyboard& k){
		switch (k.key()) {
			case 's':
				vcr.start(&audio);
				break;
			case 'q':
				vcr.stop();
				break;
			default:
				break;
		}

		return 1;
	}

	static void audioCB(AudioIOData& io){
		MyApp * self = (MyApp *)io.user();
		while(io()){
			self->phase += 0.001;
			float s0 = rng.uniformS() * sin(self->phase);
			float s1 = sin(self->phase * 100.);
			io.out(0) = s0;
			io.out(1) = s1;
		}
	}

	AudioIO audio;
	Array image;
	float phase;
};

int main(){
	MyApp app;
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    vcr.setPath(cwd);


	MainLoop::start();
	return 0;
}
