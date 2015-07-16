/*
Based on Allocore Example: Audio To Graphics by Lance Putnam

*/

#include <iostream>

#include "allocore/io/al_App.hpp"
#include "alloaudio/al_OutputMaster.hpp"

using namespace std;
using namespace al;

class MyApp : public App{
public:
    double phase;
    OutputMaster outMaster;

    MyApp(int num_chnls, double sampleRate,
          const char * address = "", int inport = 19375,
          const char * sendAddress = "localhost", int sendPort = -1)
        : outMaster(num_chnls, sampleRate, address, inport, sendAddress, sendPort)
    {
        nav().pos(0,0,4);
        initWindow();
		initAudio(sampleRate, 512, num_chnls, num_chnls);
    }

    // Audio callback
    void onSound(AudioIOData& io){

        // Set the base frequency to 55 Hz
        double freq = 55/io.framesPerSecond();

        while(io()){

            // Update the oscillators' phase
            phase += freq;
            if(phase > 1) phase -= 1;

            // Generate two sine waves at the 5th and 4th harmonics
            float out1 = cos(5*phase * 2*M_PI);
            float out2 = sin(4*phase * 2*M_PI);

            // Send scaled waveforms to output...
            io.out(0) = out1*0.2;
            io.out(1) = out2*0.2;
            io.out(2) = out1*0.4;
            io.out(3) = out2*0.4;
        }
		outMaster.onAudioCB(io);
    }


    void onAnimate(double dt){

    }

    void onDraw(Graphics& g, const Viewpoint& v){

    }
};


int main(){
	int num_chnls = 4;
	double sampleRate = 44100;
	const char * address = "localhost";
	int inport = 3002;
	const char * sendAddress = "localhost";
	int sendPort = 3003;
	cout << "Listening to \"" << address << "\" on port " << inport << endl;
	cout << "Sending to \"" << sendAddress << "\" on port " << sendPort << endl;
	MyApp(num_chnls, sampleRate, address, inport, sendAddress, sendPort).start();
}
