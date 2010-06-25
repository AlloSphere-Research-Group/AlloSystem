#include "graphics/al_Context.hpp"

using namespace al;

ViewPort :: ViewPort(double eyeSep) 
:	mEyeSep(eyeSep), mRatio(1), mCamera(Camera::defaultCamera())
{
	dimensions(4,3,0,0);
}

ViewPort :: ~ViewPort() {

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

void ViewPort::setLookAt(double tx, double ty, double tz) {

	//gl.matrixMode(gl.ModelView); 
	glMatrixMode(GL_MODELVIEW);
	//gl.identity();
	glLoadIdentity();
	
	const Vec3d& vec = mCamera->vec();
	const Vec3d& uy = mCamera->uy();
	const Vec3d& uz = mCamera->uz();
	const double focalLength = mCamera->focalLength();
	
	//gl.lookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ)
	gluLookAt(tx + vec[0], ty + vec[1], tz + vec[2],
			tx + (vec[0] + uz[0]*focalLength),
			ty + (vec[1] + uz[1]*focalLength),
			tz + (vec[2] + uz[2]*focalLength),
			uy[0], uy[1], uy[2]);
}