/*
Allocore Example: Reverberation

Description:
This demonstrates how to apply a reverberation effect to the audio line input.

Author:
Lance Putnam, 4/25/2011, putnam.lance@gmail.com
*/

#include "allocore/al_Allocore.hpp"
using namespace al;


void audioCB(AudioIOData& io){
	while(io()){
	
	}
}


int main (int argc, char * argv[]){

	AudioDevice::printAll();

	AudioIO audioIO(256, 44100, audioCB, 0, 2, 1);
	audioIO.start();
	
	printf("\nPress 'enter' to quit...\n"); getchar();
	return 0;
}
