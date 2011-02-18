#ifndef INCLUDE_AL_GRAPHICS_STEREOGRAPHIC_HPP
#define INCLUDE_AL_GRAPHICS_STEREOGRAPHIC_HPP

/*
 *  A collection of functions and classes related to application mainloops
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */

/*
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
*/

#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/spatial/al_Camera.hpp"

namespace al{

/// A framed area on a display screen
struct Viewport {
	float l, b, w, h;	///< left, bottom, width, height

	Viewport(float w=800, float h=600) : b(0), l(0), w(w), h(h) {}
	Viewport(float l, float b, float w, float h) : l(l), b(b), w(w), h(h) {}

	/// Get aspect ratio
	float aspect() const { return (h!=0 && w!=0) ? float(w)/h : 1; }

	/// Set dimensions
	void set(float l_, float b_, float w_, float h_){ l=l_; b=b_; w=w_; h=h_; }
};


///	Higher-level utility class to manage various stereo rendering techniques
class Stereographic {
public:
	enum StereoMode{
		Anaglyph=0,	/**< Red (left eye) / cyan (right eye) stereo */
		Active,		/**< Active quad-buffered stereo */
		Dual,		/**< Dual side-by-side stereo */
		LeftEye,	/**< Left eye only */
		RightEye	/**< Right eye only */
	};
	enum AnaglyphMode {
		RedBlue = 0,
		RedGreen,
		RedCyan,
		BlueRed,
		GreenRed,
		CyanRed
	};
	
	Stereographic() 
	: mMode(Anaglyph), mAnaglyphMode(RedCyan), mClearColor(Color(0)), mStereo(false) {}

	~Stereographic() {}

	/// Draw the scene according to the stored stereographic mode
	void draw			(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw);
	
	/// So many different ways to draw :-)
	void drawMono		(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw);
	void drawActive		(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw);
	void drawAnaglyph	(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw);
	void drawDual		(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw);
	void drawLeft		(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw);
	void drawRight		(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw);

	/// Blue line sync for active stereo (for those projectors that need it)
	/// add this call at the end of rendering (just before the swap buffers call)
	void drawBlueLine(double window_width, double window_height);
	
	Stereographic& clearColor(const Color& v){ mClearColor=v; return *this; }	///< Set background clear color
	Stereographic& mode(StereoMode v){ mMode=v; return *this; }					///< Set stereographic mode
	Stereographic& stereo(bool v){ mStereo=v; return *this; }					///< Set stereographic active
	Stereographic& anaglyphMode(AnaglyphMode v) { mAnaglyphMode=v; return *this; }	///< set glasses type
	
	const Color& clearColor() const { return mClearColor; }		///< Get background clear color
	StereoMode mode() const { return mMode; }					///< Get stereographic mode
	bool stereo() const { return mStereo; }						///< Get stereographic active
	AnaglyphMode anaglyphMode() const { return mAnaglyphMode; }	///< get anaglyph glasses type
	
protected:
	StereoMode mMode;
	AnaglyphMode mAnaglyphMode;
	Color mClearColor;
	bool mStereo;
};

} // al::

#endif
