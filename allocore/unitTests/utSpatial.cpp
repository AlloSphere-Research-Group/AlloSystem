#include "utAllocore.h"

int utSpatial(){

	{
		Pose a;
	}

	{
		Nav a;
		a.smooth(0);
		a.home();
		a.move(1,0,0);
		
		a.step();		assert(a.vec() == Vec3d(1.0,0,0));
		a.step();		assert(a.vec() == Vec3d(2.0,0,0));
		a.step(0.5);	assert(a.vec() == Vec3d(2.5,0,0));
	}

	return 0;
}
