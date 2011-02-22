#include "alloutil/al_World.hpp"

namespace al{

void World::add(ViewpointWindow& win, bool animates){
	win.add(new NavInputControl(mNav));
	win.add(new DrawActors(win, *this));
	win.add(new SceneInputControl(*this));
	if(animates) win.add(new AnimateActors(*this));
	mWindows.push_back(&win);
}

} // al::

