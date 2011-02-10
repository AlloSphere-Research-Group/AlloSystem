#ifndef INCLUDE_AL_CAMERA_HPP
#define INCLUDE_AL_CAMERA_HPP

#include "allocore/math/al_Vec.hpp"
#include "allocore/math/al_Matrix4.hpp"
#include "allocore/math/al_Frustum.hpp"
#include "allocore/spatial/al_CoordinateFrame.hpp"

namespace al {

/// Camera stores optics settings important for rendering
class Camera {
public:

	///
	Camera(
		double fovy=30,
		double nearClip=0.1,
		double farClip=100,
		double focalLength=6,
		double eyeSep=0.02
	);

	// setters
	Camera& fovy(double v);									///< Set vertical field of view, in degrees
	Camera& near(double v){ mNear=v; return *this; }		///< Set frustum near plane distance
	Camera& far(double v){ mFar=v; return *this; }			///< Set frustum far plane distance
	Camera& focalLength(double v){ mFocalLength=v; return *this; } ///< Set focal length
	Camera& eyeSep(double v){ mEyeSep=v; return *this; }	///< Set eye separation
	Camera& zoom(double v){ mZoom=v; return *this; }		///< Set zoom amount

	double fovy() const { return mFovy; }					///< Get vertical field of view, in degrees
	double near() const { return mNear; }					///< Get frustum near plane distance
	double far() const { return mFar; }						///< Get frustum far plane distance
	double focalLength() const { return mFocalLength; }		///< Get focal length
	double eyeSep() const { return mEyeSep; }				///< Get eye separation
	double zoom() const { return mZoom; }					///< Get zoom amount
	double IOD() const { return eyeSep() * focalLength()/30.0; }	///< Get automatic inter-ocular distance

	/// Get test frustum
	
	/// @param[in] f			The resulting test frustum
	/// @param[in] p			A position and orientation in world coordinates
	/// @param[in] aspect		Aspect ratio (width/height) of viewport
	/// @param[in] isStereo		Whether scene is in stereo (widens near/far planes to fit both eyes)
	void frustum(Frustumd& f, const Pose& p, double aspect, bool isStereo=false) const;
	
	/// Returns half the height of the frustum at a given depth
	
	/// To get the half-width multiply the half-height by the viewport aspect
	/// ratio.
	double heightAtDepth(double depth) const { return depth*mTanFOV; }
	
	// calculate desired fovy, given the Y height of the border at a specified Z depth:
	static double getFovyForHeight(double height, double depth) {
		return 2.*M_RAD2DEG*atan(height/depth);
	}

protected:
	double mFovy;				// Camera aperture (degrees)
	double mTanFOV;				// Cached factor for computing frustum dimensions
	double mNear, mFar;			// Cutting plane distances
	double mFocalLength;		// Focal length along vd
	double mEyeSep;				// Eye separation
	double mZoom;
//	Vec3d mStereoOffset;		// eye offset vector (right eye; left eye is inverse), usually (1, 0, 0)
};


} // al::

#endif
