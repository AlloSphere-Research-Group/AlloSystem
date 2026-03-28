#ifndef INC_AL_STEREOGRAPHIC_HPP
#define INC_AL_STEREOGRAPHIC_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Utilities for rendering Graphics in various stereographic modes

	Author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include <functional>
#include "allocore/graphics/al_Viewport.hpp"
#include "allocore/math/al_Mat.hpp"
#include "allocore/math/al_Matrix4.hpp"
#include "allocore/math/al_Vec.hpp"
#include "allocore/types/al_Color.hpp"

namespace al{

class Graphics;
class Drawable;
class Lens;
class Pose;

///	Higher-level utility class to manage various stereo rendering techniques
///
/// \ingroup allocore
class Stereoscopic {
public:

	/// Stereoscopic mode
	enum StereoMode : unsigned char {
		ANAGLYPH=0,		/**< Red (left eye) / cyan (right eye) stereo */
		ACTIVE,			/**< Active quad-buffered stereo */
		DUAL,			/**< Dual side-by-side stereo */
		LEFT_EYE,		/**< Left eye only */
		RIGHT_EYE		/**< Right eye only */
	};

	/// Anaglyph mode
	enum AnaglyphMode : unsigned char {
		RED_CYAN,		/**< Left eye red, right eye cyan (the norm) */
		RED_BLUE,		/**< Left eye red, right eye blue */
		RED_GREEN,		/**< Left eye red, right eye green */
		CYAN_RED,		/**< Left eye cyan, right eye red */
		BLUE_RED,		/**< Left eye blue, right eye red */
		GREEN_RED		/**< Left eye green, right eye red */
	};


	/// Draw the scene according to the stored stereographic mode

	/// \param[in] g		graphics interface
	/// \param[in] lens		local viewing frustum
	/// \param[in] pose		viewer position and orientation
	/// \param[in] vp		region of screen to render to
	/// \param[in] draw		function object with drawing commands
	/// \param[in] clear	whether to clear the color/depth buffers
	/// \param[in] pixelaspect	additional aspect multipler (for non-square pixels)
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
	Stereoscopic& clearColor(const Color& v){ mClearColor=v; return *this; }

	/// Set stereographic mode
	Stereoscopic& mode(StereoMode v){ mMode=v; return *this; }

	/// Set stereographic active
	Stereoscopic& stereo(bool v){ mStereo=v; return *this; }

	/// Set anaglyph mode
	Stereoscopic& anaglyphMode(AnaglyphMode v){ mAnaglyphMode=v; return *this; }

	/// Set omni mode
	///
	/// \param[in] enable	Whether mode is enabled
	/// \param[in] slices	Sets number of sub-viewport slices to render
	/// \param[in] fov		Sets field of horizontal view in degrees. 
	///						Any other FOV will be ignored.
	Stereoscopic& omni(bool enable, unsigned char slices, double fov=360){
		return omni(enable).omniSlices(slices).omniFov(fov);
	}
	Stereoscopic& omni(bool enable){ mOmni = enable; return *this; }
	Stereoscopic& omniFov(float fov){ mOmniFovX = fov; return *this; }
	Stereoscopic& omniSlices(unsigned char slices){ mSlices = slices; return *this; }


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
	float omniFov() { return mOmniFovX; }

	// These accessors will be valid only during the Drawable's onDraw() event.

	/// Get current projection matrix
	const Matrix4d& projection() const { return mProj; }

	/// Get current view matrix
	const Matrix4d& view() const { return mView; }

	/// Get current view-projection matrix
	Matrix4d viewProjection() const { return mProj * mView; }
	Matrix4d MVP() const { return viewProjection(); }

	/// Get current eye position
	const Vec3d& eye() const { return mEye; }

	/// Get current viewport
	const Viewport& viewport() const { return mVP; }

	/// Convert a normalized screen space position to world space

	/// This converts a coordinate from normalized device coordinate (NDC) space
	/// to world space. The range of each coordinate in NDC space is [-1,1].
	Vec3d unproject(const Vec3d& ndc) const;

