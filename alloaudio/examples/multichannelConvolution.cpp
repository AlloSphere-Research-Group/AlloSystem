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

// A user defined class that can be accessed from the audio callback
const char * testPath = "/Users/ogc/Documents/ucsb/arg/AlloSystem/alloaudio/examples/test.wav";
SoundFile test(testPath);
bool once = true;

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
        //
        //
        const char * path = "/Users/ogc/Documents/ucsb/arg/AlloSystem/alloaudio/share/irs/Batcave.wav";
        //const char * path = "../share/irs/Batcave.wav";
        SoundFile sf(path);
        if(!sf.openRead()){
            cout << "Error: Could not read impulse response file." << endl;
        }
        int numIRChannels = sf.channels(), numFrames = sf.frames();
        cout << "Reading IR file '" << path << "'. Channels: " << numIRChannels << ". Frames: " << numFrames << "." << endl;
        double fs = sf.frameRate();
        if(sampleRate != fs){
            cout << "Error: Application's sampling rate (" << sampleRate << ") differs from that of the impulse response file (" << fs << ")." << endl;
        }
        float deinterleavedChannels[numIRChannels * numFrames];
        if(sf.readAllD(deinterleavedChannels) != numFrames){
            cout << "Error: failed to read expected number of frames." << endl;
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
                    cout << "Audio channel " << i << " disabled." << endl;
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
        
        //sanity check... 
        test.format(SoundFile::WAV);                                                                                                                                                          
        test.encoding(SoundFile::FLOAT);
        test.channels(1);
        test.frameRate(fs);

        if(!test.openWrite()){
            cout << "Error creating test file" << endl;
        }
        test.write(IRchannels[0], numFrames);
        test.close();
    }

    // Audio callback
    void onSound(AudioIOData& io){

        conv.onAudioCB(io);
		//outMaster.onAudioCB(io);
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
