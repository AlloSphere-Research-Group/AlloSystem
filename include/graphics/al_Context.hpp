#ifndef INCLUDE_AL_CONTEXT_HPP
#define INCLUDE_AL_CONTEXT_HPP

#include <list>

#include "math/al_Frustum.hpp"
#include "math/al_Plane.hpp"
#include "math/al_Vec.hpp"
#include "math/al_Quat.hpp"
#include "math/al_Matrix4.hpp"
#include "spatial/al_Camera.hpp"
#include "graphics/al_Config.h"

namespace al {

class Context;

class ViewPort {
public:

	ViewPort(double eyeSep = 0.0);
	~ViewPort();
	
	ViewPort& eyeSep(double v){ mEyeSep=v; return *this; }	///< Set eye separation
	
	/// Set viewport dimensions in screen coordinates
	ViewPort& dimensions(double w, double h);
	ViewPort& dimensions(double w, double h, double x, double y);

	double eyeSep() const { return mEyeSep; }				///< Get eye separation
	double ratio() const { return mRatio; }					///< Get aspect ratio (width/height)
	double nearTop() const;									///< Get frustum near plane top
	double nearRight() const;								///< Get frustum near plane right
	double farTop() const;									///< Get frustum far plane top
	double farRight() const;								///< Get frustum far plane right
	
	/// use this to apply a transform to the projection matrix
	Matrix4d& userProjectionTransform() { return mUserProjectionTransform; }

	Frustumd& monoFrustum();
	Frustumd& leftFrustum(Context * ctx);
	Frustumd& rightFrustum(Context * ctx);
	Frustumd& biFrustum(Context * ctx);		// wide enough to encompass both left & right frusti

protected:
	Camera * mCamera;					// world-space view position & orientation
	Matrix4d mUserProjectionTransform;	// user-defined projection transform
	double mLeft, mBottom, mWidth, mHeight;	// screen coordinates
	double mRatio;						// aspect ratio
	double mEyeSep;						// Eye separation for stereo
};


class Context {
public:
	enum StereoMode{
		Anaglyph=0,	/**< Red (left eye) / cyan (right eye) stereo */
		Active,		/**< Active quad-buffered stereo */
		Dual,		/**< Dual side-by-side stereo */
		LeftEye,	/**< Left eye only */
		RightEye	/**< Right eye only */
	};
	
	Context() : mStereo(false), mMode(Anaglyph), mStereoOffset(Vec3d(1, 0, 0)) {
		
	}
	
	Context& mode(StereoMode v){ mMode=v; return *this; }	///< Set stereographic mode
	Context& stereo(bool v){ mStereo=v; return *this; }		///< Set stereographic active
	
	StereoMode mode() const { return mMode; }				///< Get stereographic mode
	bool stereo() const { return mStereo; }					///< Get stereographic active

protected:
	Vec3d mStereoOffset;					// eye offset vector (right eye; left eye is inverse), usually (1, 0, 0)
	StereoMode mMode;
	bool mStereo;
	
	std::list<ViewPort> mViewPorts;
//	Notifier<onCreate>
//	Notifier<onDestroy>
};

} // al::

#endif // include guard

