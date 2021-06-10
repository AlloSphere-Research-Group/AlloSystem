#ifndef INCLUDE_AL_APP_HPP
#define INCLUDE_AL_APP_HPP
/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	Base class for common applications

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include <functional>
#include <memory> // unique_ptr
#include <string>
#include <vector>
// essential includes
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Lens.hpp"
#include "allocore/graphics/al_Stereoscopic.hpp"
#include "allocore/io/al_AudioIO.hpp"
#include "allocore/io/al_Window.hpp"
#include "allocore/io/al_ControlNav.hpp"
#include "allocore/math/al_Ray.hpp"
#include "allocore/spatial/al_Pose.hpp"
#include "allocore/types/al_Color.hpp"
// helper includes (keep minimal)
#include "allocore/graphics/al_Shapes.hpp"
#include "allocore/math/al_Random.hpp"

namespace al{

namespace osc{
	class Recv;
	class Send;
}
class SceneWindowHandler;


/// Viewpoint within a scene

/// A viewpoint is an aggregation of a viewport (screen region), a pose
/// (3D position and orientation), and a lens.
///
/// @ingroup allocore
class Viewpoint{
public:

	Viewpoint(const Pose& transform = Pose::identity());

	float anchorX() const { return mAnchorX; }
	float anchorY() const { return mAnchorY; }
	float stretchX() const { return mStretchX; }
	float stretchY() const { return mStretchY; }

	/// Set anchoring factors relative to bottom-left corner of window

	/// @param[in] ax	anchor factor relative to left edge of window, in [0,1]
	/// @param[in] ay	anchor factor relative to bottom edge of window, in [0,1]
	Viewpoint& anchor(float ax, float ay);

	/// Set stretch factors relative to bottom-left corner of window

	/// @param[in] sx	stretch factor relative to left edge of window, in [0,1]
	/// @param[in] sy	stretch factor relative to bottom edge of window, in [0,1]
	Viewpoint& stretch(float sx, float sy);

	bool hasLens() const { return NULL != mLens; }
	bool hasClearColor() const { return NULL != mClearColor; }

	/// Get lens
	const Lens& lens() const { return *mLens; }
	Viewpoint& lens(Lens& v){ mLens=&v; return *this; }

	/// Get clear color
	const Color& clearColor() const { return *mClearColor; }
	Viewpoint& clearColor(Color& v){ mClearColor=&v; return *this; }

	/// Get parent transform
	const Pose* parentTransform() const { return mParentTransform; }
	Viewpoint& parentTransform(Pose& v){ mParentTransform =&v; return *this; }
	Viewpoint& parentTransform(Pose* v){ mParentTransform = v; return *this; }

	/// Get local transform
	const Pose& transform() const { return mTransform; }
	Pose& transform(){ return mTransform; }
	Viewpoint& transform(const Pose& v){ mTransform=v; return *this; }

	Pose worldTransform() const { return mParentTransform ? (*mParentTransform) * transform() : transform(); }

	/// Get screen viewport
	const Viewport& viewport() const { return mViewport; }
	Viewport& viewport(){ return mViewport; }

	/// Get calculated viewing frustum
	Frustumd frustum() const;

	/// Call to update viewport using stretch/anchor amounts when parent dimensions change
	void onParentResize(int w, int h);

private:
	Viewport mViewport;				// screen display region
	Pose * mParentTransform;		// parent transform, 0 if none
	Pose mTransform;				// local transform
	float mAnchorX, mAnchorY;		// viewport anchor factors relative to parent window
	float mStretchX, mStretchY;		// viewport stretch factors relative to parent window
	Lens * mLens;					// if not set, will be set to scene's default lens
	Color * mClearColor;
};


/// A window with one or more Viewpoints
///
/// @ingroup allocore
class ViewpointWindow : public Window {
public:
	typedef std::vector<Viewpoint *> Viewpoints;

	///
	ViewpointWindow();

