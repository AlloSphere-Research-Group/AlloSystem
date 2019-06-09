#ifndef INCLUDE_AL_GRAPHICS_STEREOGRAPHIC_HPP
#define INCLUDE_AL_GRAPHICS_STEREOGRAPHIC_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	Utilities for rendering Graphics in various stereographic modes

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Lens.hpp"
#include "allocore/spatial/al_Pose.hpp"
#include "allocore/types/al_Color.hpp"

namespace al{

///	Higher-level utility class to manage various stereo rendering techniques
///
/// @ingroup allocore
class Stereographic {
public:

	/// Stereographic mode
	enum StereoMode{
		ANAGLYPH=0,		/**< Red (left eye) / cyan (right eye) stereo */
		ACTIVE,			/**< Active quad-buffered stereo */
		DUAL,			/**< Dual side-by-side stereo */
		LEFT_EYE,		/**< Left eye only */
		RIGHT_EYE		/**< Right eye only */
	};

	/// Anaglyph mode
	enum AnaglyphMode {
		RED_CYAN,		/**< Left eye red, right eye cyan (the norm) */
		RED_BLUE,		/**< Left eye red, right eye blue */
		RED_GREEN,		/**< Left eye red, right eye green */
		CYAN_RED,		/**< Left eye cyan, right eye red */
		BLUE_RED,		/**< Left eye blue, right eye red */
		GREEN_RED		/**< Left eye green, right eye red */
	};


	Stereographic();


	/// Draw the scene according to the stored stereographic mode

	/// @param[in] g		graphics interface
	/// @param[in] lens		local viewing frustum
	/// @param[in] pose		viewer position and orientation
	/// @param[in] vp		region of screen to render to
	/// @param[in] draw		function object with drawing commands
	/// @param[in] clear	whether to clear the color/depth buffers
	/// @param[in] pixelaspect	additional aspect multipler (for non-square pixels)
	void draw			(Graphics& g, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true, double pixelaspect=1.);

	/// Draw mono
	void drawMono		(Graphics& g, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true, double pixelaspect=1.);

	/// Draw active stereo
	void drawActive		(Graphics& g, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true, double pixelaspect=1.);

	/// Draw anaglyph stereo
	void drawAnaglyph	(Graphics& g, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true, double pixelaspect=1.);

	/// Draw dual (side-by-side, left-right) stereo
	void drawDual		(Graphics& g, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true, double pixelaspect=1.);

	/// Draw left eye only
	void drawLeft		(Graphics& g, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true, double pixelaspect=1.);

	/// Draw right eye only
	void drawRight		(Graphics& g, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true, double pixelaspect=1.);

	/// Draw blue line for active stereo sync (for those projectors that need it)

	/// Add this call at the end of rendering just before the swap buffers call.
	///
	void drawBlueLine(double window_width, double window_height);


	/// Set background clear color
	Stereographic& clearColor(const Color& v){ mClearColor=v; return *this; }

	/// Set stereographic mode
	Stereographic& mode(StereoMode v){ mMode=v; return *this; }

	/// Set stereographic active
	Stereographic& stereo(bool v){ mStereo=v; return *this; }

	/// Set anaglyph mode
	Stereographic& anaglyphMode(AnaglyphMode v){ mAnaglyphMode=v; return *this; }

	/// Set omnigraphic mode
	/// slices: sets number of sub-viewport slices to render
	/// fov (degrees) sets field of view (horizontal)
	/// NOTE: cam.fovy will be ignored in omni mode
	Stereographic& omni(bool enable) { mOmni = enable; return *this; }
	Stereographic& omni(bool enable, unsigned slices, double fov=360){
		mOmni = enable; mSlices = slices; mOmniFov = fov; return *this; }
	Stereographic& omniFov( double fov ) { mOmniFov = fov; return *this; }
	Stereographic& omniSlices( int slices ) { mSlices = slices; return *this; }


	/// Get background clear color
	const Color& clearColor() const { return mClearColor; }

	/// Get stereographic mode
	StereoMode mode() const { return mMode; }

	/// Get stereographic active
	bool stereo() const { return mStereo; }

	/// Get anaglyph mode
	AnaglyphMode anaglyphMode() const { return mAnaglyphMode; }

	/// Get current eye number (0 == right, 1 == left)
	unsigned eyeNumber() const { return mEyeNumber; }

	/// Get whether omni mode is on
	bool omni() const { return mOmni; }

