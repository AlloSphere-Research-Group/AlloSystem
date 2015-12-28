#ifndef AL_ALLOSPHEREAPP_HPP
#define AL_ALLOSPHEREAPP_HPP


#include "alloutil/al_OmniStereoGraphicsRenderer.hpp"
#include "allocore/io/al_App.hpp"
#include "allocore/io/al_AudioIO.hpp"
#include "allocore/graphics/al_Font.hpp"
#include "Cuttlebone/Cuttlebone.hpp"

#include <iostream>

#if !defined(ALLOSPHERE_BUILD_SIMULATOR) && !defined(ALLOSPHERE_BUILD_GRAPHICS_RENDERER) && !defined(ALLOSPHERE_BUILD_AUDIO_RENDERER)
#define ALLOSPHERE_BUILD_GRAPHICS_RENDERER
#endif

#if (defined(ALLOSPHERE_BUILD_SIMULATOR) && defined(ALLOSPHERE_BUILD_GRAPHICS_RENDERER)) || \
	(defined(ALLOSPHERE_BUILD_SIMULATOR) && defined(ALLOSPHERE_BUILD_AUDIO_RENDERER)) || \
	(defined(ALLOSPHERE_BUILD_GRAPHICS_RENDERER) && defined(ALLOSPHERE_BUILD_AUDIO_RENDERER))
#error More than one application type defined
#endif


namespace al {

class DummyState {};

// Audio Renderers ----------------------------------------

/// Base class for audio renderer (no state)
/// You should only inherit from this class if you don't need to receive state from the
/// simulator. You will usually want to inherit from AudioRendererBase.
class AudioRendererBaseNoState {
public:
	AudioRendererBaseNoState(int framesPerBuf=64, double framesPerSec=44100.0,
	                  void * userData = 0, int outChans = 2, int inChans = 0 ) :
	    io(framesPerBuf, framesPerSec, AudioRendererBaseNoState::AppAudioCB,
	       this, outChans, inChans)
	{
	}

	~AudioRendererBaseNoState();

	virtual void initAudio();

	virtual void onSound(AudioIOData & io) {}


protected:
	AudioIO io;
private:

	static inline void AppAudioCB(AudioIOData& io) {
		AudioRendererBaseNoState& app = io.user<AudioRendererBaseNoState>();
		io.frame(0);
		app.onSound(io);
	}
};

/// Base class for audio renderer
/// You should inherit from this class if you want to receive state from the
/// simulator. Otherwise, you can inherit from AudioRendererBaseNoState.
/// The state will be updated by a "simulator" or compute node that should
/// inherit from SimulatorBase.
template<typename State = DummyState, unsigned PORT = 63060>
class AudioRendererBase : public AudioRendererBaseNoState {
public:
	AudioRendererBase(int framesPerBuf=64, double framesPerSec=44100.0,
	                  void * userData = 0, int outChans = 2, int inChans = 0 ) :
	    AudioRendererBaseNoState(framesPerBuf, framesPerSec, userData,
	                             outChans, inChans)
	{
		mTaker.start();
	}

	virtual void initAudio() override { AudioRendererBaseNoState::initAudio(); }

	///
	/// \brief Audio callback function
	/// \param io Audio IO data
	/// All real-time audio read/write operations will take place within this
	/// function accessing the buffers and information provided by the io object.
	virtual void onSound(AudioIOData & io) {}

	///
	/// \brief updateState reads the state buffer and updates current state
	/// \return true if a new state was received
	///
	bool updateState() {
		bool newState = false;
		while (mTaker.get(mState) > 0) { newState = true;} // Pop all states in queue
		return newState;
	}

	///
	/// \brief state a reference to the shared state
	/// \return
	///
	State &state() { return mState;}

private:
	cuttlebone::Taker<State, 1400, PORT> mTaker;
	State mState;
};


//Simulator

template<typename State = DummyState, unsigned GRAPHICSPORT = 63059, unsigned AUDIOPORT = 63060>
class SimulatorBase : public App {
public:
	explicit SimulatorBase(const Window::Dim& dims = Window::Dim(320, 240),
	                          const std::string title="",
	                          double fps=40,
	                          Window::DisplayMode mode = Window::DEFAULT_BUF,
	                          int flags=0) :
	    mDims(dims), mTitle(title), mFps(fps), mMode(mode), mFlags(flags)
	{
	}

	virtual void initWindow();

	///
	/// \brief Override the onDraw to provide a GUI for the simulator
	///
	virtual void onDraw(Graphics &g, const Viewpoint &v) override;

	///
	/// \brief onAnimate contains the computation for the state
	/// \param dt the time delta since last call to onAnimate()
	///
	virtual void onAnimate(double dt) override{}

	///
	/// \brief Send the state to the audio and graphics renderers
	///
	void sendState() { mMakerGraphics.set(mState); mMakerAudio.set(mState);}

	///
	/// \brief state a reference to the shared state
	/// \return
	///
	/// You need to use this reference to set the computed state, as this is
	/// what is sent when
	State &state() { return mState;}

private:
	Window::Dim mDims;
	const std::string mTitle;
	double mFps;
	Window::DisplayMode mMode;
	int mFlags;

