
#include <iostream>

#include "allocore/system/al_Parameter.hpp"
#include "allocore/system/al_PeriodicThread.hpp"
#include "allocore/io/al_AudioIO.hpp"
#include "allocore/system/al_MainLoop.hpp"

#include "Gamma/Oscillator.h"


using namespace al;
using namespace std;

#define AUDIO_BLOCK_SIZE 512

Parameter freq("Frequency", "", 440.0);
Parameter amp("Amplitude", "", 0.1);
gam::Sine<> oscil;

double interval = 1.0; // time between random parameter changes

float ampCallback(float ampdb, void *data) {
	float amp = powf(10.0, ampdb/20.0);
//	cout << " --- Amp ---- " << amp  << " ----" << endl;
	return amp;
}

class MyThreadFunction : public ThreadFunction
{
	virtual void operator ()() {
		osc::Send sender(9010, "127.0.0.1");
		while (true) {
			float newFreq = 440.0 * (1 + rand() /(float) RAND_MAX);
			float newAmpDb = -40.0 * rand() /(float) RAND_MAX;
			freq.set(newFreq);
			amp.set(newAmpDb);
			cout << "Setting through C++: Frequency " << newFreq << " Amplitude " << newAmpDb << endl;
			al_sleep_nsec(interval * 1000000000.0);
			// Now do it through OSC
			newFreq = 440.0 * (1 + rand() /(float) RAND_MAX);
			newAmpDb = -40.0 * rand() /(float) RAND_MAX;
			sender.send("/Frequency", newFreq);
			sender.send("/Amplitude", newAmpDb);
			cout << "Setting through OSC: Frequency " << newFreq << " Amplitude " << newAmpDb << endl;
			al_sleep_nsec(interval * 1000000000.0);
		}
	}
};

void audioCB(AudioIOData& io)
{
	float f = freq.get();
	float a = amp.get();
	
	oscil.freq(f);
	
	while (io()) {
		float sample = oscil();
		io.out(0) = a * sample;
		io.out(1) = a * sample;
	}
}

int main (int argc, char * argv[])
{
	double sr = 44100;
	
	MyThreadFunction func;
	Thread th(func);
	
	amp.setProcessingCallback(ampCallback, nullptr);

	ParameterServer paramServer;
	paramServer.registerParameter(freq);
	paramServer.registerParameter(amp);
	
	AudioIO audioIO(AUDIO_BLOCK_SIZE, sr, audioCB, 0, 2, 0);
	gam::Domain::master().spu(audioIO.framesPerSecond());
	audioIO.start();
	
	MainLoop::start();
	th.join();
	
	return 0;
}
