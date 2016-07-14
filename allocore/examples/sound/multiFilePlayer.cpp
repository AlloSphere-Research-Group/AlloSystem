/*
What is our "Hello world!" app?

An agent orbits around the origin emitting the audio line input. The camera
view can be switched between a freely navigable keyboard/mouse controlled mode
and a sphere follow mode.

Requirements:
2 channels of spatial sound
2 windows, one front view, one back view
stereographic rendering
*/

#include "allocore/al_Allocore.hpp"
#include <Gamma/SoundFile.h>
#include <iostream>
#include <vector>

using namespace al;
using namespace std;

#define AUDIO_BLOCK_SIZE 512

typedef struct {
	float *values;
	int counter;
	int numblocks;
} meters_t;

typedef struct {
    gam::SoundFile sfs[4];
    float read_buffer[AUDIO_BLOCK_SIZE];
    vector<int> outputMap;
    meters_t meters;
    float gain;
    int done;
} userdata_t;


void audioCB(AudioIOData& io)
{
    userdata_t * ud = (userdata_t *) io.user();
	int numFrames = io.framesPerBuffer();

	int framesRead;
    for (int i = 0; i < 4; i++) {
        framesRead = ud->sfs[i].read(ud->read_buffer, numFrames);
        for (int j = 0; j < framesRead; j++) {
            io.out(ud->outputMap[i], j) += (ud->read_buffer[j] * ud->gain);
        }
    }
    float *curvalue = ud->meters.values;
    for (int i = 0; i < io.channelsOut(); i++) {
        for (int j = 0; j < numFrames; j++) {
    		if (*curvalue < io.out(i, j)) {
    			*curvalue = io.out(i, j);
    		}
    	}
    	curvalue++;
    }
    if (++(ud->meters.counter) == ud->meters.numblocks) {
    	for (int i = 0; i < io.channelsOut(); i++) {
    		int db = (int) (20.0*log10(ud->meters.values[i]));

    		if (db < -120) {
    			std::cout << "-- ";
    		} else {
    			std::cout << db << " ";
    		}
    	}
    	std::cout << std::endl;
    	ud->meters.counter = 0;
    }
	if (framesRead == 0) {
		ud->done = 1;
	}
}

//#define STEREO

