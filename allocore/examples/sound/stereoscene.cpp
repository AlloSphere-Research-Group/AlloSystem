// AudioScene example using the Stereo panner class
// By Andrés Cabrera mantaraya36@gmail.com
// March 7 2017

#include <iostream>


#include "allocore/sound/al_StereoPanner.hpp"
#include "allocore/sound/al_AudioScene.hpp"
#include "allocore/io/al_App.hpp"

#define BLOCK_SIZE 256
#define SAMPLE_RATE 44100

using namespace al;

static SoundSource soundSource;
static AudioScene scene(BLOCK_SIZE);
static Listener *listener;
static StereoPanner *panner;
static SpeakerLayout speakerLayout;

class MyApp : public App {
public:
	MyApp() {
		initWindow(Window::Dim(640, 480), "Stereo Audio Scene", 30);
		// Make a shape that will mark where the sound comes from.
		addTetrahedron(graphics().mesh(), 0.2);
		graphics().mesh().color(RGB(1));
		graphics().mesh().generateNormals();

		// Audio
		initAudio(SAMPLE_RATE, BLOCK_SIZE, 2, 0);
		mEnvelope = 0.3;
		speakerLayout = StereoSpeakerLayout();
		panner = new StereoPanner(speakerLayout);
		listener = scene.createListener(panner);
        // Doppler mode can be set per source:
        soundSource.dopplerType(DOPPLER_NONE);
		scene.addSource(soundSource);
	}

	virtual void onSound(AudioIOData &io) override {

		while(io()) {
			mEnvelope *= 0.9998;
			float sound = sin(mPhase) * mEnvelope;
            mPhase += M_PI * 2.0 * 440.0/io.framesPerSecond();
            if (mPhase > 2.0 * M_PI) {
                mPhase -= 2.0 * M_PI;
            }
			if (mEnvelope < 0.0001) { mEnvelope = 0.3; mPhase = 0.0;}
			soundSource.writeSample(sound);
//			io.out(0) = noise;
		}
		scene.render(io);
	}

	virtual void onDraw(Graphics &g) override {
		// Update the listener position from the visual scene's nav()
		listener->pose(nav());
//		std::cout << listener->pose().x() << "," << listener->pose().z() << std::endl;
//		std::cout << soundSource.pos().x << "," << soundSource.pos().z << std::endl;
		mLight();
		g.pushMatrix();
		// Use the mouse Y position for distance
		// Scale the mouse pixel position to the current window size
		g.translate(mX, 0.0, mY);
		g.draw(g.mesh());
		g.popMatrix();
	}

	virtual void onMouseMove(const Mouse &m) override {
		// Update mouse position
		mX = m.x()/320.0 - 1.0;
		mY = 4 -m.y()/40.0;
		// Sound source position is set whenever the mouse is moved
		soundSource.pos(mX, 0.0, mY);
	}

private:
	double mX, mY;
	Light mLight;
	// Sound variables
	float mEnvelope;
    float mPhase {0.0};
};

int main(int argc, char *argv[])
{
	MyApp().start();
	return 0;
}

