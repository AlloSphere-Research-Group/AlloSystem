#include "utAllocore.h"

struct LowPass{
	LowPass(): p(0){}
	float operator()(float v, float f){ return p = p*(1-f) + v*f; }
	float p;
} lpf;

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
		float s = io.in(0)[i];
		s = delay(lpf(s, 0.2));
		//s = float(i)/io.framesPerBuffer();
		//s = 0;		

		io.out(0)[i] = s;
		io.out(1)[i] = s;
	}
}



int utIOAudioIO(){

	AudioDevice::printAll();
	AudioIO audioIO(128, 44100, audioCB, 0, 2, 1);
	audioIO.start();

	printf("\nPress 'enter' to quit...\n"); getchar();
	return 0;
}
