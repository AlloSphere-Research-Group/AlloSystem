#ifndef AL_WORLD_HPP_INC
#define AL_WORLD_HPP_INC

#include <math.h>
#include <string>
#include <vector>

#include "allocore/al_Allocore.hpp"
#include "alloutil/al_ControlNav.hpp"

namespace al{


/// Viewpoint within a scene
class Viewpoint{
public:

	Viewpoint(const Pose& transform = Pose::identity())
	:	mViewport(0,0,0,0),
		mParentTransform(0),
		mAnchorX(0), mAnchorY(0), mStretchX(1), mStretchY(1),
		mCamera(0)
	{}

	float anchorX() const { return mAnchorX; }
	float anchorY() const { return mAnchorY; }
	float stretchX() const { return mStretchX; }
	float stretchY() const { return mStretchY; }

	bool hasCamera() const { return 0 != mCamera; }

	const Camera& camera() const { return *mCamera; }

	const Pose* parentTransform() const { return mParentTransform; }


	const Pose& transform() const { return mTransform; }
	Pose& transform(){ return mTransform; }
	
	Pose worldTransform() const { return mParentTransform ? (*mParentTransform) * transform() : transform(); }
	
	const Viewport& viewport() const { return mViewport; }
	Viewport& viewport(){ return mViewport; }

	Viewpoint& anchor(float ax, float ay){
		mAnchorX=ax; mAnchorY=ay; return *this;
	}

	Viewpoint& stretch(float sx, float sy){
		mStretchX=sx; mStretchY=sy; return *this;
	}

	Viewpoint& camera(Camera& v){ mCamera=&v; return *this; }

	Viewpoint& parentTransform(Pose& v){ mParentTransform = &v; return *this; }

protected:
	Viewport mViewport;				// screen display region
	Pose * mParentTransform;		// parent transform, 0 if none
	Pose mTransform;				// local transform
	float mAnchorX, mAnchorY;		// viewport anchor factors relative to parent window
	float mStretchX, mStretchY;		// viewport stretch factors relative to parent window
	Camera * mCamera;				// camera; if not set, will be set to scene's default camera
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

protected:
	Viewpoints mViewpoints;
	
	struct ResizeHandler : public WindowEventHandler{
		ResizeHandler(ViewpointWindow& w_): w(w_){}
		bool onResize(int dw, int dh){
			Viewpoints::iterator iv = w.mViewpoints.begin();
			
			while(iv != w.mViewpoints.end()){
				Viewpoint& vp = **iv;

				vp.viewport().l += dw * vp.anchorX();
				vp.viewport().b += dh * vp.anchorY();
				vp.viewport().w += dw * vp.stretchX();
				vp.viewport().h += dh * vp.stretchY();

				++iv;
			}
			return true;
		}
		ViewpointWindow& w;
	};

private:
	void init(){
		add(new StandardWindowKeyControls);
		add(new ResizeHandler(*this));
	}
};



class World {
public:
	
	World(const std::string& name="")
	:	//mGraphics(new GraphicsBackendOpenGL),
		mAudioIO(128, 44100, sAudioCB, this, 2, 1),
		mAudioScene(3,2, 128),
		mName(name)
	{
		mListeners.push_back(mAudioScene.createListener(2));
	}

	const std::string& name() const { return mName; }

	const Camera& camera() const { return mCamera; }
	Camera& camera(){ return mCamera; }

	const Nav& nav() const { return mNav; }
	Nav& nav(){ return mNav; }

	World& name(const std::string& v){ mName=v; return *this; }

	void add(Actor& v){
		mActors.push_back(&v);
	}
	
	void add(ViewpointWindow& win, bool animates=false);
	
	void start(){
		mAudioIO.start();
		MainLoop::start();
	}
	
	Stereographic& stereo(){ return mStereo; }

protected:

	typedef std::vector<Actor *> Actors;
	typedef std::vector<Listener *> Listeners;
	typedef std::vector<ViewpointWindow *> Windows;
	
	Actors mActors;
	Listeners mListeners;
	Windows mWindows;
	Nav mNav;
	Camera mCamera;

	Stereographic mStereo;
	GraphicsGL mGraphics;
	AudioIO mAudioIO;
	AudioScene mAudioScene;

	std::string mName;

	static void sAudioCB(AudioIOData& io){
		World& w = io.user<World>();
		int numFrames = io.framesPerBuffer();
		
		//w.mNavMaster.velScale(4);
		//w.mNavMaster.step(io.secondsPerBuffer());
		w.mNav.smooth(0.95);
		w.mNav.step(1./4);

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

				DrawAllActors drawFunc(w.mActors, w, vp);
				w.mStereo.draw(g, cam, vp.worldTransform(), vp.viewport(), drawFunc);
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
