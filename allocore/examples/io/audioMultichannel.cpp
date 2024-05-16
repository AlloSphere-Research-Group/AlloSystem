/*
Allocore Example: Multichannel Audio

Description:
Sequences through each channel of the output device playing a sound.

Author:
Lance Putnam, 2022
*/

#include "allocore/io/al_App.hpp"
#include "allocore/sound/al_AudioTestKit.hpp"
using namespace al;

class MyApp : public App{
public:
	SineOsc osc;
	LinDec env;
	Noise noise;
	int chan = 0;

	MyApp(){
		/* Look for a specific output device
		auto devO = audioIO().findDevice([](auto dev){
			// Aureon XFire
			//return dev.channelsOut == 8 && dev.nameMatches("USB2.0");
			// Any multichannel
			return dev.channelsOut > 2;
		});
		devO.print();
		audioIO().deviceOut(devO);
		//*/

		initAudio(48000, 512, -1, 0); // -1 opens all channels
		audioIO().printDevices();
		audioIO().print();
	}

	void onSound(AudioIOData& io) override {
		osc.freq(440., io.fps());
		env.length(1., io.fps());

		while(io()){
			//float s = noise() * env();
			float s = osc() * env();

			if(env.done()){
				env.reset();
				osc.phase(0);
				++chan %= io.channelsOut();
			}

			s *= 0.2;
			io.out(chan) = s;
		}
	}

	void onKeyDown(const Keyboard& k) override {
		switch(k.key()){
		case 'n': break;
		}
	}
};


int main(){
	MyApp().start();
}
