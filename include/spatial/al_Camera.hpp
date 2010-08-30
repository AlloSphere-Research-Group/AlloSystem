#ifndef INCLUDE_AL_CAMERA_HPP
#define INCLUDE_AL_CAMERA_HPP

#include "math/al_Frustum.hpp"
#include "math/al_Plane.hpp"
#include "math/al_Vec.hpp"
#include "math/al_Quat.hpp"
#include "math/al_Matrix4.hpp"
#include "spatial/al_CoordinateFrame.hpp"


namespace al {

///<	Utility wrapper of Pose for use as a 3D camera
///
class Camera : public Nav {
public:

	enum Eye{
		MONO = 0,
		LEFT,
		RIGHT
	};


	Camera(
		double fovy=30,
		double nearClip=0.01,
		double farClip=100,
		double focalLength=6,
		double eyeSep=0.02,
		double aspectRatio=1.3333
	);

	virtual ~Camera() {}

	// setters
	Camera& fovy(double v) { mFovy =v; return *this; }									///< Set vertical field of view, in degrees
	Camera& near(double v){ mNear=v; return *this; }		///< Set frustum near plane distance
	Camera& far(double v){ mFar=v; return *this; }			///< Set frustum far plane distance
	Camera& focalLength(double v){ mFocalLength=v; return *this; } ///< Set focal length
	Camera& eyeSep(double v){ mEyeSep=v; return *this; }	///< Set eye separation
	Camera& aspectRatio(double v){ mAspectRatio=v; return *this; }	///< Set the aspect ratio
	Camera& autoEyeSep(bool v){ mAutoEyeSep=v; return *this; }		///< Set auto eye separation amount
	Camera& zoom(double v){ mZoom=v; return *this; }		///< Set zoom amount

	// aliases for Nav
	const Vec3d& vf() const { return uz(); }				///< Get forward vector
	const Vec3d& vu() const { return uy(); }				///< Get up vector
	const Vec3d& vr() const { return ux(); }				///< Get right vector


	double fovy() const { return mFovy; }					///< Get frustum near plane distance
	double near() const { return mNear; }					///< Get frustum near plane distance
	double far() const { return mFar; }						///< Get frustum far plane distance
	double focalLength() const { return mFocalLength; }		///< Get focal length
	double eyeSep() const { return mEyeSep; }				///< Get eye separation
	double aspectRatio() const { return mAspectRatio; }		///< Get aspect ratio (width/height)
	bool autoEyeSep() const { return mAutoEyeSep; }			///< Get auto eye separation
	double zoom() const { return mZoom; }					///< Get zoom amount
	double IOD() const { return eyeSep() * focalLength()/30.0; }	///< Get automatic inter-ocular distance

	Matrix4d modelViewMatrix(Eye e=MONO);
	Matrix4d projectionMatrix(Eye e=MONO);

	double height(double distance);							///< Height of view at distance from camera


protected:
	double mFovy;				// Camera aperture (degrees)
	double mNear, mFar;			// Cutting plane distances
	double mFocalLength;		// Focal length along vd
	double mEyeSep;				// Eye separation
	double mAspectRatio;		// frustum aspect ratio
	bool mAutoEyeSep;			// auto calculate the eye separation

	double mZoom;

	Vec3d mStereoOffset;					// eye offset vector (right eye; left eye is inverse), usually (1, 0, 0)
};

} // al::

#endif
