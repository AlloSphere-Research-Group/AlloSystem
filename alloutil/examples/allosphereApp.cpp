
//#define ALLOSPHERE_BUILD_SIMULATOR
//#define ALLOSPHERE_BUILD_AUDIO_RENDERER
#define ALLOSPHERE_BUILD_GRAPHICS_RENDERER
#include "alloutil/al_AllosphereApp.hpp"
#include "allocore/graphics/al_Mesh.hpp"

using namespace al;

class State {
public:
	float value;
};

class GraphicsRenderer : public GraphicsRendererBase<State> {
public:
	GraphicsRenderer() {
		addSphereWithTexcoords(mMesh);
	}

	virtual void onAnimate(al_sec dt) override {
		// light.pos(nav().pos());
		pose = nav();
	}

	virtual void onDrawOmni(OmniStereo& om) override {

//		std::cout << "OnDrawOmni" << std::endl;

		Graphics g = graphics();

		bool newState = updateState();
		if (newState) {
			cout << "New State!" << std::endl;
		}
		omni().uniforms(shader());

	    omni().clearColor() = Color(state().value);

		shader().uniform("lighting", 0.5);
		shader().uniform("texture", 0.5);


		g.polygonMode(Graphics::FILL);
		g.antialiasing(Graphics::NICEST);
//		g.depthTesting(true);
		g.blending(true);

		g.draw(mMesh);
	}

private:
	Mesh mMesh;
};


class SimulatorApp : public SimulatorBase<State> {
public:

	virtual void onAnimate(double dt) {
		state().value += 0.00001;
		sendState();
	}
};


int main(int argc, char *argv[])
{
	AlloSphereApp<State, GraphicsRenderer, SimulatorApp> app;
	app.start();
	return 0;
}