int main (int argc, char * argv[])
{
    // Set speaker layout

#ifdef STEREO
    const int numSpeakers = 2;
    Speaker speakers[] = {
        Speaker(0,  45,0),
        Speaker(1, -45,0),
    };
#else
    const int numSpeakers = 54;
    Speaker speakers[] = {
        Speaker(1-1, 77.660913, 41.000000),
        Speaker(2-1, 45.088015, 41.000000),
        Speaker(3-1, 14.797289, 41.000000),
        Speaker(4-1, -14.797289, 41.000000),
        Speaker(5-1, -45.088015, 41.000000),
        Speaker(6-1, -77.660913, 41.000000),
        Speaker(7-1, -102.339087, 41.000000),
        Speaker(8-1, -134.911985, 41.000000),
        Speaker(9-1, -165.202711, 41.000000),
        Speaker(10-1, 165.202711, 41.000000),
        Speaker(11-1, 134.911985, 41.000000),
        Speaker(12-1, 102.339087, 41.000000),
        Speaker(17-1, 77.660913, 0.000000),
        Speaker(18-1, 65.647587, 0.000000),
        Speaker(19-1, 54.081600, 0.000000),
        Speaker(20-1, 42.869831, 0.000000),
        Speaker(21-1, 31.928167, 0.000000),
        Speaker(22-1, 21.181024, 0.000000),
        Speaker(23-1, 10.559657, 0.000000),
        Speaker(24-1, 0.000000, 0.000000),
        Speaker(25-1, -10.559657, 0.000000),
        Speaker(26-1, -21.181024, 0.000000),
        Speaker(27-1, -31.928167, 0.000000),
        Speaker(28-1, -42.869831, 0.000000),
        Speaker(29-1, -54.081600, 0.000000),
        Speaker(30-1, -65.647587, 0.000000),
        Speaker(31-1, -77.660913, 0.000000),
        Speaker(32-1, -102.339087, 0.000000),
        Speaker(33-1, -114.352413, 0.000000),
        Speaker(34-1, -125.918400, 0.000000),
        Speaker(35-1, -137.130169, 0.000000),
        Speaker(36-1, -148.071833, 0.000000),
        Speaker(37-1, -158.818976, 0.000000),
        Speaker(38-1, -169.440343, 0.000000),
        Speaker(39-1, -180.000000, 0.000000),
        Speaker(40-1, 169.440343, 0.000000),
        Speaker(41-1, 158.818976, 0.000000),
        Speaker(42-1, 148.071833, 0.000000),
        Speaker(43-1, 137.130169, 0.000000),
        Speaker(44-1, 125.918400, 0.000000),
        Speaker(45-1, 114.352413, 0.000000),
        Speaker(46-1, 102.339087, 0.000000),
        Speaker(49-1, 77.660913, -32.500000),
        Speaker(50-1, 45.088015, -32.500000),
        Speaker(51-1, 14.797289, -32.500000),
        Speaker(52-1, -14.797289, -32.500000),
        Speaker(53-1, -45.088015, -32.500000),
        Speaker(54-1, -77.660913, -32.500000),
        Speaker(55-1, -102.339087, -32.500000),
        Speaker(56-1, -134.911985, -32.500000),
        Speaker(57-1, -165.202711, -32.500000),
        Speaker(58-1, 165.202711, -32.500000),
        Speaker(59-1, 134.911985, -32.500000),
        Speaker(60-1, 102.339087, -32.500000),
    };
#endif

    SpeakerLayout speakerLayout;
    for (int i = 0; i< numSpeakers; i++) {
        speakerLayout.addSpeaker(speakers[i]);
    }

    float sr = 44100;

    userdata_t ud;
    ud.gain = 0.5;
    ud.done = 0;
    ud.meters.counter = 0;
    ud.meters.numblocks = 1.0 * sr/AUDIO_BLOCK_SIZE;
    ud.meters.values = (float *) calloc(60, sizeof(float));

	std::string path = "/Users/create/code/spatial_andres/";
//	std::string path = "/home/andres/Music/";

    std::vector<std::string> filenames;
    filenames.push_back(path + "Chowning/Turenas-MM/Turenas-MM-LF.aif");
    filenames.push_back(path + "Chowning/Turenas-MM/Turenas-MM-RF.aif");
    filenames.push_back(path + "Chowning/Turenas-MM/Turenas-MM-LR.aif");
    filenames.push_back(path + "Chowning/Turenas-MM/Turenas-MM-RR.aif");
//    filenames.push_back(path + "Chowning/Stria/Stria-FL.aiff");
//    filenames.push_back(path + "Chowning/Stria/Stria-FR.aiff");
//    filenames.push_back(path + "Chowning/Stria/Stria-RL.aiff");
//    filenames.push_back(path + "Chowning/Stria/Stria-RR.aiff");

    ud.outputMap.resize(filenames.size());
//  ud.outputMap[0] = 2 -1;
//    ud.outputMap[1] = 53 -1;
//    ud.outputMap[2] = 59 -1;
//    ud.outputMap[3] = 8 -1;
//    ud.outputMap[0] = 21 -1;
//    ud.outputMap[1] = 27 -1;
//    ud.outputMap[2] = 42 -1;
//    ud.outputMap[3] = 36 -1;
    ud.outputMap[0] = 2 -1;
    ud.outputMap[1] = 53 -1;
    ud.outputMap[2] = 59 -1;
    ud.outputMap[3] = 8 -1;

    for (int i = 0; i < filenames.size(); i++) {

        ud.sfs[i].path(filenames[i]);
        if (!ud.sfs[i].openRead()) {
            std::cout << " Can't open file: " << filenames[i] << std::endl;
            return -1;
        } else {
            std::cout << "Playing file: " << filenames[i] << std::endl;
        }
        // TODO check sampling rate;
    }

//    Dbap *dbap;
//    dbap = new Dbap();

    AudioIO audioIO(AUDIO_BLOCK_SIZE, sr, audioCB, &ud, 60, 0);
	audioIO.start();

	while (!ud.done) {
		al_sleep(0);
	}
	return 0;
}
