#include "alloutil/al_World.hpp"

namespace al{

void World::add(AlloView& v, bool animates){
	v.add(new NavInputControl(&mNav));
	v.add(new DrawActors(v, *this));
	v.add(new SceneInputControl(*this));
	if(animates) v.add(new AnimateActors(*this));
	mViewports.push_back(&v);
}

} // al::

