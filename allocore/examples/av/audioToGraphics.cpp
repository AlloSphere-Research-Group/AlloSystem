/*
Allocore Example: Audio To Graphics

Description:
This example demonstrates how to visualize real-time audio. It uses a single
ring buffer to buffer audio samples between the audio and graphics threads. Two
sine waves are generated in the audio thread and drawn as a Lissajous curve in
the graphics thread.

Author:
Lance Putnam, 10/2012, putnam.lance@gmail.com
*/

#include "allocore/io/al_App.hpp"
using namespace al;

class MyApp : public App{
public:

	double phase = 0.;
	RingBuffer<Vec2f> ring{1024};
	Mesh curve;

	MyApp(){
		nav().pos(0,0,4);
		initWindow();
		initAudio();
	}

	// Audio callback
	void onSound(AudioIOData& io) override {

		// Set the base frequency to 55 Hz
		double freq = 55/io.framesPerSecond();

		while(io()){

			// Update the oscillators' phase
			phase += freq;
			if(phase > 1) phase -= 1;

			// Generate two sine waves at the 5th and 4th harmonics
			float out1 = cos(5*phase * 2*M_PI);
			float out2 = sin(4*phase * 2*M_PI);

			/* Write the waveforms to the ring buffer.
			Note that this call CANNOT block or wait on a lock from somewhere
			else. The audio	thread must keep writing to the buffer without
			regard for what other threads might be reading from it. */
			ring.write(Vec2f(out1, out2));

			// Send scaled waveforms to output...
			io.out(0) = out1*0.2;
			io.out(1) = out2*0.2;
		}
	}


	void onAnimate(double dt) override {

		curve.reset().lineStrip();

		/* We first need to determine the oldest sample we will attempt read
		from the buffer. Note that we do not want to attempt to read the entire
		history. We have to keep in mind that the audio thread will keep writing
		to the buffer no matter what. Therefore, it is very likely that the very
		oldest samples will get overwritten while we are reading. */
		int oldest = ring.size()-4;

		/* Here we cache the current write position of the ring buffer. We want
		this to remain fixed while we are reading. */
		int wpos = ring.pos();

		/* Now we read samples directly from the buffer from oldest to newest.
		As long as the amount of time between reading samples is faster than
		that of writing, we can expect to stay ahead of the audio thread... */
		for(int i=oldest; i>=0; --i){
			Vec2f point = ring.readFrom(wpos, i);
			curve.vertex(point.x, point.y);
		}
	}

	void onDraw(Graphics& g) override {
		g.draw(curve);
	}
};


int main(){
	MyApp().start();
}
