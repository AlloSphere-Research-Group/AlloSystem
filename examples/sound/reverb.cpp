/*
Description:
This demonstrates how to apply a reverberation effect to the audio line input.

*/

#include "allocore/al_Allocore.hpp"
using namespace al;

Reverb<float> reverb;

void audioCB(AudioIOData& io){
	while(io()){
		float s = io.in(0);

		// compute two wet channels of reverberation
		float wet1, wet2;
		reverb(s, wet1, wet2);

		// output just the wet signals
		io.out(0) = wet1*0.2;
		io.out(1) = wet2*0.2;

//		io.out(0) = s + wet1*0.2;
//		io.out(1) = s + wet2*0.2;
	}
}


int main (int argc, char * argv[]){

	reverb.bandwidth(0.9);		// low-pass amount on input
	reverb.damping(0.5);		// high-frequency damping
	reverb.decay(0.8);			// tail decay factor
	reverb.diffusion(0.76, 0.666, 0.707, 0.571); // diffusion amounts

	AudioIO audioIO(256, 44100, audioCB, 0, 2, 1);
	audioIO.start();

	printf("\nPress 'enter' to quit...\n"); getchar();
	return 0;
}
