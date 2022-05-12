/*
 *  exSpatialAudio.cpp
 *  How to set up a simple spatial audio scene in allocore
 *  Or, how to build a spatializer in 9 steps
 *
 *  Created by Ryan McGee on 4/20/11.
 *  Allocore Code Sprint 20th of April 2011
 *
 */

#include "allocore/io/al_AudioIO.hpp"
#include "allocore/sound/al_Vbap.hpp"
#include "allocore/sound/al_Dbap.hpp"
#include "allocore/sound/al_Ambisonics.hpp"
using namespace al;


int main(){

	static const int BLOCK_SIZE = 256;

	// 1) Create a speaker layout
	SpeakerLayout speakerLayout = HeadsetSpeakerLayout();

	// 2) Create a panner: DBAP, VBAP, Ambisonics, or Stereo
	//Dbap* panner = new Dbap(speakerLayout);
	//Vbap* panner = new Vbap(speakerLayout);
	AmbisonicsSpatializer* panner = new AmbisonicsSpatializer(speakerLayout, 2, 1);  // dimension and order
	//StereoPanner *panner = new StereoPanner(speakerLayout);

	// 3) Create an audio scene with single argument for frames per buffer
	AudioScene scene(BLOCK_SIZE);

	// 4) Create listener(s)
	Listener * listener;

	// 5) Create a Sound Source
	SoundSource src;

	// 6) Initialize the listener(s) with their individual speaker layout and panner
	listener = scene.createListener(panner);

	// 7) Add the sound source to the scene
	scene.addSource(src);

    // Optionally, disable per sample processing to save CPU. Recommended to disable Doppler in this case as well.
	//scene.usePerSampleProcessing(false);
    //src.dopplerType(DOPPLER_NONE);

	// 8) update the listener's speaker layout and panner
	//    call this to dynamically change a listener's speaker layout and panner
	// maybe rename this to update() ?
	listener->compile();

	// Print out relevant panner info (ex. number of triplets found for VBAP)
	panner->print();

	// 9) Create an audio IO for the audio scene
	//     Last 2 arguments are for  # out chans, and # in chans
	AudioIO audioIO;

	audioIO.configure([&](AudioIOData& io){
		static unsigned int t = 0;

		int numFrames = io.framesPerBuffer();

		for(int i=0; i<numFrames; i++){

			double sec = (t / io.fps());

			// Create an oscillating trajectory
			float x = sin(sec*0.5*2*M_PI);
			src.pos(x, 0, 0);

			// Generate a test signal
			float smp = sin(sec*440*2*M_PI)*0.5;	// tone
			//float smp = rnd::uniformS()*0.1;		// noise

			float env = 1 - (sec - unsigned(sec));
			smp *= env*env;

			// Write sample to the source
			src.writeSample(smp);

			++t;
		}

		//render this scene buffer (renders as many frames as specified at initialization)
		scene.render(io);
	}, BLOCK_SIZE, 44100, 2,0);

	// Start the IO!
	audioIO.start();

	// run until the user hits any key
	getchar();

	return 0;
}

