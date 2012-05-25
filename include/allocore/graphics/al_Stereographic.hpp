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
	/// @param[in] lens		local viewing frustum
	/// @param[in] pose		viewer position and orientation
	/// @param[in] vp		region of screen to render to
	/// @param[in] draw		function object with drawing commands
	/// @param[in] clear	whether to clear the color/depth buffers
	/// @param[in] pixelaspect	additional aspect multipler (for non-square pixels)
	void draw			(Graphics& gl, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true, double pixelaspect=1.);
	
	// So many different ways to draw :-)
	void drawMono		(Graphics& gl, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true, double pixelaspect=1.);
	void drawActive		(Graphics& gl, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true, double pixelaspect=1.);
	void drawAnaglyph	(Graphics& gl, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true, double pixelaspect=1.);
	void drawDual		(Graphics& gl, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true, double pixelaspect=1.);
	void drawLeft		(Graphics& gl, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true, double pixelaspect=1.);
	void drawRight		(Graphics& gl, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear=true, double pixelaspect=1.);
	
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
	
	/// Get current omni fov:
	double omniFov() { return mOmniFov; }
	
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
