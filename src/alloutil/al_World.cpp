#include "alloutil/al_World.hpp"

namespace al{

World::World(
	const std::string& name,
	double audioRate, int audioBlockSize,
	int audioOutputs, int audioInputs
)
:	//mGraphics(new GraphicsBackendOpenGL),
	mAudioIO(audioBlockSize, audioRate, sAudioCB, this, audioOutputs, audioInputs),
	mAudioScene(3,2, audioBlockSize),
	mName(name),
	mNavControl(mNav),
	mAutoStepNav(true)
{
	mListeners.push_back(mAudioScene.createListener(2));
	mListeners[0]->speakerPos(0,0, -45);
	mListeners[0]->speakerPos(1,1,  45);
}

World::~World(){
	mAudioIO.close();
	if(name()!="" && oscSend().opened()) sendDisconnect();
}

World& World::add(ViewpointWindow& win, bool animates){
//	win.add(new NavInputControl(mNav));
	win.add(mNavControl);
	win.add(*new DrawActors(win, *this));
	win.add(*new SceneInputControl(*this));
	if(animates) win.add(*new AnimateActors(*this));
	mWindows.push_back(&win);
	return *this;
}

} // al::

