#ifndef INCLUDE_AL_CAMERA_HPP
#define INCLUDE_AL_CAMERA_HPP

#include "math/al_Vec.hpp"
#include "math/al_Quat.hpp"

/* temporary. eventually replace with al_Graphics: */
#if defined (__APPLE__) || defined (OSX)
	#include <OpenGL/OpenGL.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>
	#include <OpenGL/glu.h>	
#elif defined(__linux__)
	#include <GL/glew.h>
	#include <GL/gl.h>
	#include <GL/glext.h>
	#include <GL/glu.h>
	#include <time.h>	
#elif defined(WIN32)
	#include <windows.h>
	#include <gl/gl.h>
	#include <gl/glu.h>
	#pragma comment( lib, "winmm.lib")
	#pragma comment( lib, "opengl32.lib" )
	#pragma comment( lib, "glu32.lib" )	
#endif

namespace al {

///<	A refernce frame
///		Combines a Vec3d position with a Quat orientation
class Pose {
public:
	///	position in 3-space
	Vec3d mPos;
	/// orientation of reference frame as a quaternion (relative to global axes)
	Quatd mQuat;
	
	Pose() : mPos(0) {}
	
	// TODO: conversion operators for Pose->Vec3d, Pose->Quatd?
	
protected:
};

///<	A mobile refernce frame
///		A Pose that knows how to accumulate velocities
class Nav : public Pose {
public:
	///	d.pos for spatial component
	/// d.q for angular component
	Pose mVel;
	
	/// unit vectors for the current pose
	Vec3d mUX, mUY, mUZ;
	
	/// set mUX, mUY, mUZ based on mQuat:
	void updateUnitVectors();
	
	/// accumulate pose based on velocity
	void step(double dt);
	/// accumulate pose based on velocity (faster path for dt == 1)
	void step();
	
	/// scale the velocities by amt:
	void decay(double amt) {
		mVel.mPos *= amt;
		mVel.mQuat *= amt;
		updateUnitVectors();
	}
	
	void	view(const Quatd & v) { mQuat.set(v); updateUnitVectors(); }
	void	turn(const Quatd & v) { mVel.mQuat.set(v); }
	
	void	move(double x, double y, double z) { moveX(x); moveY(y); moveZ(z); }
	void	moveX(double amount) { mVel.mPos[0] = amount; }
	void	moveY(double amount) { mVel.mPos[1] = amount; }
	void	moveZ(double amount) { mVel.mPos[2] = amount; }
	
	void	push(double x, double y, double z) { pushX(x); pushY(y); pushZ(z); }
	void	pushX(double amount) { mVel.mPos[0] += amount; }
	void	pushY(double amount) { mVel.mPos[1] += amount; }
	void	pushZ(double amount) { mVel.mPos[2] += amount; }
	
	void	halt() { mVel.mQuat.reset(); mVel.mPos.set(0); }
	void	home() { mQuat.reset(); mPos.set(0); updateUnitVectors(); }
	
	// get the azimuth, elevation & distance from this to another point
	void toAED(const Vec3d & to, double azimuth, double elevation, double distance);
	
protected:
};


/*
Steve Baker's Golden Rule for OpenGL cameras:

The only functions that should be called when the glMatrixMode is set to 
GL_PROJECTION are:

glLoadIdentity - to initialise the stack.
gluPerspective/glFrustum/glOrtho/gluOrtho2 - to set the appropriate projection 
onto the stack.
You *could* use glLoadMatrix to set up your own projection matrix (if you 
understand the restrictions and consequences) - but I'm told that this can cause 
problems for some OpenGL implementations which rely on data passed to glFrustum, 
etc to determine the near and far clip planes.
*/
///<	Utility wrapper of Pose for use as a 3D camera
///		
class Camera : public Nav {
public:

	enum StereoMode{
		Anaglyph=0,	/**< Red (left eye) / cyan (right eye) stereo */
		Active,		/**< Active quad-buffered stereo */
		Dual,		/**< Dual side-by-side stereo */
		LeftEye,	/**< Left eye only */
		RightEye	/**< Right eye only */
	};
	
	struct Frustum{
		double left, right, bottom, top, near, far;
	};

	Camera(double aper=30, double nearClip=0.01, double farClip=100, double focalLen=6, double eyeSep=0.02);

	/// Set viewport dimensions in screen coordinates
	Camera& dimensions(double w, double h);
	
