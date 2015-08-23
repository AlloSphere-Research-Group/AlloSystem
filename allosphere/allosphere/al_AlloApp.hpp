#ifndef INC_AL_UTIL_ALLOAPP_HPP
#define INC_AL_UTIL_ALLOAPP_HPP

#include "alloutil/al_OmniStereoGraphicsRenderer.hpp"
#include "Cuttlebone/Cuttlebone.hpp"
#include "alloutil/al_Simulator.hpp"
#include "alloutil/al_AlloSphereAudioSpatializer.hpp"
#include "alloutil/al_InterfaceServerClient.hpp"

/*!
	Application for distributed rendering in the Allosphere
*/


namespace al {

template<class State>
class AlloApp : public OmniStereoGraphicsRenderer, public AlloSphereAudioSpatializer, public Simulator
{
public:
	AlloApp() :
		mMaker(Simulator::defaultBroadcastIP()),
		Simulator(Simulator::defaultInterfaceServerIP())
	{
		mState = new State;
		memset(mState, 0, sizeof(State));

		SearchPaths searchPaths;
		searchPaths.addSearchPath(".", false);
		searchPaths.addSearchPath("/alloshare/blob", false);
		searchPaths.addAppPaths();

		init();
		if (!initState()) {
			// Print warning or quit?
		}
		initWindow(Window::Dim(0, 0, 600, 400), "Blob Control Center", 60);

		// init audio and ambisonic spatialization
		AlloSphereAudioSpatializer::initAudio();
		AlloSphereAudioSpatializer::initSpatialization();

		// OSC
//		App::oscRecv().open(9999, "", 0.1, Socket::UDP|Socket::DGRAM);
//		App::oscRecv().handler(*this);
//		App::oscRecv().start();

//		// set interface server nav/lens to App's nav/lens
		InterfaceServerClient::setNav(OmniStereoGraphicsRenderer::nav());
		InterfaceServerClient::setLens(OmniStereoGraphicsRenderer::lens());

		AlloSphereAudioSpatializer::audioIO().start();
		InterfaceServerClient::connect();
		mMaker.start(); // it's important to call this.
//		start();
	}

	virtual void init() {}

	virtual bool initState() {}

	virtual void onSound(AudioIOData &io) {}

	virtual void start() {
		mTaker.start();                        // non-blocking
		OmniStereoGraphicsRenderer::start();  // blocks
	  }

	virtual void step(double dt) {}

	virtual void exit() {}

	State *state() {return mState;}
	cuttlebone::Taker<State, 9000> &taker() {return mTaker;}
	cuttlebone::Maker<State, 9000> &maker() {return mMaker;}

private:
	cuttlebone::Maker<State, 9000> mMaker;
	cuttlebone::Taker<State, 9000> mTaker;
	State *mState;
};

} // al::

#endif 
