#ifndef AL_WORLD_HPP_INC
#define AL_WORLD_HPP_INC

#include <math.h>
#include <string>
#include <vector>

#include "allocore/al_Allocore.hpp"
#include "alloutil/al_ControlNav.hpp"

namespace al{


/// Viewpoint within a scene

/// A viewpoint is an aggregation of a viewport (screen region), a pose
/// (3D position and orientation), and a camera.
class Viewpoint{
public:

	Viewpoint(const Pose& transform = Pose::identity())
	:	mViewport(0,0,0,0),
		mParentTransform(0),
		mAnchorX(0), mAnchorY(0), mStretchX(1), mStretchY(1),
		mCamera(0), mClearColor(0)
	{}

	float anchorX() const { return mAnchorX; }
	float anchorY() const { return mAnchorY; }
	float stretchX() const { return mStretchX; }
	float stretchY() const { return mStretchY; }

	Viewpoint& anchor(float ax, float ay){
		mAnchorX=ax; mAnchorY=ay; return *this;
	}

	Viewpoint& stretch(float sx, float sy){
		mStretchX=sx; mStretchY=sy; return *this;
	}

	bool hasCamera() const { return 0 != mCamera; }
	bool hasClearColor() const { return 0 != mClearColor; }

	const Camera& camera() const { return *mCamera; }
	Viewpoint& camera(Camera& v){ mCamera=&v; return *this; }
	
	const Color& clearColor() const { return *mClearColor; }
	Viewpoint& clearColor(Color& v){ mClearColor=&v; return *this; }

	const Pose* parentTransform() const { return mParentTransform; }
	Viewpoint& parentTransform(Pose& v){ mParentTransform = &v; return *this; }

	const Pose& transform() const { return mTransform; }
	Pose& transform(){ return mTransform; }
	
	Pose worldTransform() const { return mParentTransform ? (*mParentTransform) * transform() : transform(); }
	
	const Viewport& viewport() const { return mViewport; }
	Viewport& viewport(){ return mViewport; }

protected:
	Viewport mViewport;				// screen display region
	Pose * mParentTransform;		// parent transform, 0 if none
	Pose mTransform;				// local transform
	float mAnchorX, mAnchorY;		// viewport anchor factors relative to parent window
	float mStretchX, mStretchY;		// viewport stretch factors relative to parent window
	Camera * mCamera;				// camera; if not set, will be set to scene's default camera
	Color * mClearColor;
};



class Actor : public SoundSource, public Nav {//, public gfx::Drawable {
public:

	virtual ~Actor(){}

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
	virtual void onDraw(Graphics& g, const Viewpoint& v){}

protected:
};


//class Spectator : public Listener {
//public:
//	typedef std::vector<Viewpoint *> Viewpoints;
//
//protected:
//};


/// A window with one or more Viewpoints
class ViewpointWindow : public Window {
public:
	using Window::add;
	typedef std::vector<Viewpoint *> Viewpoints;

	ViewpointWindow(){
		init();
	}
	
	ViewpointWindow(
		int l, int t, int w, int h,
		const std::string title,
		double fps=40,
		DisplayMode::t mode = DisplayMode::DefaultBuf
	){
		init();
		create(Dim(l,t,w,h), title, fps, mode);
	}

	const Viewpoints& viewpoints() const { return mViewpoints; }
	
	void add(Viewpoint& v){ mViewpoints.push_back(&v); }


	virtual bool onResize(int dw, int dh){
		Viewpoints::iterator iv = mViewpoints.begin();
		
		while(iv != mViewpoints.end()){
			Viewpoint& vp = **iv;

			vp.viewport().l += dw * vp.anchorX();
			vp.viewport().b += dh * vp.anchorY();
			vp.viewport().w += dw * vp.stretchX();
			vp.viewport().h += dh * vp.stretchY();

			++iv;
		}
		return true;
	}

protected:
	Viewpoints mViewpoints;

private:
	void init(){
		add(new StandardWindowKeyControls);
	}
};



class World {
public:

	typedef std::vector<Actor *> Actors;
	typedef std::vector<Listener *> Listeners;
	typedef std::vector<ViewpointWindow *> Windows;

	World(
		const std::string& name="",
		double audioRate=44100, int audioBlockSize=128,
		int audioOutputs=-1, int audioInputs=-1
	);
	
	virtual ~World();

	const AudioIO&		audioIO() const { return mAudioIO; }
	AudioIO&			audioIO(){ return mAudioIO; }

	const AudioScene&	audioScene() const { return mAudioScene; }
	AudioScene&			audioScene(){ return mAudioScene; }

	const Actors&		actors() const { return mActors; }
	Actors&				actors(){ return mActors; }

	const Camera&		camera() const { return mCamera; }
	Camera&				camera(){ return mCamera; }

	const Graphics&		graphics() const { return mGraphics; }
	Graphics&			graphics(){ return mGraphics; }

	const std::string&	name() const { return mName; }
	World&				name(const std::string& v){ mName=v; return *this; }

	const Nav&			nav() const { return mNav; }
	Nav&				nav(){ return mNav; }

	const Nav&			navDraw() const { return mNavDraw; }
	Nav&				navDraw(){ return mNavDraw; }

	const NavInputControl& navControl() const { return mNavControl; }
	NavInputControl&	navControl(){ return mNavControl; }

	osc::Recv&			oscRecv(){ return mOSCRecv; }
	osc::Send&			oscSend(){ return mOSCSend; }

	const Stereographic& stereo() const { return mStereo; }
	Stereographic&		stereo(){ return mStereo; }

	const Windows&		windows() const { return mWindows; }
	Windows&			windows(){ return mWindows; }

	/// Add an actor to the world
	void add(Actor& v){
		mActors.push_back(&v);
	}
	
