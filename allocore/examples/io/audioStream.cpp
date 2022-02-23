/*
Allocore Example: Audio Stream

Description:
This example demonstrates how to start an audio stream and process input and
output.

Author:
Lance Putnam, 2022
*/


#include "allocore/io/al_AudioIO.hpp"
using namespace al;

int main(){

	AudioIO io;

	// Print information about found devices
	printf("Audio devices found:\n");
	io.printDevices();
	printf("\n");


	// Streaming setup consists of three steps:
	// 1. Set devices
	// 2. Configure streams
	// 3. Start streams

	//---- Step 1: Set input and output devices

	// Find output device using predicate:
	auto devOut = io.findDevice([](AudioIO::Device d){
		//return d.channelsOut >= 2;
		return d.hasOutput();
		//return d.hasOutput() && d.nameMatches("Speakers");
	});
	io.deviceOut(devOut);


	//---- Step 2: Configure streams
	// Set stream parameters
	io.configure(
		512,		// frames/buffer (block size)
		44100,		// frame rate
		2,			// output channels
		1			// input channels
	);
	// Set callback
	io.callback([](const AudioIOData& io){
		// Loop through the frames in the block
		while(io()){
			float s = io.in(0); // get input (line-in or mic)
			// process samples here...
			io.out(0) = s; // set left channel
			io.out(1) = s; // set right channel
		}
	});


	//---- Step 3: Start the audio stream; this launches a new thread
	io.start();


	/* Simplified one-step version that uses defaults:
	io.configure([](const AudioIOData& io){
		// Loop through the frames in the block
		while(io()){
			float s = io.in(0); // get input (line-in or mic)
			// process samples here...
			io.out(0) = s; // set left channel
			io.out(1) = s; // set right channel
		}
	}).start();
	*/


	// Print some information about the i/o streams
	printf("Audio stream info:\n");
	io.print();

	printf("\nPress 'enter' to quit...\n"); fflush(stdout); getchar();
	return 0;
}
