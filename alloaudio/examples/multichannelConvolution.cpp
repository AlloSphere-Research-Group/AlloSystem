/*
Based on Allocore Example: Audio To Graphics by Lance Putnam

*/

#include <iostream>

#include "allocore/io/al_App.hpp"
#include "alloaudio/al_OutputMaster.hpp"
#include "alloaudio/al_Convolver.hpp"
#include "Gamma/SoundFile.h"
#include "string.h"

#define BLOCK_SIZE 64


using namespace std;
using namespace al;
using namespace gam;

class MyApp : public App{
public:
    double phase;
    OutputMaster outMaster;
    Convolver conv;

    MyApp(int num_chnls, double sampleRate,
          const char * address = "", int inport = 19375,
          const char * sendAddress = "localhost", int sendPort = -1)
        : outMaster(num_chnls, sampleRate, address, inport, sendAddress, sendPort)
    {
        nav().pos(0,0,4);
        initWindow();
        // Load IRs
        const char * path = "r1_ortf.wav";
        SoundFile sf(path);
        sf.openRead();
        int numIRChannels = sf.channels(), numFrames = sf.frames();
        double fs = sf.frameRate();
        if(sampleRate != fs){
            cout << "Warning: Application's sampling rate differs from that of impulse response file." << endl;
        }
        float deinterleavedChannels[numIRChannels * numFrames];
        if(sf.readAllD(deinterleavedChannels) != numFrames){
            cout << "Could not read impulse response file" << endl;
        }
        int numActiveChannels;
        vector<int> disabledChannels;
        if(numIRChannels > num_chnls){
            numActiveChannels = num_chnls;
        }
        else{
            numActiveChannels = numIRChannels;
            if(numActiveChannels != num_chnls){//more outputs than IR channels
                for(int i = numActiveChannels; i < num_chnls; ++i){
                    disabledChannels.push_back(i);
                }
            }
        }
		initAudio(fs, BLOCK_SIZE, numActiveChannels, numActiveChannels);//convolver also supports a mode with mono input
        
        // Setup convolver
        vector<float *> IRchannels;
        for(int i = 0; i< num_chnls; ++i){
            float channelData[numFrames];
            memcpy(channelData, &deinterleavedChannels[i * numFrames], sizeof(float) * numFrames);
            IRchannels.push_back(channelData);
        }
        unsigned int options = 1;//uses FFTW_MEASURE
        //many to many mode
        conv.configure(this->audioIO(), IRchannels, numFrames, -1, false, disabledChannels, BLOCK_SIZE, options);
    }

    // Audio callback
    void onSound(AudioIOData& io){

        conv.onAudioCB(io);
		outMaster.onAudioCB(io);
    }


    void onAnimate(double dt){

    }

    void onDraw(Graphics& g, const Viewpoint& v){

    }
};


int main(){
	int num_chnls = 2;
	double sampleRate = 44100;
	const char * address = "localhost";
	int inport = 3002;
	const char * sendAddress = "localhost";
	int sendPort = 3003;
	cout << "Listening to \"" << address << "\" on port " << inport << endl;
	cout << "Sending to \"" << sendAddress << "\" on port " << sendPort << endl;
	MyApp(num_chnls, sampleRate, address, inport, sendAddress, sendPort).start();
}
