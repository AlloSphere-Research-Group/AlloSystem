#include "alloutil/al_App.hpp"

namespace al{

bool ViewpointWindow::onResize(int dw, int dh){
	//printf("ViewpointWindow onResize: %d %d\n", dw, dh);
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


App::App()
:	mName(""),
	mNavControl(mNav),
	mClockAnimate(0), mClockNav(0)
{
}


App::~App(){
	mAudioIO.close(); // FIXME: can happen after accessed data is freed
	
	for(unsigned i=0; i<mFacWindows.size(); ++i){
		delete mFacWindows[i];
	}
	
	if(name()!="" && oscSend().opened()) sendDisconnect();
}


static void AppAudioCB(AudioIOData& io){
	App& app = io.user<App>();
	//int numFrames = io.framesPerBuffer();
	
	//w.mNavMaster.velScale(4);
	//w.mNavMaster.step(io.secondsPerBuffer());
	if(app.clockNav() == &app.audioIO()){
		app.nav().smooth(0.95);
		app.nav().step(1./4);
	}
	//app.mListeners[0]->pose(app.nav());
	
	if(app.clockAnimate() == &app.audioIO()){
		app.onAnimate(io.secondsPerBuffer());
	}
	
	io.frame(0);
	app.onSound(app.audioIO());

	//app.mAudioScene.encode(numFrames, io.framesPerSecond());
	//app.mAudioScene.render(&io.out(0,0), numFrames);
	//printf("%f\n", io.out(0,0));
}


void App::initAudio(
	double audioRate, int audioBlockSize,
	int audioOutputs, int audioInputs
){
	mAudioIO.callback = AppAudioCB;
	mAudioIO.user(this);
	mAudioIO.framesPerSecond(audioRate);
	mAudioIO.framesPerBuffer(audioBlockSize);
	mAudioIO.channelsOut(audioOutputs);
	mAudioIO.channelsIn(audioInputs);
//	mAudioScene(3,2, audioBlockSize);
//	mListeners.push_back(mAudioScene.createListener(2));
//	mListeners[0]->speakerPos(0,0, -45);
//	mListeners[0]->speakerPos(1,1,  45);
}


ViewpointWindow * App::initWindow(
	const Window::Dim& dims,
	const std::string title,
	double fps,
	DisplayMode::t mode,
	int flags
){
	ViewpointWindow * win = new ViewpointWindow(dims, title, fps, mode);
	mFacViewpoints.push_back(Viewpoint());
	
	int last = mFacViewpoints.size()-1;
	{
		Viewpoint& vp = mFacViewpoints[last];
		vp.parentTransform(nav());
		win->add(vp);
	}
	
	mFacWindows.push_back(win);
	add(*win);
	return win;
}


void App::start(){
	if(!clockAnimate() && !mWindows.empty()){
		clockAnimate(mWindows[0]);
	}
	if(usingAudio()) mAudioIO.start();
	if(name()!="" && oscSend().opened()) sendHandshake();
	MainLoop::start();
}


App& App::add(ViewpointWindow& win){
	win.add(mNavControl);
	win.add(*new SceneWindowHandler(win, *this));
	win.add(*new SceneInputHandler(*this));
	mWindows.push_back(&win);

	// TODO: for now, first window will clock master Nav
	if(mWindows.size() == 1){
		mClockNav = mWindows[0];
	}
	return *this;
}


bool App::usingAudio() const {
	return audioIO().callback == AppAudioCB;
}



bool App::SceneWindowHandler::onFrame(){

	if(app.clockNav() == &win){
		app.nav().smooth(0.8);
		app.nav().step();
	}

	app.navDraw() = app.nav();
	app.navDraw().quat().normalize();

	if(app.clockAnimate() == &win){
		app.onAnimate(win.spfActual());
	}

	Graphics& g = app.graphics();
	g.depthTesting(true);

	struct DrawFunc : public Drawable {
		App& app;
		Viewpoint& v;
		DrawFunc(App& a, Viewpoint& v_)
			:	app(a), v(v_){}
		virtual void onDraw(Graphics& g){
			g.pushMatrix(g.MODELVIEW);
			app.onDraw(g,v);
			g.popMatrix(g.MODELVIEW);
		}
	};

	ViewpointWindow::Viewpoints::const_iterator iv = win.viewpoints().begin();

	for(; iv != win.viewpoints().end(); ++iv){
		Viewpoint& vp = *(*iv);
		
		// if no camera, set to default scene camera
		if(!vp.hasCamera()) vp.camera(app.camera());
		const Camera& cam = vp.camera();

		Color defaultClearColor = app.stereo().clearColor();
		if(!vp.hasClearColor()){
			vp.clearColor(const_cast<Color&>(app.stereo().clearColor()));
		}
		else{
			app.stereo().clearColor(vp.clearColor());
		}

		DrawFunc drawFunc(app, vp);
		app.stereo().draw(g, cam, vp.worldTransform(), vp.viewport(), drawFunc);
		app.stereo().clearColor(defaultClearColor);
	}

	return true;
}



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

