#include "graphics/al_Context.hpp"

using namespace al;

ViewPort :: ViewPort(double eyeSep) 
:	mEyeSep(eyeSep), mRatio(1)
{
	dimensions(4,3,0,0);
}

ViewPort& ViewPort::dimensions(double w, double h){ return dimensions(w,h,mLeft,mBottom); }

ViewPort& ViewPort::dimensions(double w, double h, double x, double y){
	mLeft=x; mBottom=y; mWidth=w; mHeight=h;
	return *this;
}

//void ViewPort::calcFrustum(){
//	mRatio = mWidth / (mHeight>0 ? mHeight : 1e-10);
//
//	if(stereo() && (mode() == Dual)) 
//		mRatio /= 2;		// assume screen space is twice the width of the display
//
//	double zm = pow(2, -zoom());
//	mNearTop = near() * mTanFOV * zm;
//	mFarTop = far() * mTanFOV * zm;
//	mNearOverFocalLength = near() / (focalLength() * 2.) * zm;
//
//	// Derive the eye offset vector
//	mStereoOffset = mUX * (eyeSep()*0.5);
//
//	// TODO: this is redundant
//	mFrustum.setCamInternals(aperture(), ratio(), near(), far());
//}
