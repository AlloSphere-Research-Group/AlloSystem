#include <cmath>
#include "utAllocore.h"
#include "allocore/io/al_AudioIO.hpp"
using namespace al;

struct LowPass{
	LowPass(): p(0){}
	float operator()(float v, float f){ return p = p*(1-f) + v*f; }
	float p;
};

struct SineWave{
	SineWave(float freq): p(0), f(freq){}
	float operator()(){ p+=f; p=p>1?p-1:(p<0?p+1:p); return std::sin(p*6.2831853); }
	float p,f;
};

template <int N=20000>
struct DelayLine{
	DelayLine(): tap(0) { for(int i=0; i<N; ++i) buf[i]=0; }
	float operator()(float v){ buf[tap]=v; return buf[tap=(++tap)==N?0:tap]; }
	float buf[N];
	int tap;
};

int utIOAudioIO(){

	float SR = 44100;
	int blockSize = 256;
	int chansI = 1;
	int chansO = 1;

	LowPass lpf;
	SineWave sine(440./SR);
	DelayLine<> delay;

	AudioIO io;
	io.configure([&](AudioIOData& io){
		while(io()){
			float s = io.in(0);
			s = delay(lpf(s, 0.2));
			//s = float(i)/io.framesPerBuffer();
			//s = 0;
			s += sine()*0.1;

			io.out(0) = s;
			io.out(1) = s;
		}
	}, blockSize, SR, chansO, chansI);

	// Make sure parameters match those passed to constructor
	io.open();
	assert(io.framesPerBuffer() == blockSize);
	assert(io.fps() == SR);
	assert(io.channelsOut() == chansO);
	assert(io.channelsIn() == chansI);
	io.close();

/*
	// Test virtual channels
	int maxChansOut = AudioDevice::defaultOutput().channelsOutMax();
	int maxChansIn  = AudioDevice::defaultInput().channelsInMax();
	io.channelsOut(maxChansOut + 1);
	io.channelsIn (maxChansIn  + 1);
	io.open();
	assert(io.channelsOutDevice() == maxChansOut); // opened all hardware channels?
	assert(io.channelsOut() == (maxChansOut+1)); // got our extra virtual channel?
	assert(io.channelsInDevice() == maxChansIn); // opened all hardware channels?
	assert(io.channelsIn() == (maxChansIn+1)); // got our extra virtual channel?
*/
	//io.start();

	//printf("\nPress 'enter' to quit...\n"); getchar();
	return 0;
}
