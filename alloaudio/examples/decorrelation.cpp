/*
  Decorrelation example
  by: Andres Cabrera
*/

#include <iostream>

#include "allocore/io/al_App.hpp"
#include "alloaudio/al_OutputMaster.hpp"
#include "alloaudio/al_Decorrelation.hpp"

#include "Gamma/Noise.h"
#include "Gamma/Envelope.h"

/* This example will generate an exponentially decaying noise burst and
 * then produce 8 decorrelated copies of it using random phase all-pass
 * filters. It uses the Kendall method to produce the IRs, that consists
 * of generating a random phase spectrum and then doing an inverse FFT
 * to obtain the IR for that random phase filter.
 * In this example, every 3 bursts of noise, a new set of decorrelation IRs
 * is produced changin the maxjump parameter. As the maxjump parameter
 * decreases, the correlation between the IRs increases as there will be less
 * variation in the randomness of the phase, and the sound will become
 * "narrower".
*/

using namespace al;

class MyApp : public App{
public:
	float mMaxjump;
	int mCounter;
	int mCounterTarget;

	Decorrelation decorrelation;

	gam::NoisePink<> noise;
	gam::Decay<> env;

	// The decorrelation object is initialized in the constructor
	// The IR size is set to 1024
	// The input channel is set to -1 to indicate parallel or "many to many" mode
	// The number of channels is set to 8
	// And the last parameter "inputsAreBuses" is set to true to indicate
	// that inputs are buses instead of hardware input channels
	MyApp() :
		decorrelation(1024, -1, 8, true),
		mMaxjump(M_PI), env(48000)

	{
		nav().pos(0,0,4);
//		initWindow(); // Don't call init window if you don't need to display a window
		audioIO().channelsBus(8); // Make 8 buses
		// You can append the decorrelation object to the AudioIO object
		// This will add the audio process callback to the AudioIO processing
		// after the user defined callback below
		audioIO().append(decorrelation);
		initAudio(48000, 512, 8, 8);
		mCounterTarget = 3 * 48000/ 512;
		mCounter = mCounterTarget; // Force a generation of IRs on first pass
		decorrelation.configure(audioIO(), 1000, mMaxjump);
	}

	// Audio callback
	void onSound(AudioIOData& io){
		if (++mCounter >= mCounterTarget) { // Check if it's time to generate new IRs.
			decorrelation.configure(audioIO(), 1000, mMaxjump);
			std::cout << "Max jump set to: " << mMaxjump << std::endl;
			mMaxjump *= 0.5;
			if (mMaxjump < 0.05) { mMaxjump = M_PI; }
			mCounter = 0;
		}
		int frame = 0;
		while(io()){
			float sample = noise() * env(); // Generate noise bursts
			if (env.done()) { env.reset(); }
			for (int i = 0 ; i < 8; i++) { // Copy the noise burst to the 8 buses
				io.bus(i, frame) = sample;
			}
			frame++;
		}
		// Decorrelation is performed without the need to add anything here
		// as the decorrelation object was "appended" to the AudioIO object.
	}
};


int main(){
	MyApp().start();
}