	/// Get current omni fov:
	double omniFov() { return mOmniFov; }

	// These accessors will be valid only during the Drawable's onDraw() event
	// they can be useful to simulate the OpenGL pipeline transforms
	//	e.g. Matrix4d::multiply(Vec4d eyespace, stereo.modelView(), Vec4d objectspace);
	//	e.g. Matrix4d::multiply(Vec4d clipspace, stereo.projection(), Vec4d eyespace);
	//	e.g. Matrix4d::multiply(Vec4d clipspace, stereo.modelViewProjection(), Vec4d objectspace);
	// to convert in the opposite direction, use Matrix4::inverse().

	/// Get current projection matrix
	const Matrix4d& projection() const { return mProjection; }

	/// Get current view matrix
	const Matrix4d& view() const { return mView; }

	/// Get current eye position
	const Vec3d& eye() const { return mEye; }

	/// Get current viewport
	const Viewport& viewport() const { return mVP; }

	/// Convert a normalized screen space position to world space

	/// This converts a coordinate from normalized device coordinate (NDC) space
	/// to world space. The range of each coordinate in NDC space is [-1,1].
	Vec3d unproject(const Vec3d& ndc);

	/// Convert screen pixel coordinate to world position
	template <class T>
	Vec<3,T> pixelToWorld(const Vec<2,T>& p);

	template <class T>
	Vec<3,T> pixelToWorld(T x, T y){ return pixelToWorld(Vec<2,T>(x,y)); }

	/// Transform a vector from world space to clip space
	template <class T>
	Vec<4,T> toClipSpace(const Vec<4,T>& v) const;

	/// Returns origin of world space converted to clip space
	Vec4d toClipSpace() const;

	/// Transform a vector from world space to normalized device coordinate space

	/// Normalized device coordinate space is the viewing frustrum normalized
	/// to a cube with each dimension spanning [-1,1]. The x-coordinate
	/// corresponds to [left,right], the y-coordinate to [bottom,top], and the
	/// z-coordinate to [near,far].
	template <class T>
	Vec<3,T> toNDCSpace(const Vec<4,T>& v) const;
	template <class T>
	Vec<3,T> toNDCSpace(const Vec<3,T>& v) const;

	/// Returns origin of world space converted to normalized device coordinate space
	Vec3d toNDCSpace() const;

protected:
	StereoMode mMode;
	AnaglyphMode mAnaglyphMode;
	Color mClearColor;
	unsigned mSlices;	// number of omni slices
	double mOmniFov;	// field of view of omnigraphics
	Matrix4d mProjection, mView;
	Vec3d mEye;
	unsigned mEyeNumber;
	Viewport mVP;
	bool mStereo;
	bool mOmni;

	void pushDrawPop(Graphics& g, Drawable& draw);
	void sendViewport(Graphics& g, const Viewport& vp);
	void sendClear(Graphics& g);

	void drawEye(StereoMode eye, Graphics& g, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear, double pixelaspect);

public:
	// \deprecated Use view()
	const Matrix4d& modelView() const { return mView; }

	// Get product of current projection and modelview matrices
	Matrix4d modelViewProjection() const { return mProjection * mView; }
};


template <class T>
Vec<3,T> Stereographic::pixelToWorld(const Vec<2,T>& p){
	Vec<3,T> ndc;
	ndc.x = (p.x / mVP.w) * 2. - 1.;
	ndc.y = (p.y / mVP.h) *-2. + 1.;
	ndc.z = toNDCSpace().z;
	return unproject(ndc);
}

template <class T>
inline Vec<4,T> Stereographic::toClipSpace(const Vec<4,T>& v) const {
	return modelViewProjection() * v;
}

inline Vec4d Stereographic::toClipSpace() const {
	return modelViewProjection().col(3);
}

template <class T>
inline Vec<3,T> Stereographic::toNDCSpace(const Vec<4,T>& v) const {
	Vec<4,T> clipSpace = toClipSpace(v);
	return clipSpace.get(0,1,2) / clipSpace[3];
}
template <class T>
inline Vec<3,T> Stereographic::toNDCSpace(const Vec<3,T>& v) const {
	return toNDCSpace(Vec<4,T>(v,1));
}

inline Vec3d Stereographic::toNDCSpace() const {
	Vec4d clipSpace = toClipSpace();
	return clipSpace.get(0,1,2) / clipSpace[3];
}

} // al::

#endif
