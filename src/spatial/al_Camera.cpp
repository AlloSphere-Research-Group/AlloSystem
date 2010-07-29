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
		case RIGHT: {
			Vec3d right = ux();
			Vec3d eye_pos = pos()+right*0.5;
			
			return Matrix4d(
				right,
				uy(),
				uz(),
				eye_pos
			);
		}
		
		case LEFT: {
			Vec3d right = ux();
			Vec3d eye_pos = pos()-right*0.5;
			
			return Matrix4d(
				right,
				uy(),
				uz(),
				eye_pos
			);
		}
		
		case MONO:
		default:
			return Matrix4d(
				ux(),
				uy(),
				uz(),
				pos()
			);
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
		case RIGHT: {
			double D = 0.5*mEyeSep*mNear/mFocalLength;
			
			double top = height(mNear)*0.5;
			double bottom = -top;
			double left = -mAspectRatio*top - D;
			double right = mAspectRatio*top - D;
			
			return Matrix4d::perspective(
				left, right, bottom, top, mNear, mFar
			);
		}
		
		case LEFT: {
			double D = 0.5*mEyeSep*mNear/mFocalLength;
			
			double top = height(mNear)*0.5;
			double bottom = -top;
			double left = -mAspectRatio*top + D;
			double right = mAspectRatio*top + D;
			
			return Matrix4d::perspective(
				left, right, bottom, top, mNear, mFar
			);
		}
		
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



void Camera::mono(){
	//clear(ColorBufferBit | DepthBufferBit);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//viewport(mLeft,mBottom, mWidth,mHeight);
	glViewport(mLeft, mBottom, mWidth, mHeight);
	setFrustum(0);
	setLookAt(0,0,0);
}
*/

/*
void Camera::setLookAt(double tx, double ty, double tz){

	//gl.matrixMode(gl.ModelView); 
	glMatrixMode(GL_MODELVIEW);
	//gl.identity();
	glLoadIdentity();
	
	//gl.lookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ)
	gluLookAt(tx + vec()[0], ty + vec()[1], tz + vec()[2],
			tx + (vec()[0] + uz()[0]*mFocalLength),
			ty + (vec()[1] + uz()[1]*mFocalLength),
			tz + (vec()[2] + uz()[2]*mFocalLength),
			uy()[0], uy()[1], uy()[2]);
}
*/

} // al::
