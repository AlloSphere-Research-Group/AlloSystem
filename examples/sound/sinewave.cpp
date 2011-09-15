/*
Allocore Example: Sine wave

Description:
This demonstrates how to play a sine wave using Gamma.

Author:
Matt Wright, 4/2011, matt@create.ucsb.edu
*/



#include "allocore/al_Allocore.hpp"
#include "Gamma/Oscillator.h"

using namespace al;

gam::TableSine<> mysine(440);

void audioCB(AudioIOData& io){
	while(io()){
		float s = mysine()*0.2;
		io.out(0) = s;
		io.out(1) = s;
	}
}


int main(){
	AudioIO audioIO(256, 44100, audioCB, 0, 2, 1);

	// This call is necessary to synchronize the sample rate with all sound objects
	gam::Sync::master().spu(audioIO.fps());

	audioIO.start();

	printf("\nPress 'enter' to quit...\n"); getchar();
	return 0;
}
