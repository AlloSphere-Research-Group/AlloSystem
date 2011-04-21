/*
Description:

Calls Gamma TableSine from AlloCore

Matt Wright

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


int main (int argc, char * argv[]){
	AudioIO audioIO(256, 44100, audioCB, 0, 2, 1);

	// This call is necessary to synchronize the sample rate with all sound objects
	gam::Sync::master().spu(audioIO.fps());

	audioIO.start();

	printf("\nPress 'enter' to quit...\n"); getchar();
	return 0;
}
