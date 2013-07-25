/*
Allocore Example: Audio System Probe

Description:
A useful tool to test channels etc. of an audio device

Author:
Graham Wakefield, 2011
*/

#include "allocore/al_Allocore.hpp"
using namespace al;

int channels;
bool started = false;
rnd::Random<> rng;

float amp = 0.5;
std::vector<float> gains;
unsigned long long count = 0;

void solo(int c) {
	printf("solo %d\n", c);
	for (int i=0; i<channels; i++) gains[i] = 0.f;
	if (c < channels) gains[c] = 1.f;
}
void unmute() {
	for (int i=0; i<channels; i++) gains[i] = 1.f;
}

void audioCB(AudioIOData& io){
	double sr = io.fps();
	if (!started) {
		printf("callback started\n");
		started = true;
	}

	while(io()){
		float t1 = 1.f - fmod(count / sr, 1.);	// time in seconds
		for (int i=0; i<channels; i++) {
			float t2 = fmod(t1 * (i+1), 1.);
			io.out(i) = rng.uniformS() * gains[i] * amp * t2 * t1;
		}
		count++;
	}
}	


int main (int argc, char * argv[]){

	AudioDevice::printAll();

	const AudioDevice& dev = AudioDevice::defaultOutput();
	channels = dev.channelsOutMax();

	gains.resize(channels);
	unmute();
	dev.print();
	

	AudioIO audioIO; 
	audioIO.deviceIn(AudioDevice::defaultInput());
	audioIO.deviceOut(AudioDevice::defaultOutput());
	audioIO.callback = audioCB;
	audioIO.start();
	
	printf("\nPress + and - (and return) to adjust volume\nPress a number (and return) to solo the channel\nPress 'space' (and return) to hear all channels\nPress 'q' (and return) to quit...\n"); 
	while (true) {
		char c = getchar();
		switch(c) {
			case 'q':
				exit(0);
			case 48:
			case 49:
			case 50:
			case 51:
			case 52:
			case 53:
			case 54:
			case 55:
			case 56:
			case 57:
				solo(c - 48);
				break;
			case ' ':
				unmute();
				break;
			case '+':
				amp *= 2.f;
				printf("amp %f\n", amp);
				break;
			case '-':
				amp *= 0.5f;
				printf("amp %f\n", amp);
				break;
			default:
				printf("char %d\n", (int)c);
				break;
		}
		
	}
	return 0;
}