	/// Set viewport dimensions in screen coordinates
	Camera& dimensions(double w, double h, double x, double y);

	Camera& focalLength(double v){ mFocalLength=v; return *this; } ///< Set focal length
	Camera& aperture(double v);								///< Set vertical field of view, in degrees
	Camera& eyeSep(double v){ mEyeSep=v; return *this; }	///< Set eye separation
	Camera& near(double v){ mNear=v; return *this; }		///< Set frustum near plane distance
	Camera& far(double v){ mFar=v; return *this; }			///< Set frustum far plane distance
	Camera& mode(StereoMode v){ mMode=v; return *this; }	///< Set stereographic mode
	Camera& stereo(bool v){ mStereo=v; return *this; }		///< Set stereographic active

	const Vec3d& pos() const { return mPos; }				///< Get position
	const Vec3d& vf() const { return mUZ; }					///< Get forward vector
	const Vec3d& vu() const { return mUY; }					///< Get up vector
	const Vec3d& vr() const { return mUX; }					///< Get right vector
	double focalLength() const { return mFocalLength; }		///< Get focal length
	double aperture() const { return mAperture; }			///< Get vertical field of view, in degrees
	double eyeSep() const { return mEyeSep; }				///< Get eye separation
	Frustum frustum(double eyePos=0) const;					///< Get frustum from an eye position
	double near() const { return mNear; }					///< Get frustum near plane distance
	double nearTop() const;									///< Get frustum near plane top
	double nearRight() const;								///< Get frustum near plane right
	double far() const { return mFar; }						///< Get frustum far plane distance
	double farTop() const;									///< Get frustum far plane top
	double farRight() const;								///< Get frustum far plane right
	StereoMode mode() const { return mMode; }			///< Get stereographic mode
	double ratio() const { return mRatio; }					///< Get aspect ratio (width/height)
	bool stereo() const { return mStereo; }					///< Get stereographic active
	
	void setLookAt(double tx, double ty, double tz);

	void begin(double w, double h, double x=0, double y=0);
	void end();

	int eyeEnd() const;
	int eyeStart() const;
	void setEye(int i);

protected:
	double mFocalLength;		// Focal length along vd
	double mAperture;			// Camera aperture (degrees)
	double mEyeSep;				// Eye separation
	double mNear, mFar;			// Cutting plane distances
	double mRatio;				// frustum aspect ratio 
	double mNearTop;			// frustum near plane top
	double mFarTop;				// frustum far plane top
	double mTanFOV;
	double mZoom;
	double mNearOverFocalLength;
//	double ndfl;				// ???
	Vec3d mStereoOffset;					// eye offset vector (right eye; left eye is inverse), usually (1, 0, 0)
	double mLeft, mBottom, mWidth, mHeight;
	StereoMode mMode;
	bool mStereo;

	void setFrustum(double sep);
	void left();
	void mid();
	void right();
};

//class Plane{
//public:
//
//	Vec3d normal,point;
//	double d;
//
//	Plane(const Vec3d &v1, const Vec3d &v2, const Vec3d &v3);
//
//	void set3Points(const Vec3d &v1, const Vec3d &v2, const Vec3d &v3);
//	void setNormalAndPoint(const Vec3d &normal, const Vec3d &point);
//	void setCoefficients(double a, double b, double c, double d);
//	double distance(const Vec3d &p) const;
//};
//class Camera{
//public:
//
//	struct Frustum{
//		double left, right, bottom, top, near, far;
//	};
//
//	Camera(double focalLen=6, double aper=30, double eyeSep=0.02, double nearCut=0.01, double farCut=100);
//};

//Plane::Plane(const Vec3d &v1, const Vec3d &v2, const Vec3d &v3){
//	set3Points(v1,v2,v3);
//}
//
//void Plane::set3Points(const Vec3d &v1, const Vec3d &v2, const Vec3d &v3){
//	Vec3d aux1, aux2;
//
//	aux1 = v1 - v2;
//	aux2 = v3 - v2;
//
//	normal = aux2.cross(aux1);
//
//	normal.normalize();
//	point = v2;
//	d = -(normal.dot(point));
//}
//
//void Plane::setNormalAndPoint(const Vec3d &nrm, const Vec3d &point){
//	normal = nrm.sgn();
//	d = -(normal.dot(point));
//}
//
//void Plane::setCoefficients(double a, double b, double c, double d_){
//	normal(a,b,c);
//	double l = normal.norm();
//	normal(a/l,b/l,c/l);
//	d = d_/l;
//}
//
//double Plane::distance(const Vec3d &p) const{
//	return (d + normal.dot(p));
//}


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
	mQuat.rotateby(mVel.mQuat);
	updateUnitVectors();
	
