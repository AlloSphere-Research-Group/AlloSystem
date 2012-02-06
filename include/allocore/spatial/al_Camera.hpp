#ifndef INCLUDE_AL_CAMERA_HPP
#define INCLUDE_AL_CAMERA_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


	File description:
	Optical properties for scene rendering

	File author(s):
	Wesley Smith, 2010, wesley.hoke@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include "allocore/math/al_Vec.hpp"
#include "allocore/math/al_Matrix4.hpp"
#include "allocore/math/al_Frustum.hpp"
#include "allocore/spatial/al_Pose.hpp"

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
	Camera& fovx(double v, double aspect);					///< Set horizontal field of view, in degrees
	Camera& near(double v){ mNear=v; return *this; }		///< Set frustum near plane distance
	Camera& far(double v){ mFar=v; return *this; }			///< Set frustum far plane distance
	Camera& focalLength(double v){ mFocalLength=v; return *this; } ///< Set focal length
	Camera& eyeSep(double v){ mEyeSep=v; return *this; }	///< Set eye separation

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
	/// @param[in] isStereo		Whether scene is in stereo (widens near/far planes to fit both eyes)
	void frustum(Frustumd& f, const Pose& p, double aspect, bool isStereo=false) const;
	
	/// Returns half the height of the frustum at a given depth
	
	/// To get the half-width multiply the half-height by the viewport aspect
	/// ratio.
	double heightAtDepth(double depth) const { return depth*mTanFOV; }
	
	/// Returns half the height of the frustum at the near plane
	double heightAtNear() const { return heightAtDepth(near()); }
	
	// calculate desired fovy, given the Y height of the border at a specified Z depth:
	static double getFovyForHeight(double height, double depth) {
		return 2.*M_RAD2DEG*atan(height/depth);
	}

	/// Calculate required fovy to produce a specific fovx
	/// @param[fovx] field-of-view in X axis to recreate
	/// @param[aspect] aspect ratio of viewport
	/// @return field-of-view in Y axis, usable by Camera.fovy() 
	static double getFovyForFovX(double fovx, double aspect) {
		double farW = tan(0.5*fovx*M_DEG2RAD);
		return 2.*M_RAD2DEG*atan(farW/aspect);
	}

protected:
	double mFovy;				// Camera aperture (degrees)
	double mTanFOV;				// Cached factor for computing frustum dimensions
	double mNear, mFar;			// Cutting plane distances
	double mFocalLength;		// Focal length along vd
	double mEyeSep;				// Eye separation
//	Vec3d mStereoOffset;		// eye offset vector (right eye; left eye is inverse), usually (1, 0, 0)
};



} // al::

#endif
