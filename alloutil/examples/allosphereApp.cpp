
//#define ALLOSPHERE_BUILD_AUDIO_RENDERER
//#define ALLOSPHERE_BUILD_GRAPHICS_RENDERER
#include "alloutil/al_AllosphereApp.hpp"
#include "allocore/graphics/al_Mesh.hpp"

using namespace al;

class State {
public:
	State() : value(0.0) {}
	float value;
};

class GraphicsRenderer : public GraphicsRendererBase<State> {
public:
	GraphicsRenderer() {
		addSphere(mMesh, 0.1);
	}

	virtual void onAnimate(al_sec dt) override {
		// light.pos(nav().pos());
		pose = nav();
	}

	virtual void onDrawOmni(OmniStereo& om) override {

		Graphics g = graphics();

		bool newState = updateState();
//		if (newState) {
//			cout << "New State! " << state().value << std::endl;
//		}
		omni().uniforms(shader());

		shader().uniform("lighting", 0.5);
		shader().uniform("texture", 0.5);

	    omni().clearColor() = Color(state().value, 0, 0);

		g.polygonMode(Graphics::FILL);
		g.antialiasing(Graphics::NICEST);
		g.depthTesting(true);
		g.blending(true);

		g.draw(mMesh);
	}

private:
	Mesh mMesh;
};


class SimulatorApp : public SimulatorBase<State> {
public:

	virtual void onAnimate(double dt) override {
		state().value += 0.01;
		if (state().value >= 1.0) {
			state().value = 0;
			cout << "State wrapped!" << std::endl;
		}
//		cout << "New State! " << state().value << std::endl;
		sendState();
	}
};

class AudioRenderer : public AudioRendererBase<State> {
public:
	AudioRenderer() : mPhase(0), mPhaseInc(0)
	{
	}

//	virtual void initAudio() override
//	{
//		AudioRendererBase::initAudio();
//	}

	virtual void onSound(AudioIOData &io) override
	{
		float frequency;
		if (updateState()) {
			frequency = 220.0 + (state().value * 660.0);
			mPhaseInc = 2 * M_PI * frequency / io.framesPerSecond();
//			std::cout << "onSound : " << value << std::endl;
		}
		while (io()) {
			io.out(0) = sin(mPhase);
			mPhase += mPhaseInc;
			while (mPhase >= 2 * M_PI) {
				mPhase -= 2 * M_PI;
			}
		}

	}
private:
	double mPhase;
	double mPhaseInc;
};


int main(int argc, char *argv[])
{
	AlloSphereApp<State, GraphicsRenderer, SimulatorApp, AudioRenderer> app;
	app.start();
	return 0;
}
