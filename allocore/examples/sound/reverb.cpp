/*
Allocore Example: Reverberation

Description:
This demonstrates how to apply a reverberation effect to the audio line input.

Author:
Lance Putnam, 4/25/2011, putnam.lance@gmail.com
*/

#include "allocore/io/al_AudioIO.hpp"
#include "allocore/sound/al_Reverb.hpp"
using namespace al;

int main(){
	
	Reverb<float> reverb;
	reverb.bandwidth(0.9);		// Low-pass amount on input, in [0,1]
	reverb.damping(0.5);		// High-frequency damping, in [0,1]
	reverb.decay(0.8);			// Tail decay factor, in [0,1]

	// Diffusion amounts
	// Values near 0.7 are recommended. Moving further away from 0.7 will lead
	// to more distinct echoes.
	reverb.diffusion(0.76, 0.666, 0.707, 0.571);

	AudioIO audioIO;
	audioIO.configure([&](AudioIOData& io){
		while(io()){
			float dry = io.in(0);

			// Compute two wet channels of reverberation
			float wet1, wet2;
			reverb(dry, wet1, wet2);

			// Output just the wet signals
			io.out(0) = wet1*0.2;
			io.out(1) = wet2*0.2;
		}
	}, 256, 44100, 2,1);

	audioIO.start();

	printf("\nPress 'enter' to quit...\n"); getchar();
}
