#ifndef INCLUDE_AL_CAMERA_HPP
#define INCLUDE_AL_CAMERA_HPP

#include "math/al_Frustum.hpp"
#include "math/al_Plane.hpp"
#include "math/al_Vec.hpp"
#include "math/al_Quat.hpp"
#include "spatial/al_CoordinateFrame.hpp"

namespace al {


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
class Camera : public NavRef {
public:

	enum StereoMode{
		Anaglyph=0,	/**< Red (left eye) / cyan (right eye) stereo */
		Active,		/**< Active quad-buffered stereo */
		Dual,		/**< Dual side-by-side stereo */
		LeftEye,	/**< Left eye only */
		RightEye	/**< Right eye only */
	};
	
	struct FrustumGL{
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

	Frustumd& computeTestFrustum();
	Frustumd& testFrustum(){ return mFrustum; }

	const Vec3d& pos() const { return vec(); }				///< Get position
	const Vec3d& vf() const { return uz(); }				///< Get forward vector
	const Vec3d& vu() const { return uy(); }				///< Get up vector
	const Vec3d& vr() const { return ux(); }				///< Get right vector
	double focalLength() const { return mFocalLength; }		///< Get focal length
	double aperture() const { return mAperture; }			///< Get vertical field of view, in degrees
	double eyeSep() const { return mEyeSep; }				///< Get eye separation
	FrustumGL frustum(double eyePos=0) const;				///< Get frustum from an eye position
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
	Vec3d mStereoOffset;					// eye offset vector (right eye; left eye is inverse), usually (1, 0, 0)
	double mLeft, mBottom, mWidth, mHeight;
	StereoMode mMode;
	Frustumd mFrustum;
	bool mStereo;

	void calcFrustum();
	void setFrustum(double sep);
	void left();
	void mid();
	void right();
};

} // al::

#endif