	State mState;
	cuttlebone::Maker<State, 1400, GRAPHICSPORT> mMakerGraphics;
	cuttlebone::Maker<State, 1400, AUDIOPORT> mMakerAudio;
};

// Graphics Renderer

template<typename State = DummyState, unsigned PORT = 63059>
class GraphicsRendererBase : public OmniStereoGraphicsRenderer {
public:
	GraphicsRendererBase() {
		mTaker.start();
	}

	///
	/// \brief override onDrawOmni() to draw graphics
	/// \param om OmniStereo object for rendering
	///
	virtual void onDrawOmni(OmniStereo& om) override {}

	///
	/// \brief updateState reads the state buffer and updates current state
	/// \return true if a new state was received
	///
	bool updateState() {
		bool newState = false;
		while (mTaker.get(mState) > 0) { newState = true;} // Pop all states in queue
		return newState;
	}

	State &state() { return mState;}

private:

	State mState;
	cuttlebone::Taker<State, 1400, PORT> mTaker;
};


///
/// The AlloSphereApp class provides a convenience bundling of functionality
/// that allow it to serve as the basis for simulator, graphics renderer and
/// audio renderer nodes. Depending on how the build is configured, it will
/// build the required functionality. The build should be configured
/// through cmake and the provided run scripts. You shouldn't need to change
/// anything in this app and the functionality should be implemented in the
/// classes passed as template parameters. Inherit from GraphicsRendererBase,
/// SimulatorBase and AudioRendererBase to provide the functionality.
///
/// Define the shared state as a class to be passed as the first template
/// parameter.
///
/// This convenience class handles the network connections (local or remote)
/// and the sending of state from the simulator to the graphics and audio
/// rendering nodes. Bear in mind that the same state is sent to both the
/// audio and graphics node which could create issues with bandwidth if the
/// state is too large or updated too frequently.
///
/// Typical usage looks like:
/// \code
/// int main(int argc, char *argv[])
/// {
/// 	AlloSphereApp<State, GraphicsRenderer, SimulatorApp, AudioRenderer> app;
/// 	app.start();
/// 	return 0;
/// }
/// \endcode
///
template<typename State = DummyState,
         typename RenderApp = GraphicsRendererBase<State>,
         typename SimulatorApp = SimulatorBase<State>,
         typename AudioApp = AudioRendererBase<State> >
#ifdef ALLOSPHERE_BUILD_GRAPHICS_RENDERER
class AlloSphereApp : public RenderApp, public AudioRendererBaseNoState
#endif
#ifdef ALLOSPHERE_BUILD_AUDIO_RENDERER
class AlloSphereApp : public AudioApp, public OmniStereoGraphicsRenderer
#endif
#ifdef ALLOSPHERE_BUILD_SIMULATOR
class AlloSphereApp : public OmniStereoGraphicsRenderer, public AudioRendererBaseNoState
#endif
{
public:
	AlloSphereApp()
	{

#ifdef ALLOSPHERE_BUILD_SIMULATOR
		SimulatorApp simulatorApp;
		simulatorApp.initWindow();
		simulatorApp.windows()[0]->title("Simulator");
//		std::cout << "Running Simulator" << std::endl;
		simulatorApp.start();
#endif


#ifdef ALLOSPHERE_BUILD_AUDIO_RENDERER
		this->initAudio();

Window::Dim(320, 240);
		this->dimensions(Window::Dim(320, 240));
		this->title("Audio Renderer");
//		std::cout << "Running Audio Renderer" << std::endl;
#endif

#ifdef ALLOSPHERE_BUILD_GRAPHICS_RENDERER
//		std::cout << "Running Graphics Renderer" << std::endl;
#endif

	}

	virtual ~AlloSphereApp() {}

	// Inherited from RenderApp
//	virtual void onDraw(Graphics& gl) override { }
//	virtual void onAnimate(al_sec dt) override {}
//	virtual bool onCreate() override {}
//	virtual bool onFrame() override {}
//	virtual void onDrawOmni(OmniStereo& omni) override {}
//	virtual std::string vertexCode() override {}
//	virtual std::string fragmentCode() override {}

#ifdef ALLOSPHERE_BUILD_AUDIO_RENDERER
	//Inherited from AudioApp
//	virtual void onSound(AudioIOData & io) override = 0;
#endif

protected:

private:

};

// Implementations

template<typename State, unsigned GRAPHICSPORT, unsigned AUDIOPORT>
void SimulatorBase<State, GRAPHICSPORT, AUDIOPORT>::initWindow()
{
	mMakerGraphics.start();
	mMakerAudio.start();
	App::initWindow(mDims, mTitle, mFps, mMode, mFlags);
}

template<typename State, unsigned GRAPHICSPORT, unsigned AUDIOPORT>
void SimulatorBase<State, GRAPHICSPORT, AUDIOPORT>::onDraw(Graphics &g, const Viewpoint &v) {
	float H = v.viewport().h;
	float W = v.viewport().w;
	g.pushMatrix(Graphics::PROJECTION);
	g.loadMatrix(Matrix4f::ortho2D(0, W, 0, H));
	g.pushMatrix(Graphics::MODELVIEW);

	g.blendAdd();
	Font f("allocore/share/fonts/VeraMono.ttf", 40);
	g.translate(int(W/2 - f.width("Simulator")/2), int(H/2 - f.size()/2));
	g.currentColor(1,1,0,1);
	f.render(g, "Simulator");

	g.popMatrix();
	g.popMatrix(Graphics::PROJECTION);
}

}


#endif // AL_ALLOSPHEREAPP_HPP

