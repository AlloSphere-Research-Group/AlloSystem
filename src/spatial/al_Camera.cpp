#include "spatial/al_Camera.hpp"


namespace al{

Camera :: Camera(
	double fovy, 
	double nearClip, 
	double farClip, 
	double focalLength, 
	double eyeSep,
	double aspectRatio
)
:	Nav(Vec3d(0, 0, -4)),
	mFovy(fovy),
	mNear(nearClip),
	mFar(farClip),
	mFocalLength(focalLength),
	mEyeSep(eyeSep),
	mAspectRatio(aspectRatio),
	mAutoEyeSep(true),
	mZoom(0)
{}


Matrix4d Camera::modelViewMatrix(Eye e) {
	switch(e) {
		case RIGHT: 
			return Matrix4d::lookAtOffAxis(ux(), uy(), uz(), pos(), mEyeSep);
		case LEFT: 
			return Matrix4d::lookAtOffAxis(ux(), uy(), uz(), pos(), -mEyeSep);
		case MONO:
		default: 
			return Matrix4d::lookAt(ux(), uy(), uz(), pos());
	}
}

double Camera::height(double distance) {
	return 2*distance * tan(mFovy*M_DEG2RAD*0.5);
}

Matrix4d Camera::projectionMatrix(Eye e) {
	if(mAutoEyeSep) {
		mEyeSep = mFocalLength/30.;
	}
	switch(e) {
		case RIGHT: 
			return Matrix4d::perspectiveOffAxis(mFovy, mAspectRatio, mNear, mFar, mEyeSep, mFocalLength);
		case LEFT: 
			return Matrix4d::perspectiveOffAxis(mFovy, mAspectRatio, mNear, mFar, -mEyeSep, mFocalLength);
		case MONO:
		default:
			return Matrix4d::perspective(
				mFovy, mAspectRatio, mNear, mFar
			);
	}
}

/*
void Camera::calcFrustum(){
	mRatio = mWidth / (mHeight>0 ? mHeight : 1e-10);

	if(stereo() && (mode() == Dual)) 
		mRatio /= 2;		// assume screen space is twice the width of the display

	double zm = pow(2, -zoom());
	mNearTop = near() * mTanFOV * zm;
	mFarTop = far() * mTanFOV * zm;
	mNearOverFocalLength = near() / (focalLength() * 2.) * zm;

	// Derive the eye offset vector
	mStereoOffset = mUX * (eyeSep()*0.5);
}
*/

/*
Camera& Camera::aperture(double v){
	mFOVY=v;
	static double const tanCoef = 0.01745329252*0.5;	// degree-to-radian over /2
	mTanFOV = tan(tanCoef * aperture());
	return *this;
}
*/

} // al::
