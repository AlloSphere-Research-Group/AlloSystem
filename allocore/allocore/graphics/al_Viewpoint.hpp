#ifndef INC_AL_VIEWPOINT_HPP
#define INC_AL_VIEWPOINT_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	A viewing region with its own camera

	Author(s):
	Lance Putnam, 2010
*/

#include "allocore/graphics/al_Viewport.hpp"
#include "allocore/spatial/al_Pose.hpp"

namespace al{

template <class T> class Frustum;
class Color;
class Lens;

/// Viewpoint within a scene

/// A viewpoint is an aggregation of a viewport (screen region), a pose
/// (3D position and orientation), and a lens.
class Viewpoint{
public:

	Viewpoint(const Pose& transform = Pose::identity());

	float anchorX() const { return mAnchorX; }
	float anchorY() const { return mAnchorY; }
	float stretchX() const { return mStretchX; }
	float stretchY() const { return mStretchY; }

	/// Set anchoring factors relative to bottom-left corner of window

	/// \param[in] ax	anchor factor relative to left edge of window, in [0,1]
	/// \param[in] ay	anchor factor relative to bottom edge of window, in [0,1]
	Viewpoint& anchor(float ax, float ay);

	/// Set stretch factors relative to bottom-left corner of window

	/// \param[in] sx	stretch factor relative to left edge of window, in [0,1]
	/// \param[in] sy	stretch factor relative to bottom edge of window, in [0,1]
	Viewpoint& stretch(float sx, float sy);

	/// Get lens
	const Lens& lens() const { return *mLens; }
	Viewpoint& lens(Lens& v){ mLens=&v; return *this; }
	bool hasLens() const { return nullptr != mLens; }

	/// Get clear color
	const Color& clearColor() const { return *mClearColor; }
	Viewpoint& clearColor(Color& v){ mClearColor=&v; return *this; }
	bool hasClearColor() const { return nullptr != mClearColor; }

	/// Get parent transform
	const Pose * parentTransform() const { return mParentTransform; }
	Viewpoint& parentTransform(Pose& v){ return parentTransform(&v); }
	Viewpoint& parentTransform(Pose* v){ mParentTransform = v; return *this; }

	/// Get local transform
	const Pose& transform() const { return mTransform; }
	Pose& transform(){ return mTransform; }
	Viewpoint& transform(const Pose& v){ mTransform=v; return *this; }

	Pose worldTransform() const;

	/// Get screen viewport
	const Viewport& viewport() const { return mViewport; }
	Viewport& viewport(){ return mViewport; }

	/// Get calculated viewing frustum
	Frustum<double> frustum() const;

	/// Call to update viewport using stretch/anchor amounts when parent dimensions change
	void onParentResize(int w, int h);

private:
	Viewport mViewport{0,0,0,0};		// screen display region
	Pose * mParentTransform = nullptr;	// parent transform, nullptr if none
	Pose mTransform;					// local transform
	float mAnchorX=0.f, mAnchorY=0.f;	// viewport anchor factors relative to parent window
	float mStretchX=1.f, mStretchY=1.f;	// viewport stretch factors relative to parent window
	Lens * mLens = nullptr;				// if not set, will be set to scene's default lens
	Color * mClearColor = nullptr;
};

} // al::
#endif
