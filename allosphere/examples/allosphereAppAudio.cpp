
#include "allosphere/al_AlloSphereApp.hpp"
#include "allocore/graphics/al_Mesh.hpp"

// This exmaple shows how to use the AlloSphereApp and companion classes to
// create a distributed multimedia app. An AlloSphereApp is comprised of three
// separate applications that can run on different machines. One is the
// simulator that performs computation and updates model data. It pipes this
// "state" to the audio and graphics renderers. The graphics and audio renderers
// will receive the state and perform their specific functions.
// The AlloSystem build system will build and run the three applications for
// you without change either on a single machine, or on the appropiate
// machines from a cluster if a configuration file is found.

using namespace al;

// Define a "state". This is the information that the simulator will send
// to the graphics and audio renderers.
class State {
public:
	State() : value(0.0) {}
	float value;
};

// Define a state to broadcast to the audio renderers
class AudioState {
public:
	AudioState() : frequency(220.0) {}
	float frequency;
};

// For the simulator, you only need to override the onAnimate() function.
// The simulator will run by default at 30 fps and will display a small
// window that you can use to create a GUI or alternate display.
class SimulatorApp : public SimulatorBase<State, AudioState> {
public:
	virtual void onAnimate(double dt) override {
		// Write to the member variables of state as needed
		state().value += 0.01;
		if (state().value >= 1.0) {
			state().value = 0;
			cout << "State wrapped!" << std::endl;
		}
//		cout << "New State! " << state().value << std::endl;
		// When done and the state is ready to be updated and broadcast, call
		// the sendState function
		sendState();
		audioState().frequency = 220.0 + (state().value * 660.0);
		sendAudioState();
	}

	// If you need to draw on this window, override the onDraw() function:
//	virtual void onDraw(Graphics &g, const Viewpoint &v) override;
};

// For the graphics renderer, inherit from GraphicsRendererBase<State>
// Override the onDrawOmni() function to draw. Overriding the onAnimate()
// function as shown below will allow keyboard navigation to affect the
// render, which can be useful while prototyping
class GraphicsRenderer : public GraphicsRendererBase<State> {
public:
	GraphicsRenderer() {
		addSphere(mMesh, 0.1);
	}

	virtual void onAnimate(al_sec dt) override {
		// light.pos(nav().pos());
		// Allow keyboard navigation in the rendering window
		// Only really useful when running on the desktop
		pose = nav();
	}

	virtual void onDrawOmni(OmniStereo& om) override {

		Graphics g = graphics();

		// Call updateState() to process the state message queue. If there
		// is a new state, the function will return true. You can then
		// perform any calculations that need to be done when the state
		// changes.
		bool newState = updateState();
		if (newState) {
//			cout << "New State! " << state().value << std::endl;
		}
		omni().uniforms(shader());

		shader().uniform("lighting", 0.5);
		shader().uniform("texture", 0.5);

		// You can get the current state with state(). This will return a
		// variable of type State.
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


// The audio renderer will open the sound card and stream audio to it.
// Inherit from AudioRendererBase<AudioState>.
// Like the graphics renderer, it gets the state from the simulator. The
// onSound() function is the audio callback, so all audio processes will
// go there.
class AudioRenderer : public AudioRendererBase<AudioState> {
public:
	AudioRenderer() : mPhase(0), mPhaseInc(0)
	{
		initAudio();
	}

	// You can perform intialization operations in the initAudio() function,
	// but remember to initialize audio manually or call the parent's
	// AudioRendererBase::initAudio() function
//	virtual void initAudio() override
//	{
//		AudioRendererBase::initAudio();
//	}

	virtual void onSound(AudioIOData &io) override
	{
		if (updateAudioState()) {
			mPhaseInc = 2 * M_PI * audioState().frequency / io.framesPerSecond();
			std::cout << "onSound : " << audioState().frequency << std::endl;
		}
		while (io()) {
			io.out(0) = 0.1 * sin(mPhase);
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
	// To make the distributed applications, you need to create an instance
	// of AlloSphereApp passing the state, graphics, simultaor and audio
	// classes as template parameters
	AlloSphereApp<State, GraphicsRenderer, SimulatorApp, AudioRenderer> app;
	// Start the app.
	app.start();
	return 0;
}