	/// Convert screen pixel coordinate to world position
	template <class T>
	Vec3d pixelToWorld(const Vec<2,T>& p) const;

	template <class T>
	Vec3d pixelToWorld(T x, T y) const { return pixelToWorld(Vec<2,T>(x,y)); }

	/// Transform a vector from world space to clip space
	template <class T>
	Vec4d toClipSpace(const Vec<4,T>& v) const;

	/// Returns origin of world space converted to clip space
	Vec4d toClipSpace() const;

	/// Transform a vector from world space to normalized device coordinate space

	/// Normalized device coordinate space is the viewing frustrum normalized
	/// to a cube with each dimension spanning [-1,1]. The x-coordinate
	/// corresponds to [left,right], the y-coordinate to [bottom,top], and the
	/// z-coordinate to [near,far].
	template <class T>
	Vec3d toNDCSpace(const Vec<4,T>& v) const;
	template <class T>
	Vec3d toNDCSpace(const Vec<3,T>& v) const;

	/// Returns origin of world space converted to normalized device coordinate space
	Vec3d toNDCSpace() const;

protected:
	Matrix4d mProj{1}, mView{1};
	Vec3d mEye;
	Viewport mVP;
	Color mClearColor{0};
	float mOmniFovX = 360; // field of view of omni
	unsigned char mSlices = 24; // number of omni slices
	StereoMode mMode = ANAGLYPH;
	AnaglyphMode mAnaglyphMode = RED_CYAN;
	unsigned char mEyeNumber = 0;
	bool mStereo = false;
	bool mOmni = false;

	// onDraw wrapped in push/pop calls
	void pushDrawPop(Graphics& g, Drawable& draw);
	// set scissor/viewport regions
	void sendViewport(Graphics& g, const Viewport& vp);
	// clear color/depth buffers based on current viewport
	void sendClear(Graphics& g);
	// sets eye and view members
	void setView(const Pose& pose, double eyeShift=0.);

	void drawEye(StereoMode eye, Graphics& g, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear, double pixelaspect);

	struct ViewSlice{
		Viewport vp;
		double fovy;
		double aspect;
		Quatd ori;
	};

	double angleOfOmniSlice(int slice) const; // angle at center of slice

	ViewSlice omniSlice(int i, const Viewport& vp, double pixelAspect) const;

	void forEachViewSlice(const std::function<void(ViewSlice)>& onSlice, const Lens& lens, const Viewport& vp, double pixelAspect) const;

public:
	// \deprecated Use view()
	const Matrix4d& modelView() const { return mView; }

	// Get product of current projection and modelview matrices
	Matrix4d modelViewProjection() const { return mProj * mView; }
};


template <class T>
Vec3d Stereoscopic::pixelToWorld(const Vec<2,T>& p) const {
	Vec3d ndc;
	ndc.x = (p.x / mVP.w) * 2. - 1.;
	ndc.y = (p.y / mVP.h) *-2. + 1.;
	ndc.z = toNDCSpace().z;
	return unproject(ndc);
}

template <class T>
inline Vec4d Stereoscopic::toClipSpace(const Vec<4,T>& v) const {
	return modelViewProjection() * v;
}

inline Vec4d Stereoscopic::toClipSpace() const {
	return modelViewProjection().col<3>();
}

template <class T>
inline Vec3d Stereoscopic::toNDCSpace(const Vec<4,T>& v) const {
	auto clipSpace = toClipSpace(v);
	return clipSpace.xyz() / clipSpace.w;
}
template <class T>
inline Vec3d Stereoscopic::toNDCSpace(const Vec<3,T>& v) const {
	return toNDCSpace(Vec<4,T>(v, T(1)));
}

inline Vec3d Stereoscopic::toNDCSpace() const {
	auto clipSpace = toClipSpace();
	return clipSpace.xyz() / clipSpace.w;
}

} // al::
#endif
