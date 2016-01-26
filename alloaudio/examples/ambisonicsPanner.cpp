
/*
 *  ambisonicsScene.cpp
 *
 *  Created by Ryan McGee on 4/20/11.
 *  Allocore Code Sprint 20th of April 2011
 *  Modified by Andrés Cabrera Jan 2016
 *
 */

#include "allocore/al_Allocore.hpp"
#include "allocore/system/al_Parameter.hpp"
#include "alloaudio/al_AmbiTunedDecoder.hpp"
#include "alloutil/al_AlloSphereSpeakerLayout.hpp"

using namespace al;

#define BLOCK_SIZE (512)

SpeakerLayout speakerLayout = AlloSphereSpeakerLayout();
AmbisonicsTunedSpatializer* panner = new AmbisonicsTunedSpatializer();

AudioScene scene(BLOCK_SIZE);
Listener * listener;
SoundSource src;
Parameter x("X", "Position", 1);
Parameter y("Y", "Position", 1);
Parameter z("Z", "Position", 1);

void audioCB(AudioIOData& io){

	static unsigned int t = 0;
	int numFrames = io.framesPerBuffer();

	for(int i=0; i<numFrames; i++){

		double sec = (t / io.fps());

		src.pos(x.get(), y.get(), z.get());

		// Generate a test signal
		float smp = sin(sec*440*2*M_PI)*0.5;	// tone
//		float smp = rnd::uniformS()*0.1;		// noise

		float env = 1 - (sec - unsigned(sec));
		smp *= env*env;

		// Write sample to the source
		src.writeSample(smp);

		++t;
	}

	//render this scene buffer (renders as many frames as specified at initialization)
	scene.render(io);
}

int main (int argc, char * argv[]){

	// 7) Initialize the listener(s) with their individual speaker layout and panner
	listener = scene.createListener(panner);

	// 8) Add the sound source to the scene
	scene.addSource(src);

    // Optionally, disable per sample processing to save CPU. Recommended to disable Doppler in this case as well.
	//scene.usePerSampleProcessing(false);
    //src.dopplerType(DOPPLER_NONE);

	// 9) update the listener's speaker layout and panner
	//    call this to dynamically change a listener's speaker layout and panner
	// maybe rename this to update() ?
	listener->compile();

	// Print out relevant panner info (ex. number of triplets found for VBAP)
	panner->print();

	// 10) Create an audio IO for the audio scene
	//     Last 3 arguments are for user data, # out chans, and # in chans
	AudioIO audioIO(BLOCK_SIZE, 44100, audioCB, NULL, 2, 0);

	// Start the IO!
	audioIO.start();

	// run until the user hits any key
	getchar();

	return 0;
}


