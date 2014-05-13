/*

*/

#include "allocore/al_Allocore.hpp"
#include "allocore/io/al_AudioIO.hpp"
#include <iostream>
#include <cstdlib>

using namespace al;
using namespace std;

#define AUDIO_BLOCK_SIZE 512

typedef struct {
    int count;
    int numSamps;
    int curOutput;
    float gain;
} userdata_t;


void audioCB(AudioIOData& io)
{
    userdata_t * ud = (userdata_t *) io.user();
	int numFrames = io.framesPerBuffer();

    for (int j = 0; j < numFrames; j++) {
        float noise = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        io.out(ud->curOutput, j) = ud->gain * noise;
        ud->count++;
        if (ud->count == ud->numSamps) {
            ud->count = 0;
            ud->curOutput++;
            ud->curOutput = ud->curOutput%io.channelsOut();
            std::cout << "Channel " << ud->curOutput << std::endl;
        }
    }
}

int main (int argc, char * argv[])
{
    const int numSpeakers = 2;
    double sr = 44100;
    double durSecs = 1; // duration of each burst

    userdata_t ud;
    ud.gain = 0.01;
    ud.count = 0;
    ud.numSamps = (int)(sr *durSecs);
    ud.curOutput = 0;

    AudioIO audioIO(AUDIO_BLOCK_SIZE, sr, audioCB, &ud, numSpeakers, 0);
	audioIO.start();

    MainLoop::start();

	return 0;
}
