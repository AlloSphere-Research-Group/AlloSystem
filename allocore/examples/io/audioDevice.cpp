/*
Allocore Example: Audio Device

Description:
The example demonstrates how to start an audio stream and process input and
output.

Author:
Lance Putnam, 4/25/2011
*/


#include "allocore/io/al_AudioIO.hpp"
using namespace al;

// A user defined class that can be accessed from the audio callback
struct UserData{
	float ampL, ampR;
};


// Our main audio callback
void audioCB(AudioIOData& io){

	UserData& user = *(UserData *)io.user();
	float ampL = user.ampL;
	float ampR = user.ampR;

	// loop through the number of samples in the block
	while(io()){

		float s = io.in(0);		// get the line-in or microphone sample

		io.out(0) = s * ampL;	// set left and right output channel samples
		io.out(1) = s * ampR;
	}
}


int main(){

	// Set parameters of audio stream
	int blockSize = 64;				// how many samples per block?
	float sampleRate = 44100;		// sampling rate (samples/second)
	int outputChannels = 2;			// how many output channels to open
	int inputChannels = 1;			// how many input channels to open
	UserData user = {-0.5, 0.5};	// external data to be passed into callback

	printf("Audio devices found:\n");
	AudioDevice::printAll();
	printf("\n");

	// Create an audio i/o object using default input and output devices
	AudioIO io(blockSize, sampleRate, audioCB, &user, outputChannels, inputChannels);

	// Start the audio stream; this launches a new thread
	io.start();

	// Print some information about the i/o streams
	printf("Audio stream info:\n");
	io.print();

	printf("\nPress 'enter' to quit...\n"); getchar();
	return 0;
}
