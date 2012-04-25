#ifndef INCLUDE_AL_GRAPHICS_STEREOGRAPHIC_HPP
#define INCLUDE_AL_GRAPHICS_STEREOGRAPHIC_HPP

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
	Utilities for rendering Graphics in various stereographic modes

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/spatial/al_Camera.hpp"
#include "allocore/spatial/al_Pose.hpp"
#include "allocore/types/al_Color.hpp"

namespace al{

///	Higher-level utility class to manage various stereo rendering techniques
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
		RED_BLUE = 0,	/**< */
		RED_GREEN,		/**< */
		RED_CYAN,		/**< */
		BLUE_RED,		/**< */
		GREEN_RED,		/**< */
		CYAN_RED		/**< */
	};
	
	Stereographic() 
	:	mMode(ANAGLYPH), mAnaglyphMode(RED_CYAN), mClearColor(Color(0)), mSlices(24), mOmniFov(360),
		mStereo(false), mOmni(false)
	{}

	~Stereographic(){}

	/// Draw the scene according to the stored stereographic mode
	
	/// @param[in] gl		graphics interface
	/// @param[in] cam		local viewing frustum
	/// @param[in] pose		viewer position and orientation
	/// @param[in] vp		region of screen to render to
	/// @param[in] draw		function object with drawing commands
	/// @param[in] clear	whether to clear the color/depth buffers
	void draw			(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true);
	
	// So many different ways to draw :-)
	void drawMono		(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true);
	void drawActive		(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true);
	void drawAnaglyph	(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true);
	void drawDual		(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true);
	void drawLeft		(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true);
	void drawRight		(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true);
	
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

	/// Get whether omni mode is on
	bool omni() const { return mOmni; }
	
	// These accessors will be valid only during the Drawable's onDraw() event
	// they can be useful to simulate the OpenGL pipeline transforms
	//	e.g. Matrix4d::multiply(Vec4d eyespace, stereo.modelView(), Vec4d objectspace); 
	//	e.g. Matrix4d::multiply(Vec4d clipspace, stereo.projection(), Vec4d eyespace);
	//	e.g. Matrix4d::multiply(Vec4d clipspace, stereo.modelViewProjection(), Vec4d objectspace);
	// to convert in the opposite direction, use Matrix4::inverse(). 
	const Matrix4d& modelView() const { return mModelView; }
	const Matrix4d& projection() const { return mProjection; }
	Matrix4d modelViewProjection() const { return mProjection * mModelView; }
	const Vec3d& eye() const { return mEye; }
	const Viewport& viewport() const { return mVP; }
	
protected:
	StereoMode mMode;
	AnaglyphMode mAnaglyphMode;
	Color mClearColor;
	unsigned mSlices;	// number of omni slices
	double mOmniFov;	// field of view of omnigraphics	
	Matrix4d mProjection, mModelView;
	Vec3d mEye;
	Viewport mVP;
	bool mStereo;
	bool mOmni;

	void pushDrawPop(Graphics& gl, Drawable& draw);
	void sendViewport(Graphics& gl, const Viewport& vp);
	void sendClear(Graphics& gl);
};

} // al::

#endif
