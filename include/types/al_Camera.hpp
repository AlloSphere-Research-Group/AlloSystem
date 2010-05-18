#ifndef INCLUDE_AL_CAMERA_HPP
#define INCLUDE_AL_CAMERA_HPP

#include "math/al_Plane.hpp"
#include "math/al_Vec.hpp"
#include "math/al_Quat.hpp"


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
	
	void	halt() { mVel.mQuat.identity(); mVel.mPos.set(0); }
	void	home() { mQuat.identity(); mPos.set(0); updateUnitVectors(); }
	
	// get the azimuth, elevation & distance from this to another point
	void toAED(const Vec3d & to, double azimuth, double elevation, double distance);
	
protected:
};



/// Frustum geometry
class FrustumG{
public:

	enum{ OUTSIDE, INTERSECT, INSIDE };

	Plane<double> pl[6];
	Vec3d ntl,ntr,nbl,nbr,ftl,ftr,fbl,fbr;	// frustrum vertices
	float mNear, mFar, mRatio, mAngle, mTanFOV;
	float nw,nh,fw,fh;						// near and far plane dimensions

	FrustumG(){}
	~FrustumG(){}

	/// Set
	void setCamInternals(float angle, float ratio, float nearDist, float farDist);
	
	/// Set from camera position, look, and up vectors
	void setCamDef(const Vec3d& pos, const Vec3d& look, const Vec3d& up);


	int pointInFrustum(const Vec3d& p) const;
	int sphereInFrustum(const Vec3d& p, float raio) const;
	//int boxInFrustum(AABox &b);

	void drawPoints();
	void drawLines();
	void drawPlanes();
	void drawNormals();

private:
	enum{ TOP=0, BOTTOM, LEFT, RIGHT, NEARP, FARP };
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
	Camera& zoom(double v){ mZoom=v; return *this; }		///< Set zoom amount

	FrustumG& computeTestFrustum();
	FrustumG& testFrustum(){ return mFrustumG; }

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
	StereoMode mode() const { return mMode; }				///< Get stereographic mode
	double ratio() const { return mRatio; }					///< Get aspect ratio (width/height)
	bool stereo() const { return mStereo; }					///< Get stereographic active
	double zoom() const { return mZoom; }					///< Get zoom amount
	
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
	FrustumG mFrustumG;
	bool mStereo;

	void calcFrustum();
	void setFrustum(double sep);
	void left();
	void mid();
	void right();
};

} // al::

#endif
