#ifndef AL_ALLOSPHEREAPP_HPP
#define AL_ALLOSPHEREAPP_HPP


#include "alloutil/al_OmniStereoGraphicsRenderer.hpp"
#include "allocore/io/al_App.hpp"
#include "allocore/io/al_AudioIO.hpp"
#include "allocore/graphics/al_Font.hpp"
#include "Cuttlebone/Cuttlebone.hpp"

#include <iostream>

#if !defined(ALLOSPHERE_BUILD_SIMULATOR) && !defined(ALLOSPHERE_BUILD_GRAPHICS_RENDERER) && !defined(ALLOSPHERE_BUILD_AUDIO_RENDERER)
#define ALLOSPHERE_BUILD_SIMULATOR
#endif

#if (defined(ALLOSPHERE_BUILD_SIMULATOR) && defined(ALLOSPHERE_BUILD_GRAPHICS_RENDERER)) || \
	(defined(ALLOSPHERE_BUILD_SIMULATOR) && defined(ALLOSPHERE_BUILD_AUDIO_RENDERER)) || \
	(defined(ALLOSPHERE_BUILD_GRAPHICS_RENDERER) && defined(ALLOSPHERE_BUILD_AUDIO_RENDERER))
#error More than one application type defined
#endif

#ifndef ALLOAPP_PACKET_SIZE
#define PACKET_SIZE 1400
#else
#define PACKET_SIZE ALLOAPP_PACKET_SIZE
#endif

// The network ports used to communicate state from the simulator to the
// renderers
#define ALLOAPP_GRAPHICS_PORT 63059
#define ALLOAPP_AUDIO_PORT 63060

namespace al {

class DummyState {};

// Audio Renderers ----------------------------------------

/// Base class for audio renderer (no state)
/// You should only inherit from this class if you don't need to receive state from the
/// simulator. You will usually want to inherit from AudioRendererBase.
class AudioRendererBaseNoState {
public:
	AudioRendererBaseNoState(int framesPerBuf=128, double framesPerSec=44100.0,
	                         int outChans = 2, int inChans = 0 ) :
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
template<typename AudioState = DummyState, unsigned PORT = ALLOAPP_AUDIO_PORT>
class AudioRendererBase : public AudioRendererBaseNoState {
public:
	AudioRendererBase(int framesPerBuf=64, double framesPerSec=44100.0,
	                  int outChans = 2, int inChans = 0 ) :
	    AudioRendererBaseNoState(framesPerBuf, framesPerSec,
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
	virtual void onSound(AudioIOData & io) override {}

	///
	/// \brief updateAudioState reads the state buffer and updates current state
	/// \return true if a new state was received
	///
	bool updateAudioState() {
		bool newState = false;
		while (mTaker.get(mState) > 0) { newState = true;} // Pop all states in queue
		return newState;
	}

	///
	/// \brief popAudioState pops a single state from the state buffer and updates
	/// the current state
	/// \return Returns the number of states still pending in the buffer
	///
	int popAudioState() {
		return mTaker.get(mState);
	}

	///
	/// \brief state a reference to the shared state
	/// \return
	///
	AudioState &audioState() { return mState;}

private:
	cuttlebone::Taker<AudioState, 1400, PORT> mTaker;
	AudioState mState;
};


//Simulator

template<typename State = DummyState, typename AudioState = DummyState,
         unsigned GRAPHICSPORT = ALLOAPP_GRAPHICS_PORT,
         unsigned AUDIOPORT = ALLOAPP_AUDIO_PORT>
class SimulatorBase : public App, public osc::PacketHandler {
public:
	explicit SimulatorBase(const Window::Dim& dims = Window::Dim(320, 240),
	                       const std::string title="",
	                       double fps=60,
	                       Window::DisplayMode mode = Window::DEFAULT_BUF,
	                       int flags=0,
						   const char *broadcastIP = "127.0.0.1") :
	    mDims(dims), mTitle(title), mFps(fps), mMode(mode), mFlags(flags),
	    mMakerAudio(broadcastIP),
		mMakerGraphics(broadcastIP)
	{
		mMakerGraphics.start();
		mMakerAudio.start();
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
	/// \brief onMessage will recieve the messages from Device Server
	/// \param m
	///
	virtual void onMessage(osc::Message &m) override {};

	///
	/// \brief Send the state to the graphics renderers
	///
	void sendState() { mMakerGraphics.set(mState);}

	///
	/// \brief Send the state to the audio renderers
	///
	void sendAudioState() { mMakerAudio.set(mAudioState);}

	///
	/// \brief state a reference to the shared state
	/// \return
	///
	/// You need to use this reference to set the computed state, as this is
	/// what is sent when sendState() is called
	State &state() { return mState;}

	///
	/// \brief state a reference to the shared state
	/// \return
	///
	/// You need to use this reference to set the computed state, as this is
	/// what is sent when sendAudioState() is called
	AudioState &audioState() { return mAudioState;}

private:
	Window::Dim mDims;
	const std::string mTitle;
	double mFps;
	Window::DisplayMode mMode;
	int mFlags;

	State mState;
	cuttlebone::Maker<State, 1400, GRAPHICSPORT> mMakerGraphics;
	AudioState mAudioState;
	cuttlebone::Maker<AudioState, 1400, AUDIOPORT> mMakerAudio;
};

// Graphics Renderer
// TODO: can we pass a State reference in on the onDraw and onAnimate calls
// (e.g., something like void onDraw(Graphics& g, State& s)
// and onAnimate(double dt, State&, s))?
// this reminds the user that she must use State to communicate.
template<typename State = DummyState, unsigned PORT = ALLOAPP_GRAPHICS_PORT>
class GraphicsRendererBase : public OmniStereoGraphicsRenderer {
public:
	GraphicsRendererBase() {
		mTaker.start();
	}

	///
	/// \brief override onDraw() to draw graphics
	/// \param g Graphics object for rendering
	///
	virtual void onDraw(Graphics &g) override {}

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
	/// \brief popState pops a single state from the state buffer and updates
	/// the current state
	/// \return true if a new state was in the state buffer
	///
	bool popState() {
		return mTaker.get(mState) > 0;
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
class AlloSphereApp : public SimulatorApp
#endif
{
public:
	AlloSphereApp()
	{

#ifdef ALLOSPHERE_BUILD_SIMULATOR
//		this->initAudio();

		this->initWindow();
//		this->initAudio(44100, 1024, 2, 0);
		std::cout << "Running Simulator" << std::endl;
#endif


#ifdef ALLOSPHERE_BUILD_AUDIO_RENDERER
//		this->initAudio();
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

template<typename State, typename AudioState, unsigned GRAPHICSPORT, unsigned AUDIOPORT>
void SimulatorBase<State, AudioState, GRAPHICSPORT, AUDIOPORT>::initWindow()
{
	App::initWindow(mDims, mTitle, mFps, mMode, mFlags);
}

template<typename State, typename AudioState, unsigned GRAPHICSPORT, unsigned AUDIOPORT>
void SimulatorBase<State, AudioState, GRAPHICSPORT, AUDIOPORT>::onDraw(Graphics &g, const Viewpoint &v) {
	g.clearColor(Color(0.0, 1.0, 0.0));
	g.clear(Graphics::COLOR_BUFFER_BIT);
}

}


#endif // AL_ALLOSPHEREAPP_HPP

