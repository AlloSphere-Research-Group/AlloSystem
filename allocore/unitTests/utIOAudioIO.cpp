#include "utAllocore.h"

struct LowPass{
	LowPass(): p(0){}
	float operator()(float v, float f){ return p = p*(1-f) + v*f; }
	float p;
} lpf;

struct SineWave{
	SineWave(float freq): p(0), f(freq){}
	float operator()(){ p+=f; p=p>1?p-1:(p<0?p+1:p); return sin(p*M_2PI); }
	float p,f;
} sine(440./44100.);

template <int N=20000>
struct DelayLine{
	DelayLine(): tap(0) { for(int i=0; i<N; ++i) buf[i]=0; }
	float operator()(float v){ buf[tap]=v; return buf[tap=(++tap)==N?0:tap]; }
	float buf[N];
	int tap;
};

DelayLine<> delay;

void audioCB(AudioIOData& io){

	for(int i=0; i<io.framesPerBuffer(); ++i){
		float s = io.in(0,i);
		s = delay(lpf(s, 0.2));
		//s = float(i)/io.framesPerBuffer();
		//s = 0;
		s += sine()*0.1;

		io.out(0,i) = s;
		io.out(1,i) = s;
	}
}



int utIOAudioIO(){

	//AudioDevice::printAll();
	AudioIO audioIO(256, 44100, audioCB, 0, 1, 1);

	// Make sure parameters match those passed to constructor
	audioIO.open();
	assert(audioIO.framesPerBuffer() == 256);
	assert(audioIO.fps() == 44100);
	assert(audioIO.channelsOut() == 1);
	assert(audioIO.channelsIn() == 1);
	audioIO.close();

	// Test virtual channels
	int maxChansOut = AudioDevice::defaultOutput().channelsOutMax();
	int maxChansIn  = AudioDevice::defaultInput().channelsInMax();
	audioIO.channelsOut(maxChansOut + 1);
	audioIO.channelsIn (maxChansIn  + 1);
	audioIO.open();
	assert(audioIO.channelsOutDevice() == maxChansOut); // opened all hardware channels?
	assert(audioIO.channelsOut() == (maxChansOut+1)); // got our extra virtual channel?
	assert(audioIO.channelsInDevice() == maxChansIn); // opened all hardware channels?
	assert(audioIO.channelsIn() == (maxChansIn+1)); // got our extra virtual channel?

//	audioIO.start();

	//printf("\nPress 'enter' to quit...\n"); getchar();
	return 0;
}
