#include "utAllocore.h"

#include "catch.hpp"

TEST_CASE( "Spatial", "[spatial]" ) {

	{
		Pose a;
	}

	{
		Nav a;
		a.smooth(0);
		a.home();
		a.move(1,0,0);

		a.step();		REQUIRE(a.vec() == Vec3d(1.0,0,0));
		a.step();		REQUIRE(a.vec() == Vec3d(2.0,0,0));
		a.step(0.5);	REQUIRE(a.vec() == Vec3d(2.5,0,0));
	}
}