	// accumulate position:
	for (int i=0; i<3; i++) {
		mPos[i] += mVel.mPos[0] * mUX[i] + mVel.mPos[1] * mUY[i] + mVel.mPos[2] * mUZ[i];
	}

}

void Nav :: step(double dt) {
	// accumulate orientation:
	Quatd q2(mQuat);
	q2.rotateby(mVel.mQuat);
	mQuat.slerp(q2, dt);
	updateUnitVectors();
	
	// accumulate position:
	for (int i=0; i<3; i++) {
		mPos[i] += dt * (mVel.mPos[0] * mUX[i] + mVel.mPos[1] * mUY[i] + mVel.mPos[2] * mUZ[i]);
	}
}






Camera :: Camera(double aper, double nearClip, double farClip, double focalLen, double eyeSep)
:	mFocalLength(focalLen), mEyeSep(eyeSep), mNear(nearClip), mFar(farClip), 
	mStereo(false), mMode(Anaglyph), 
	mZoom(0)
{
	aperture(aper);
}


void Camera::setLookAt(double tx, double ty, double tz){

	//gl.matrixMode(gl.ModelView); 
	glMatrixMode(GL_MODELVIEW);
	//gl.identity();
	glLoadIdentity();
	
	//gl.lookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ)
	gluLookAt(tx + mPos[0], ty + mPos[1], tz + mPos[2],
			tx + (mPos[0] + mUZ[0]*mFocalLength),
			ty + (mPos[1] + mUZ[1]*mFocalLength),
			tz + (mPos[2] + mUZ[2]*mFocalLength),
			mUY[0], mUY[1], mUY[2]);
}


Camera& Camera::aperture(double v){
	mAperture=v;
	static double const tanCoef = 0.01745329252*0.5;	// degree-to-radian over /2
	mTanFOV = tan(tanCoef * aperture());
	return *this;
}

Camera& Camera::dimensions(double w, double h){ return dimensions(w,h,mLeft,mBottom); }

Camera& Camera::dimensions(double w, double h, double x, double y){
	mLeft=x; mBottom=y; mWidth=w; mHeight=h;
	return *this;
}
	
double Camera::nearTop() const { return mNearTop; }
double Camera::nearRight() const { return nearTop() * ratio(); }
double Camera::farTop() const { return mFarTop; }
double Camera::farRight() const { return farTop() * ratio(); }


