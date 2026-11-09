#include "allocore/graphics/al_Lens.hpp"
#include "allocore/math/al_Frustum.hpp"
#include "allocore/spatial/al_Pose.hpp"
#include "allocore/graphics/al_Viewpoint.hpp"

namespace al{

Viewpoint::Viewpoint(const Pose& transform)
:	mTransform(transform)
{}

Viewpoint& Viewpoint::anchor(float ax, float ay){
	mAnchorX=ax; mAnchorY=ay;
	return *this;
}

Viewpoint& Viewpoint::stretch(float sx, float sy){
	mStretchX=sx; mStretchY=sy;
	return *this;
}

Pose Viewpoint::worldTransform() const {
	return mParentTransform ? (*mParentTransform) * mTransform : mTransform;
}

Frustumd Viewpoint::frustum() const {
	Frustumd fr;
	lens().frustum(fr, worldTransform(), viewport().aspect());
	return fr;
}

void Viewpoint::onParentResize(int w, int h){
	mViewport.l = w * anchorX();
	mViewport.b = h * anchorY();
	mViewport.w = w * stretchX();
	mViewport.h = h * stretchY();
}

} // al::
