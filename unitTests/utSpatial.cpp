#include "utAllocore.h"

int utSpatial(){

	{
		Quatd q0(2.4e-14, 0, 1, 0), q1(1, 0, -0.0004, 0);
		
		for (int i=0; i<10; i++) {
			Quatd s = Quatd::slerp(q0, q1, i*0.1);
			
			Vec3d x = s.toVectorX();
			Vec3d y = s.toVectorY();
			Vec3d z = s.toVectorZ();
			
			s.print();
			x.print();
			y.print();
			z.print();
		}
	}

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
