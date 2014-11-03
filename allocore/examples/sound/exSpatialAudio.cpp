/*
 *  exSpatialAudio.cpp
 *  How to set up a simple spatial audio scene in allocore
 *  Or, how to build a spatializer in 10 steps
 *
 *  Created by Ryan McGee on 4/20/11.
 *  Allocore Code Sprint 20th of April 2011
 *
 */

#include "allocore/al_Allocore.hpp"
#include "allocore/sound/al_Vbap.hpp"
#include "allocore/sound/al_Dbap.hpp"
#include "allocore/sound/al_Ambisonics.hpp"

using namespace al;

#define NUM_FRAMES (256)

// 1) Create a panner: DBAP, VBAP, or HOA
Dbap* panner = new Dbap();
//Vbap* panner = new Vbap();
//AmbisonicsSpatializer* panner = new AmbisonicsSpatializer(2, 1);  // dimension and order

// 2) Create an audio scene with single arguement for frames per buffer
AudioScene scene(NUM_FRAMES);

// 3) Create listener(s)
Listener * listener;

// 4) Create Speaker Layout
SpeakerLayout speakerLayout = HeadsetSpeakerLayout();

// 5) Create a Sound Source
SoundSource src;

// 6) Create an audio callback function for the source and scene
void audioCB(AudioIOData& io){
	
	int numFrames = io.framesPerBuffer();
	
	for(int i=0; i<numFrames; i++){
		//Write each sample to the source
		//(this just writes random noise for testing)
		src.writeSample(rnd::uniform(1.,-1.));
	}
	
	//render this scene buffer (renders as many frames as specified at initialization)
	scene.render(io);
	
}

int main (int argc, char * argv[]){
	
	// 7) Initialize the listener(s) with their individual speaker layout and panner
	listener = scene.createListener(speakerLayout,panner);
	
	// 8) Add the sound source to the scene
	scene.addSource(src);
	
	// 9) update the listener's speaker layout and panner
	//    call this to dynamically change a listener's speaker layout and panner
	// maybe rename this to update() ?
	listener->compile();
	
	// Output's relevant panner info (ex. number of triplets found for VBAP)
	panner->dump();//Check VBAP
	
	// 10) Create an audio IO for the audio scene
	//     Last 3 arguemnts are for user data, #out chans, and # in chans
	AudioIO audioIO(NUM_FRAMES, 44100, audioCB, NULL, 2, 0);
	
	// Start the IO!
	audioIO.start();
	
	// run until the user hits any key
	getchar();

	return 0;
}

