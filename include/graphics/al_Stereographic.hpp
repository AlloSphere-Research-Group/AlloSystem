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

#include "graphics/al_Common.hpp"
#include "protocol/al_Graphics.hpp"
#include "spatial/al_CoordinateFrame.hpp"

namespace al {
namespace gfx{

/*
	Stereographic
	Higher-level utility class to manage various stereo rendering techniques
*/
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
	
	Stereographic() : mFovy(60), mNear(0.01), mFar(100), mFocal(2), mEyeSep(0.1), mMode(Anaglyph), mAnaglyphMode(RedBlue), mStereo(false)
	{}
	~Stereographic() {}

	void draw(Graphics& gl, Nav& cam, void (*draw)(void *), double width, double height, void * userdata);

	/// So many different ways to draw :-)
	void drawMono(Graphics& gl, Nav& cam, void (*draw)(void *), double width, double height, void * userdata);
	void drawActive(Graphics& gl, Nav& cam, void (*draw)(void *), double width, double height, void * userdata);
	void drawAnaglyph(Graphics& gl, Nav& cam, void (*draw)(void *), double width, double height, void * userdata);
	void drawDual(Graphics& gl, Nav& cam, void (*draw)(void *), double width, double height, void * userdata);
	void drawLeft(Graphics& gl, Nav& cam, void (*draw)(void *), double width, double height, void * userdata);
	void drawRight(Graphics& gl, Nav& cam, void (*draw)(void *), double width, double height, void * userdata);
	
	/// Blue line sync for active stereo (for those projectors that need it)
	/// add this call at the end of rendering (just before the swap buffers call)
	void drawBlueLine(double window_width, double window_height);
	
	Stereographic& mode(StereoMode v){ mMode=v; return *this; }	///< Set stereographic mode
	Stereographic& stereo(bool v){ mStereo=v; return *this; }		///< Set stereographic active
	Stereographic& anaglyphMode(AnaglyphMode v) { mAnaglyphMode=v; return *this; }	///< set glasses type
	
	Stereographic& fovy(double v){ mFovy=v; return *this; }	
	Stereographic& near(double v){ mNear=v; return *this; }	
	Stereographic& far(double v) { mFar=v; return *this; }	
	Stereographic& focal(double v){ mFocal=v; return *this; }	
	Stereographic& eyesep(double v){ mEyeSep=v; return *this; }	
	
	StereoMode mode() const { return mMode; }				///< Get stereographic mode
	bool stereo() const { return mStereo; }					///< Get stereographic active
	AnaglyphMode anaglyphMode() const { return mAnaglyphMode; }	///< get anaglyph glasses type
	
	double fovy() const { return mFovy; }
	double near() const { return mNear; }
	double far() const { return mFar; }
	double focal() const { return mFocal; }
	
	double IOD() const { return mEyeSep * mFocal/30.0; }
	
protected:
	double mFovy, mNear, mFar, mFocal;
	double mEyeSep;
	
	StereoMode mMode;
	AnaglyphMode mAnaglyphMode;
	bool mStereo;
};

} // gfx::
} // al::

#endif /* include guard */