	/// @param[in] dims		window dimensions
	/// @param[in] title	window title
	/// @param[in] fps		frames/second
	/// @param[in] mode		window display mode
	ViewpointWindow(
		const Dim& dims,
		const std::string title="",
		double fps=40,
		DisplayMode mode = DEFAULT_BUF
	);


	/// Get the list of viewpoints
	Viewpoints& viewpoints(){ return mViewpoints; }
	const Viewpoints& viewpoints() const { return mViewpoints; }

	/// Add a new viewpoint to the window
	ViewpointWindow& add(Viewpoint& v);

	std::vector<std::function<void(void)>>& drawCalls(){ return mDrawCalls; }

protected:
	Viewpoints mViewpoints;
	std::vector<std::function<void(void)>> mDrawCalls;

	virtual bool onResize(int dw, int dh);

private:
	friend class SceneWindowHandler;
	StandardWindowKeyControls mStandardKeyControls;
	bool mResized;
};


/// Application helper class
///
/// @ingroup allocore
class App {
public:

	typedef std::vector<ViewpointWindow *> Windows;

	App();
	virtual ~App();


	/// Initialize audio

	/// @param[in] sampleRate		Sample rate.  Unsupported values will use default rate of device.
	/// @param[in] blockSize		Number of sample frames to process per callback
	/// @param[in] outputChannels	Number of output channels to open or -1 for all
	/// @param[in] inputChannels	Number of input channels to open or -1 for all
	void initAudio(
		double sampleRate=44100, int blockSize=128,
		int outputChannels=-1, int inputChannels=-1
	);


	/// Initialize a new window

	/// @param[in] dims				Window dimensions
	/// @param[in] title			Window title
	/// @param[in] fps				Frames/second
	/// @param[in] mode				Window display mode
	/// @param[in] flags			Additional flags (unused)
	ViewpointWindow * initWindow(
		const Window::Dim& dims = Window::Dim(800,600),
		const std::string title="",
		double fps=40,
		Window::DisplayMode mode = Window::DEFAULT_BUF,
		int flags=0
	);

	/// Start rendering; begins audio and drawing callbacks
	void start();


	/// Sound generation callback

	/// Sound can either be written directly to the audio output channels
	/// or to the sound source's internal buffer which is rendered later by the
	/// spatial audio decoder.
	virtual void onSound(AudioIOData& io){}

	/// Animation (model update) callback
	virtual void onAnimate(double dt){}

	/// Drawing callback (in world coordinates)

	/// This will be called from the main graphics renderer. Since it may be
	/// called multiple times, no state updates should be made in it.
	virtual void onDraw(Graphics& g, const Viewpoint& v){ onDraw(g); }
	virtual void onDraw(Graphics& g){}

	/// Called when a keyboard key is pressed
	virtual void onKeyDown(const ViewpointWindow& w, const Keyboard& k){ onKeyDown(k); }
	virtual void onKeyDown(const Keyboard& k){}

	/// Called when a keyboard key is released
	virtual void onKeyUp(const ViewpointWindow& w, const Keyboard& k){ onKeyUp(k); }
	virtual void onKeyUp(const Keyboard& k){}

	/// Called when a mouse button is pressed
	virtual void onMouseDown(const ViewpointWindow& w, const Mouse& m){ onMouseDown(m); }
	virtual void onMouseDown(const Mouse& m){}

	/// Called when a mouse button is released
	virtual void onMouseUp(const ViewpointWindow& w, const Mouse& m){ onMouseUp(m); }
	virtual void onMouseUp(const Mouse& m){}

	/// Called when the mouse moves while a button is down
	virtual void onMouseDrag(const ViewpointWindow& w, const Mouse& m){ onMouseDrag(m); }
	virtual void onMouseDrag(const Mouse& m){}

	/// Called when the mouse moves
	virtual void onMouseMove(const ViewpointWindow& w, const Mouse& m){ onMouseMove(m); }
	virtual void onMouseMove(const Mouse& m){}


	/// Called upon creation of a window
	virtual void onCreate(const ViewpointWindow& win){}

	/// Called upon destruction of a window
	virtual void onDestroy(const ViewpointWindow& win){}

