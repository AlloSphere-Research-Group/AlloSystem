#ifndef INCLUDE_AL_UTIL_FPS_HPP
#define INCLUDE_AL_UTIL_FPS_HPP

#include "allocore/system/al_Time.hpp"

namespace al {
	
class FPS {
public:
	
	FPS() : lastFrameT(al_time()), dt(0.1), frame(0), fps_measured(10) { birth = lastFrameT; }
	
	void onFrame() {
		// timing:
		al_sec f = al_time();
		dt = f-lastFrameT;
		lastFrameT = f;
		fps_measured = fps_measured + 0.1 * ((1./dt) - fps_measured);
		frame++;
	}
	
	// get current time:
	al_sec now() { return al_time() - birth; }
	
	double fps() { return fps_measured; }
	
	al_sec birth, lastFrameT, dt;
	unsigned frame;
	double fps_measured;
};

} //al::

#endif
