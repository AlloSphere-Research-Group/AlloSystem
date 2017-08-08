
#include <iostream>

#include "allocore/ui/al_Parameter.hpp"
#include "allocore/system/al_PeriodicThread.hpp"
#include "allocore/io/al_AudioIO.hpp"
#include "allocore/system/al_MainLoop.hpp"

#include "Gamma/Oscillator.h"


using namespace al;
using namespace std;

#define AUDIO_BLOCK_SIZE 512

// Parameter declaration
// Access to these parameters via the get and set functions is competely
// thread safe
Parameter freq("Frequency", "", 440.0);
Parameter amp("Amplitude", "", 0.1);
gam::Sine<> oscil;

osc::Send sender(9010, "127.0.0.1");
double interval = 1.0; // time between random parameter changes

// This function running on its own thread will periodically change
// the parameters
class MyThreadFunction : public ThreadFunction
{
public:
	bool running = true;

	virtual void operator ()() override {
		while (running) {
			// Random values
			float newFreq = 440.0 * (1 + rand() /(float) RAND_MAX);
			float newAmpDb = -40.0 * rand() /(float) RAND_MAX;
			// The parameters can be set through C++ assignment:
			freq = newFreq;
			// or functions
			amp.set(newAmpDb);
			cout << "Setting through C++: Frequency " << newFreq << " Amplitude " << newAmpDb << endl;
			al_sleep_nsec(interval * 1000000000.0);
			// Now do it through OSC:
			newFreq = 440.0 * (1 + rand() /(float) RAND_MAX);
			newAmpDb = -40.0 * rand() /(float) RAND_MAX;
			sender.send("/Frequency", newFreq);
			sender.send("/Amplitude", newAmpDb);
			cout << "Setting through OSC: Frequency " << newFreq << " Amplitude " << newAmpDb << endl;
			al_sleep_nsec(interval * 1000000000.0);
		}
	}
};

// The audio callback reads the parameters with the get function
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

// This callback is registered below using the Parameter::setProcessingCallback()
// function. It is called any time the parameter's value is changed, no
// matter how that change occured.
float ampCallback(float ampdb, void *data) {
	float amp = powf(10.0, ampdb/20.0);
//	cout << " --- Amp ---- " << amp  << " ----" << endl;
	return amp;
}

int main (int argc, char * argv[])
{
	double sr = 44100;
	
	// Thread that will generate random values for the parameters
	MyThreadFunction func;
	Thread th(func);

	// Set the function to be called whenever the value of the "amp"
	// parameter changes
	amp.setProcessingCallback(ampCallback, nullptr);

	ParameterServer paramServer("127.0.0.1", 9010);
	// You can register parameters through the registerParameter() function
//	paramServer.registerParameter(freq);
//	paramServer.registerParameter(amp);
	// Or you can use the streaming operators:
	paramServer << freq << amp;

	// Print information about the server. Shows address, port and OSC parameter addresses
	paramServer.print();

	paramServer.addListener("127.0.0.1", 13560); // Try listening for OSC on this port to check that values are being forwarded
	
	// Setup and start audio
	AudioIO audioIO(AUDIO_BLOCK_SIZE, sr, audioCB, 0, 2, 0);
	gam::Domain::master().spu(audioIO.framesPerSecond());
	audioIO.start();
	
	MainLoop::start();
	func.running = false; // Signal the function to end
	th.join(); // join the thread (wait until thread function exits)
	
	return 0;
}
