#include "allocore/spatial/al_Pose.hpp"

namespace al{

//Nav :: Nav(const Vec3d &v)
//:	Pose(v)
//{
//	updateUnitVectors();
//}
//
//void Nav :: step() {
//	// accumulate orientation:
//	mQuat = mQuat * vel().quat();
//	updateUnitVectors();
//	
//	// accumulate position:
//	for (int i=0; i<3; i++) {
//		vec()[i] += vel().vec().dot(Vec3d(ux()[i], uy()[i], uz()[i]));
//	}
//
//}
//
//void Nav :: step(double dt) {
//	// accumulate orientation:
//	Quatd q2(quat());
//	q2 *= vel().quat();
//	mQuat.slerp(q2, dt);
//	updateUnitVectors();
//	
//	// accumulate position:
//	for (int i=0; i<3; i++) {
//		vec()[i] += dt * vel().vec().dot(Vec3d(ux()[i], uy()[i], uz()[i]));
//	}
//}


void Pose :: toAED(const Vec3d& to, double& azimuth, double& elevation, double& distance) const {
	
	Vec3d rel = to - vec();
	distance = rel.mag();
	
	if (distance > quat().eps()*2) 
	{
		rel.normalize();
		
		Vec3d ux, uy, uz;
		
		quat().toVectorX(ux);
		quat().toVectorY(uy);
		quat().toVectorZ(uz);	
		
		// dot product of A & B vectors is the similarity or cosine:
		double xness = rel.dot(ux); 
		double yness = rel.dot(uy);
		double zness = rel.dot(uz);
		
		azimuth = -atan2(xness, zness);
		elevation = asin(yness);
	} else {
		// near origin; might as well assume 0 to avoid denormals
		// do not set az/el; they may already have more meaningful values
		distance = 0.0;
	}

}


} // al::