	/// Called upon resize of a window
	virtual void onResize(const ViewpointWindow& win, int w, int h){}


	/// Called just before the app exits and before any object destructors
	virtual void onExit(){}


	/// Set application name
	App& name(const std::string& v){ mName=v; return *this; }

	/// Get application name
	const std::string&	name() const { return mName; }

	/// Get seconds since application start
	double appTime() const;

	/// Get app time at start of animate clock

	/// Unlike appTime(), this is held constant throughout the entire animation
	/// clock cycle and thus is the correct time to use for any animation.
	double animateTime() const { return mAnimateTime; }


	/// Get navigation pose (position/orientation)
	Nav& nav(){ return mNav; }
	const Nav& nav() const { return mNav; }

	/// Get navigation pose (position/orientation) used while drawing
	const Nav& navDraw() const { return mNavDraw; }
	Nav& navDraw(){ return mNavDraw; }

	/// Get navigation keyboard/mouse controller
	const NavInputControl& navControl() const { return mNavControl; }
	NavInputControl& navControl(){ return mNavControl; }


	/// Get audio i/o object
	AudioIO& audioIO(){ return mAudioIO; }
	const AudioIO& audioIO() const { return mAudioIO; }

	//const AudioScene&	audioScene() const { return mAudioScene; }
	//AudioScene&			audioScene(){ return mAudioScene; }


	/// Get the array of windows
	Windows& windows(){ return mWindows; }
	const Windows& windows() const { return mWindows; }

	/// Get a window by index
	ViewpointWindow& window(int i=0){ return *(windows()[i]); }
	const ViewpointWindow& window(int i=0) const { return *(windows()[i]); }

	/// Get mouse x coordinate in [0,1]
	float mouseX1(int window=0, bool clip=true);
	/// Get mouse y coordinate in [0,1]
	float mouseY1(int window=0, bool clip=true);


	/// Get graphics renderer
	Graphics& graphics(){ return mGraphics; }
	const Graphics& graphics() const { return mGraphics; }

	/// Get 3D rendering state
	Stereoscopic& stereo(){ return mStereo; }
	const Stereoscopic& stereo() const { return mStereo; }

	/// Get (camera) lens
	Lens& lens(){ return mLens; }
	const Lens& lens() const { return mLens; }

	/// Get background color
	const Color& background() const { return stereo().clearColor(); }

	/// Set background color
	App& background(const Color& c){ stereo().clearColor(c); return *this; }


	osc::Recv& oscRecv();
	osc::Send& oscSend();
	void sendHandshake();
	void sendDisconnect();

	App& clockAnimate(void * v){ mClockAnimate=v; return *this; }
	const void * clockAnimate() const { return mClockAnimate; }

	App& clockNav(void * v){ mClockNav=v; return *this; }
	const void * clockNav() const { return mClockNav; }

	/// Add a window to the app

	/// @param[in] win			The window to add
	///
	App& add(ViewpointWindow& win);


	/// Returns true if using audio
	bool usingAudio() const;

	/// Get a pick ray from screen space coordinates
	 // i.e. use mouse xy
	Rayd getPickRay(const ViewpointWindow& w, int screenX, int screenY);

private:

	typedef std::vector<Viewpoint *> Viewpoints;

	Viewpoints mFacViewpoints;
	Windows mFacWindows;
	double mStartTime = 0.;
	friend class SceneWindowHandler;
	double mAnimateTime = 0.;

	// graphics
	Windows mWindows;
	Lens mLens;
	Stereoscopic mStereo;
	Graphics mGraphics;

	// sound
	AudioIO mAudioIO;

	// spatial
	Nav mNav;
	Nav mNavDraw;	// this version remains invariant throughout all drawing

	// control
	NavInputControl mNavControl;
	std::unique_ptr<osc::Recv> mOSCRecv;
	std::unique_ptr<osc::Send> mOSCSend;

	std::string mName;
	void * mClockAnimate = NULL;
	void * mClockNav = NULL;
};

} // al::
#endif
