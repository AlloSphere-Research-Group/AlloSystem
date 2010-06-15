#include "utAllocore.h"

int utSpatial(){

	{
		Pose a;
	}

	{
		Nav a;
		a.mPos.set(0,0,0);
		a.mQuat.identity();
		a.mVel.mPos.set(0,0,0);
		a.mQuat.identity();
	}

	return 0;
}
