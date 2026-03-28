#ifndef INC_AL_LENS_HPP
#define INC_AL_LENS_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Optical properties for scene rendering

	Author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Wesley Smith, 2010, wesley.hoke@gmail.com
*/

namespace al {

template <class T> class Frustum;
class Pose;

/// Stores optics settings important for rendering
/// @ingroup allocore
class Lens {
public:

	/// @param[in] fovy			vertical field of view, in degrees
	/// @param[in] nearClip		frustum near plane distance
	/// @param[in] farClip		frustum far plane distance
	/// @param[in] focalLength	focal length, distance of zero parallax
	/// @param[in] eyeSep		eye separation amount
	Lens(
		double fovy=30,
		double nearClip=0.1,
		double farClip=100,
		double focalLength=6,
		double eyeSep=0.02
	);

	// setters
	Lens& fovy(double v);									///< Set vertical field of view, in degrees
	Lens& fovx(double v, double aspect);					///< Set horizontal field of view, in degrees
	Lens& near(double v){ mNear=v; return *this; }			///< Set frustum near plane distance
	Lens& far(double v){ mFar=v; return *this; }			///< Set frustum far plane distance
	Lens& focalLength(double v){ mFocalLength=v; return *this; } ///< Set focal length
	Lens& eyeSep(double v){ mEyeSep=v; return *this; }		///< Set eye separation

	double fovy() const { return mFovy; }					///< Get vertical field of view, in degrees
	double near() const { return mNear; }					///< Get frustum near plane distance
	double far() const { return mFar; }						///< Get frustum far plane distance
	double focalLength() const { return mFocalLength; }		///< Get focal length
	double eyeSep() const { return mEyeSep; }				///< Get eye separation
	double eyeSepAuto() const { return focalLength()/30.0; }///< Get automatic inter-ocular distance

	/// Get test frustum

	/// @param[in] f			The resulting test frustum
	/// @param[in] p			A position and orientation in world coordinates
	/// @param[in] aspect		Aspect ratio (width/height) of viewport
	void frustum(Frustum<double>& f, const Pose& p, double aspect) const;

	/// Returns half the height of the frustum at a given depth

	/// To get the half-width multiply the half-height by the viewport aspect
	/// ratio.
	double heightAtDepth(double depth) const { return depth*mTanFOV; }

	/// Returns half the height of the frustum at the near plane
	double heightAtNear() const { return heightAtDepth(near()); }


	/// Calculate desired fovy, given the Y height of the border at a specified Z depth:
	static double getFovyForHeight(double height, double depth);

	/// Calculate required fovy to produce a specific fovx

	/// @param[in] fovx		field-of-view in X axis to recreate
	/// @param[in] aspect	aspect ratio of viewport
	/// @return field-of-view in Y axis, usable by Lens::fovy()
	static double getFovyForFovX(double fovx, double aspect);

protected:
	double mFovy;				// Lens aperture (degrees)
	double mTanFOV;				// Cached factor for computing frustum dimensions
	double mNear, mFar;			// Cutting plane distances
	double mFocalLength;		// Focal length along vd
	double mEyeSep;				// Eye separation
};

} // al::
#endif