	/// Add a window to the world
	
	/// @param[in] win			The window to add
	/// @param[in] animates		Whether actors are animated based on frame rate
	///							of this window. There should only be one window
	///							in the world that animates.
	void add(ViewpointWindow& win, bool animates=false);


	void sendHandshake(){
		oscSend().send("/handshake", name(), oscRecv().port());
	}
	
	void sendDisconnect(){
		oscSend().send("/disconnectApplication", name());
	}

	void start(){
		if(name()!="" && oscSend().opened()) sendHandshake();
		mAudioIO.start();
		MainLoop::start();
	}

protected:
	
	Actors mActors;
	Listeners mListeners;
	Windows mWindows;
	Nav mNav;
	Nav mNavDraw;	// this version remains invariant throughout all drawing
	Camera mCamera;

	Stereographic mStereo;
	GraphicsGL mGraphics;
	AudioIO mAudioIO;
	AudioScene mAudioScene;
	
	NavInputControl mNavControl;

	std::string mName;
	
	osc::Recv mOSCRecv;
	osc::Send mOSCSend;

	static void sAudioCB(AudioIOData& io){
		World& w = io.user<World>();
		int numFrames = io.framesPerBuffer();
		
		//w.mNavMaster.velScale(4);
		//w.mNavMaster.step(io.secondsPerBuffer());
		w.mNav.smooth(0.95);
		w.mNav.step(1./4);
		w.mListeners[0]->pose(w.mNav);

		Actors::iterator it = w.mActors.begin();

		while(it != w.mActors.end()){
			io.frame(0);
			(*it)->onSound(w.mAudioIO);
			++it;
		}

		w.mAudioScene.encode(numFrames, io.framesPerSecond());
		w.mAudioScene.render(&io.out(0,0), numFrames);
		//printf("%f\n", io.out(0,0));
	}

	// This is called by each window to render all actors in each of its view regions
	struct DrawActors : public WindowEventHandler{
		DrawActors(ViewpointWindow& win_, World& w_): win(win_), w(w_){}

		virtual bool onFrame(){

			Graphics& g = w.mGraphics;
			//int w = v.dimensions().w;
			//int h = v.dimensions().h;
			
			w.navDraw() = w.nav();
			
			g.depthTesting(true);
			
//			printf("%f %f %f\n", w.nav().x(), w.nav().y(), w.nav().z());
//			printf("%p: %f %f %f\n", &v.camera(), cam.x(), cam.y(), cam.z());

			struct DrawAllActors : public Drawable {
				DrawAllActors(Actors& as_, World& w_, Viewpoint& v_)
					:	as(as_), w(w_), v(v_){}
				virtual void onDraw(Graphics& g){
					Actors::iterator ia = as.begin();
					while(ia != as.end()){
						Actor& a = *(*ia);
						g.pushMatrix(g.MODELVIEW);
						//g.multMatrix(a.matrix()); // draw in relative coords
						a.onDraw(g,v);
						g.popMatrix(g.MODELVIEW);
						++ia;
					}
				}
				Actors& as;
				World& w;
				Viewpoint& v;
			};

			ViewpointWindow::Viewpoints::const_iterator iv = win.viewpoints().begin();
			
			while(iv != win.viewpoints().end()){
				Viewpoint& vp = *(*iv);
				
				// if no camera, set to default scene camera
				if(!vp.hasCamera()) vp.camera(w.camera());

				const Camera& cam = vp.camera();

				// get viewpoint in world coords
//				Viewpoint vpWorld = vp;
//				vpWorld.transform() = w.nav() * vpWorld.transform();
//
//				DrawAllActors drawFunc(w.mActors, w, vpWorld);
//				w.mStereo.draw(g, cam, vpWorld.transform(), vpWorld.viewport(), drawFunc);
				//vpWorld.transform().pos() -= Vec3d(w.nav().uf()*4);

				Color defaultClearColor = w.mStereo.clearColor();
				if(!vp.hasClearColor()){
					vp.clearColor(const_cast<Color&>(w.mStereo.clearColor()));
				}
				else{
					w.mStereo.clearColor(vp.clearColor());
				}

				DrawAllActors drawFunc(w.mActors, w, vp);
				w.mStereo.draw(g, cam, vp.worldTransform(), vp.viewport(), drawFunc);
				
				w.mStereo.clearColor(defaultClearColor);
				
				++iv;
			}

			return true;
		}

		ViewpointWindow& win;
		World& w;
	};
	
	struct AnimateActors : public WindowEventHandler{
	
		AnimateActors(World& w_): w(w_){}
	
		virtual bool onFrame(){
			Actors::iterator ia = w.mActors.begin();
			while(ia != w.mActors.end()){
				Actor& a = *(*ia);
				a.step();
				a.onAnimate(window().spf());
				++ia;
			}
			return true;			
		}
		World& w;
	};
	
	struct SceneInputControl : public InputEventHandler{
		SceneInputControl(World& w_): w(w_){}
		
		virtual bool onKeyDown(const Keyboard& k){
			switch(k.key()){
				case Key::Tab: w.stereo().stereo(!w.stereo().stereo()); return false;
				default:;
			}
			return true;
		}

		World& w;
	};

};



//class Clocked{
//public:
//	virtual void onUpdate(double dt){}
//protected:
//};
//
//
//class Clock{
//public:
//
//	void add(Clocked& v){ mListeners.push_back(&v); }
//
//	void update(double dt){
//		Listeners::iterator it = mListeners.begin();
//		while(it != mListeners.end()){
//			(*it)->onUpdate(dt);
//		}
//	}
//
//protected:
//	typedef std::vector<Clocked *> Listeners;
//	Listeners mListeners;
//};

} // al::
#endif
