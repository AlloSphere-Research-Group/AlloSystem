/*
Based on Allocore Example: Audio To Graphics by Lance Putnam

*/

#include <iostream>

#include "allocore/io/al_App.hpp"
#include "alloaudio/al_OutputMaster.hpp"
#include "alloaudio/al_Convolver.hpp"
#include "Gamma/Noise.h"
#include "Gamma/Envelope.h"

#define BLOCK_SIZE 64

using namespace std;
using namespace al;
using namespace gam;

class MyApp : public App{
public:
	double phase;
	Convolver conv;
	vector<float *> IRchannels;

	MyApp(int num_chnls, double fs)
	{
		nav().pos(0,0,4);
		initWindow();
		// Make IR
		int irdur = 1.0; // duration of the IR in seconds
		int numFrames = fs * irdur;
		int numIRChannels = 2; //Number of channels in the IR
		gam::NoisePink<> noise;
		gam::Decay<> env(numFrames);

		// Make two IRs from pink noise and a exponential decay.
		// You could also load the data from a soundfile
		for (int chan = 0; chan < numIRChannels; chan++) {
			float *ir = new  float[numFrames];
			for (int frame = 0; frame < numFrames; frame++) {
				ir[frame] = noise() * env() * 0.1;
			}
			env.reset();
			IRchannels.push_back(ir);
		}
		int numActiveChannels;
		vector<int> disabledChannels;
		numActiveChannels = numIRChannels;
		if(numActiveChannels != num_chnls){//more outputs than IR channels
			for(int i = numActiveChannels; i < num_chnls; ++i){
				disabledChannels.push_back(i);
				cout << "Audio channel " << i << " disabled." << endl;
			}
		}
		initAudio(fs, BLOCK_SIZE, numActiveChannels, numActiveChannels);

		// Setup convolver
		unsigned int options = 1;//uses FFTW_MEASURE
		conv.configure(this->audioIO(), IRchannels, numFrames, -1, false, disabledChannels, BLOCK_SIZE, options);
		this->audioIO().append(conv); // By appending, the convolution will be perfomed after any other processes
	}
	~MyApp() {
		// Cleanup memory allocation
		for (unsigned int i = 0; i < IRchannels.size(); i++) {
			delete IRchannels[i];
		}
	}

//	// Audio callback
	void onSound(AudioIOData& io){
		// Nothing needed here as convolution is performed as the last process since it has been
		// "appended" to AudioIO
	}

	void onAnimate(double dt){

	}

	void onDraw(Graphics& g, const Viewpoint& v){

	}

};


int main(){
	int num_chnls = 2;
	double sampleRate = 44100;
	MyApp(num_chnls, sampleRate).start();
}
