#include "utAllocore.h"

int utSpatial(){

	{
		Pose a;
	}

	{
		Nav a;
		a.vec().set(0,0,0);
		a.quat().identity();
		a.vel().vec().set(1,0,0);
		a.vel().quat().identity();
		
		a.step();		assert(a.vec() == Vec3d(1.0,0,0));
		a.step();		assert(a.vec() == Vec3d(2.0,0,0));
		a.step(0.5);	assert(a.vec() == Vec3d(2.5,0,0));
		
	}

	return 0;
}