void Camera::begin(double w, double h, double x, double y){
	dimensions(w,h,x,y);
	
	mRatio = mWidth / (mHeight>0 ? mHeight : 1e-10);

	if(stereo() && (mode() == Dual)) 
		mRatio /= 2;		// assume screen space is twice the width of the display

	double zm = pow(2, mZoom);
	mNearTop = near() * mTanFOV * zm;
	mFarTop = far() * mTanFOV * zm;
	mNearOverFocalLength = near() / (focalLength() * 2.) * zm;
	
	// Derive the eye offset vector
	mStereoOffset = mUX * (eyeSep()*0.5);
	
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	//push(Projection);
	//push(ModelView);
	glMatrixMode(GL_PROJECTION); glPushMatrix(); //pop(Projection);
	glMatrixMode(GL_MODELVIEW); glPushMatrix(); //pop(ModelView);
	
	//enable(ScissorTest);
	glEnable(GL_SCISSOR_TEST);
	//scissor(mLeft,mBottom, mWidth,mHeight);
	glScissor(mLeft, mBottom, mWidth, mHeight);
	
	if(mode() == Dual){
		//drawBuffer(Back);
		glDrawBuffer(GL_BACK);
		//clear(ColorBufferBit | DepthBufferBit);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void Camera::end(){
	if(mode() == Anaglyph) glColorMask(1, 1, 1, 1); //colorMask(1,1,1,1);
	//disable(ScissorTest);
	glDisable(GL_SCISSOR_TEST);
	//popAttrib();
	glPopAttrib();
	glMatrixMode(GL_PROJECTION); glPopMatrix(); //pop(Projection);
	glMatrixMode(GL_MODELVIEW); glPopMatrix(); //pop(ModelView);
}

int Camera::eyeStart() const {
	if(mode() == RightEye) return 1;
	else return 0;
}

int Camera::eyeEnd() const {
	if(mode() == LeftEye) return 1;
	else return 2;
}

Camera::Frustum Camera::frustum(double sep) const{
	Frustum f;
	double sep_ndfl = sep*mNearOverFocalLength;
	
	f.left	=-nearRight() + sep_ndfl;
	f.right	= nearRight() + sep_ndfl;
	f.bottom=-nearTop();
	f.top	= nearTop();
	f.near	= near();
	f.far	= far();

	return f;
}

void Camera::setEye(int i){ stereo() ? (i ? right() : left()) : mid(); }

void Camera::setFrustum(double sep){
	//matrixMode(Projection); identity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	Frustum f = frustum(sep);

	//frustum(l,r,b,t near, far)
	//glw::frustum(f.left, f.right, f.bottom, f.top, f.near, f.far);
	glFrustum(f.left, f.right, f.bottom, f.top, f.near, f.far);
}



void Camera::mid(){
	//clear(ColorBufferBit | DepthBufferBit);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//viewport(mLeft,mBottom, mWidth,mHeight);
	glViewport(mLeft, mBottom, mWidth, mHeight);
	setFrustum(0);
	setLookAt(0,0,0);
}

void Camera::left(){
	
	setFrustum(eyeSep());
	
		switch(mMode){
		case Active:
			//drawBuffer(BackLeft);
			glDrawBuffer(GL_BACK_LEFT);
			//clear(ColorBufferBit | DepthBufferBit);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//viewport(mLeft,mBottom, mWidth,mHeight);
			glViewport(mLeft, mBottom, mWidth, mHeight);
			break;

		case Anaglyph:
			//drawBuffer(Back);
			glDrawBuffer(GL_BACK);
			//clear(DepthBufferBit);
			glClear(GL_DEPTH_BUFFER_BIT);
			glColorMask(1, 0, 0, 0);
			//colorMask(1, 0, 0, 0);
			//viewport(mLeft,mBottom, mWidth,mHeight);
			glViewport(mLeft, mBottom, mWidth, mHeight);
			break;

		case Dual:
			//viewport(mLeft, mBottom, mWidth/2, mHeight);
			glViewport(mLeft, mBottom, mWidth/2, mHeight);
			break;
			
		default:
			//drawBuffer(Back);
			glDrawBuffer(GL_BACK);
			//clear(ColorBufferBit | DepthBufferBit);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//viewport(mLeft,mBottom, mWidth,mHeight);
			glViewport(mLeft, mBottom, mWidth, mHeight);
	};
	
	setLookAt(-mStereoOffset[0], -mStereoOffset[1], -mStereoOffset[2]);
}


void Camera::right(){
	
	setFrustum(-eyeSep());

	switch(mMode){
		case Active:
			//drawBuffer(BackRight);
			glDrawBuffer(GL_BACK_RIGHT);
			//clear(ColorBufferBit | DepthBufferBit);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//viewport(mLeft,mBottom, mWidth,mHeight);
			glViewport(mLeft, mBottom, mWidth, mHeight);
			break;

		case Anaglyph:
			//drawBuffer(Back);
			glDrawBuffer(GL_BACK);
			//clear(DepthBufferBit);
			glClear(GL_DEPTH_BUFFER_BIT);
			glColorMask(0, 1, 1, 0);
			//colorMask(0,1,1,0);
			//viewport(mLeft,mBottom, mWidth,mHeight);
			glViewport(mLeft, mBottom, mWidth, mHeight);
			break;

		case Dual:
			//viewport(mLeft + mWidth/2, mBottom, mWidth/2, mHeight);
			glViewport(mLeft + mWidth/2, mBottom, mWidth/2, mHeight);
			break;
			
		default:
			//drawBuffer(Back);
			glDrawBuffer(GL_BACK);
			//clear(ColorBufferBit | DepthBufferBit);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//viewport(mLeft,mBottom, mWidth,mHeight);
			glViewport(mLeft, mBottom, mWidth, mHeight);
	};

	setLookAt(mStereoOffset[0], mStereoOffset[1], mStereoOffset[2]);
}

} // al::

#endif
