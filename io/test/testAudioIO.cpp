#include <stdio.h>
#include "io/al_AudioIO.h"

using namespace allo;

struct LowPass{
	LowPass(): p(0){}
	float operator()(float v, float f){ return p = p*(1-f) + v*f; }
	float p;
} lpf;

void audioCB(AudioIOData& io){
	for(int i=0; i<io.framesPerBuffer(); ++i){
		float s = -io.in(0)[i];
		//s = lpf(s, 0.2);
		//s = float(i)/io.framesPerBuffer();
		s = 0;		

		io.out(0)[i] = s;
		io.out(1)[i] = s;
	}
}



int main(){

	AudioDevice::printAll();
	AudioIO audioIO(128, 44100, audioCB, 0, 2, 1);
	audioIO.start();

	printf("\nPress 'enter' to quit...\n"); getchar();
	return 0;
}
