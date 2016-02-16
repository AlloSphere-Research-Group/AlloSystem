
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
#include "alloaudio/al_OutputMaster.hpp"
#include "alloaudio/al_BassManager.hpp"

using namespace al;

#define BLOCK_SIZE (512)

SpeakerLayout speakerLayout = AlloSphereSpeakerLayout();
AmbisonicsTunedSpatializer* tunedPanner = new AmbisonicsTunedSpatializer(speakerLayout);
AmbisonicsSpatializer* panner = new AmbisonicsSpatializer(speakerLayout, 3, 3);


AudioScene scene(BLOCK_SIZE);
Listener * listener;
AudioScene tunedScene(BLOCK_SIZE);
Listener * tunedListener;
SoundSource src;
Parameter x("X", "Position", 0);
Parameter y("Y", "Position", 5);
Parameter z("Z", "Position", 0);
bool useTuned = true;

void setPosition(float azimuth, float elevation, float distance)
{
  float new_x = distance * sin(2.0 * M_PI * (azimuth /360.0 )) * cos(2.0 * M_PI * (elevation /360.0 ));
  float new_y = distance * cos(2.0 * M_PI * (azimuth /360.0 )) * cos(2.0 * M_PI * (elevation /360.0 ));
	float new_z = distance * sin(2.0 * M_PI * (elevation /360.0 ));
	x.set(new_x);
	y.set(new_y);
	z.set(new_z);
	printf("x = %f y = %f z = %f\n", new_x, new_y, new_z);
}

ParameterServer paramServer("127.0.0.1", 9010);

void audioCB(AudioIOData& io){

	static unsigned int t = 0;
	int numFrames = io.framesPerBuffer();

	for(int i=0; i<numFrames; i++){

		double sec = (t / io.fps());

		src.pos(x.get(), y.get(), z.get());
				// src.pos(5, 5, 0);

		// Generate a test signal
		// float smp = sin(sec*440*2*M_PI)*0.5;	// tone
		float smp = rnd::uniformS()*0.1;		// noise

		float env = 1 - (sec * 2 - unsigned(sec * 2));
		smp *= env*env;

		// Write sample to the source
		src.writeSample(smp);

		++t;
	}
	// std::cout << "Y " << y.get() << std::endl;
	//render this scene buffer (renders as many frames as specified at initialization)
	if (useTuned) {
		tunedScene.render(io);
	} else {
		scene.render(io);
	}
}

int main (int argc, char * argv[]){

	listener = scene.createListener(panner);
	listener->compile();
	tunedListener = tunedScene.createListener(tunedPanner);
	tunedListener->compile();

	panner->print();
	tunedPanner->print();

	scene.addSource(src);
	tunedScene.addSource(src);

	paramServer.registerParameter(x);
	paramServer.registerParameter(y);
	paramServer.registerParameter(z);

	std::cout << x.getFullAddress() << std::endl;


	// 10) Create an audio IO for the audio scene
	//     Last 3 arguments are for user data, # out chans, and # in chans
	AudioIO audioIO(BLOCK_SIZE, 44100, audioCB, NULL, 60, 0);

	audioIO.device(AudioDevice(12));

	OutputMaster master(audioIO.channels(true), audioIO.fps(),
	                    "", -1, // No OSC input to control output master
		                "localhost", 26771
	                    );

	audioIO.append(master);
	// Start the IO!
	audioIO.start();

	bool run = true;
	float azimuth = 0;
	float elevation = 0;
	float distance = 5;
	while (run) {
		char c = getchar();
		switch(c) {
			case 'q':
				run = false;
				break;
			case 'w':
				elevation += 5;
				setPosition(azimuth, elevation, distance);
				printf("elevation = %f\n", elevation);
				break;
			case 'x':
				elevation -= 5;
				setPosition(azimuth, elevation, distance);
				printf("elevation = %f\n", elevation);
				break;
			case 'a':
				azimuth += 5;
				setPosition(azimuth, elevation, distance);
				printf("azimuth = %f\n", azimuth);
				break;
			case 'd':
				azimuth -= 5;
				setPosition(azimuth, elevation, distance);
				printf("azimuth = %f\n", azimuth);
				break;
			case 'e':
				distance += 0.5;
				setPosition(azimuth, elevation, distance);
				printf("distance = %f\n", distance);
				break;
			case 'c':
				distance -= 0.5;
				setPosition(azimuth, elevation, distance);
				printf("distance = %f\n", distance);
				break;
			case 'p':
				useTuned = !useTuned;
				printf("useTuned = %i\n", useTuned ? 1: 0);
				break;
			default:
				printf("char %d\n", (int)c);
				break;
		}

	}

	return 0;
}
