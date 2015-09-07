/*
Based on Allocore Example: Audio To Graphics by Lance Putnam

*/

#include <iostream>
#include <cstring>

#include "allocore/io/al_App.hpp"
#include "alloaudio/al_OutputMaster.hpp"
#include "alloaudio/al_Convolver.hpp"
#include "Gamma/SoundFile.h"
#include "Gamma/Noise.h"
#include "Gamma/Envelope.h"


#define BLOCK_SIZE 64


using namespace std;
using namespace al;


bool once = true;

class MyApp : public App{
public:
	double phase;
    Convolver conv;
	vector<float *> IRchannels;

	MyApp(int num_chnls, double fs)
    {
        nav().pos(0,0,4);
        initWindow();
        // Load IRs
		const char * path = "alloaudio/examples/Batcave1.wav";
        gam::SoundFile sf(path);

		float *deinterleavedChannels;
		int numActiveChannels;
		int numFrames;
		vector<int> disabledChannels;
        if(!sf.openRead()){
			numFrames = 44100;
			cout << "Error: Could not read impulse response file. Creating IRs from pink noise." << endl;
			numActiveChannels = num_chnls;
			deinterleavedChannels = new float[numActiveChannels * numFrames];
			gam::NoisePink<> noise;
			gam::Decay<> env(numFrames);
			for (int i = 0; i < num_chnls; i++) {
				for (int samp = 0; samp < numFrames; samp++) {
					deinterleavedChannels[i*numFrames + samp] = noise() * env() * 0.1;
				}
				env.reset();
			}
		} else {
			int numIRChannels = sf.channels();
			numFrames = sf.frames();
			cout << "Reading IR file '" << path << "'. Channels: " << numIRChannels << ". Frames: " << numFrames << "." << endl;
			double fs = sf.frameRate();
			if(fs != fs){
				cout << "Error: Application's sampling rate (" << fs << ") differs from that of the impulse response file (" << fs << ")." << endl;
			}
			deinterleavedChannels = new float[numIRChannels * numFrames];
			if(sf.readAllD(deinterleavedChannels) != numFrames){
				cout << "Error: failed to read expected number of frames." << endl;
			}
			if(numIRChannels > num_chnls){
				numActiveChannels = num_chnls;
			}
			else{
				numActiveChannels = numIRChannels;
				if(numActiveChannels != num_chnls){//more outputs than IR channels
					for(int i = numActiveChannels; i < num_chnls; ++i){
						disabledChannels.push_back(i);
						cout << "Audio channel " << i << " disabled." << endl;
					}
				}
			}
		}
		initAudio(fs, BLOCK_SIZE, numActiveChannels, numActiveChannels);//convolver also supports a mode with mono input
        
        // Setup convolver
        vector<float *> IRchannels;
		for(int i = 0; i< numActiveChannels; ++i){
			float *channelData = new float[numFrames];
            memcpy(channelData, &deinterleavedChannels[i * numFrames], sizeof(float) * numFrames);
            IRchannels.push_back(channelData);
        }
        unsigned int options = 1;//uses FFTW_MEASURE
        //many to many mode
        conv.configure(this->audioIO(), IRchannels, numFrames, -1, false, disabledChannels, BLOCK_SIZE, options);

		for(int i = 0; i< numActiveChannels; ++i){
			delete IRchannels[i];
		}
		delete deinterleavedChannels;
		audioIO().append(conv); // By appending there is no need to do anything in the audio callback
    }

    // Audio callback
    void onSound(AudioIOData& io){
		// If you don't use append above you can perform the convolution with:
		// conv.onAudioCB(io);
    }


    void onDraw(Graphics& g, const Viewpoint& v){

    }
};


int main(){
	int num_chnls = 2;
	double sampleRate = 44100;
	MyApp(num_chnls, sampleRate).start();
}
