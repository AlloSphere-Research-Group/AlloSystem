#include "spatial/al_CoordinateFrame.hpp"

namespace al{

void Nav :: toAED(const Vec3d & to, double azimuth, double elevation, double distance) {
	
	Vec3d rel = to - mPos;
	distance = rel.mag();
	
	if (distance > QUAT_EPSILON*2) 
	{
		rel.normalize();
		
		// dot product of A & B vectors is the similarity or cosine:
		double xness = rel.dot(mUX); 
		double yness = rel.dot(mUY);
		double zness = rel.dot(mUZ);
		
		azimuth = -atan2(xness, zness);
		elevation = asin(yness);
	} else {
		// near origin; might as well assume 0 to avoid denormals
		// do not set az/el; they may already have more meaningful values
		distance = 0.0;
	}

}

void Nav :: updateUnitVectors() {
	mQuat.toVectorX(mUX);
	mQuat.toVectorY(mUY);
	mQuat.toVectorZ(mUZ);
}

void Nav :: step() {
	// accumulate orientation:
	mQuat *= mVel.mQuat;
	updateUnitVectors();
	
	// accumulate position:
	for (int i=0; i<3; i++) {
		mPos[i] += mVel.mPos[0] * mUX[i] + mVel.mPos[1] * mUY[i] + mVel.mPos[2] * mUZ[i];
	}

}

void Nav :: step(double dt) {
	// accumulate orientation:
	Quatd q2(mQuat);
	q2 *= mVel.mQuat;
	mQuat.slerp(q2, dt);
	updateUnitVectors();
	
	// accumulate position:
	for (int i=0; i<3; i++) {
		mPos[i] += dt * (mVel.mPos[0] * mUX[i] + mVel.mPos[1] * mUY[i] + mVel.mPos[2] * mUZ[i]);
	}
}


} // al::
