#ifndef INCLUDE_AL_CONTEXT_HPP
#define INCLUDE_AL_CONTEXT_HPP

#include <list>
#include <vector>

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
	
	/// Set viewport dimensions in screen coordinates
	ViewPort& dimensions(double w, double h);
	ViewPort& dimensions(double w, double h, double x, double y);
	
	/// Tell this viewport which camera to use.
	/// Note: no memory-management is done, but the camera must live as long as this viewport uses it!
	ViewPort& camera(Camera * cam) { mCamera = cam; return *this; }
	
	ViewPort& eyeSep(double v){ mEyeSep=v; return *this; }	///< Set eye separation
	
	double eyeSep() const { return mEyeSep; }				///< Get eye separation
	double ratio() const { return mAspect; }					///< Get aspect ratio (width/height)
	double aspect() const { return mAspect; }					///< Get aspect ratio (width/height)
	double nearTop() const;									///< Get frustum near plane top
	double nearRight() const;								///< Get frustum near plane right
	double farTop() const;									///< Get frustum far plane top
	double farRight() const;								///< Get frustum far plane right
	
	
	double left() const { return mLeft; }					///< Screen Co-ordinates
	double bottom() const { return mLeft; }					///< Screen Co-ordinates
	double width() const { return mLeft; }					///< Screen Co-ordinates
	double height() const { return mLeft; }					///< Screen Co-ordinates
	
	Camera& camera() { return *mCamera; }
	
	/// use this to apply a transform to the projection matrix
	Matrix4d& userProjectionTransform() { return mUserProjectionTransform; }
	
	void applyFrustumStereo(double eyesep);

	Frustumd monoFrustum();
	Frustumd leftFrustum(Context * ctx);
	Frustumd rightFrustum(Context * ctx);
	Frustumd biFrustum(Context * ctx);		// wide enough to encompass both left & right frusti	
	
	/// dispatch the GL commands to apply a camera's orientation (i.e. gluLookAt)
	void view(double eyesep);

protected:
	
	double mLeft, mBottom, mWidth, mHeight;	// screen coordinates
	double mAspect;						// aspect ratio


	Camera * mCamera;					// world-space view position & orientation
	Frustum<double> mFrustum;
	Matrix4d mUserProjectionTransform;	// user-defined projection transform
	double mEyeSep;						// Eye separation for stereo. AKA IOD (inter ocular distance)
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
	
	enum AnaglyphMode {
		RedBlue = 0,
		RedGreen,
		RedCyan,
		BlueRed,
		GreenRed,
		CyanRed
	};
	
	Context() 
	: mStereo(false), mMode(Anaglyph), mAnaglyphMode(RedBlue), mStereoOffset(Vec3d(1, 0, 0)) 
	{}
	
	Context& mode(StereoMode v){ mMode=v; return *this; }	///< Set stereographic mode
	Context& stereo(bool v){ mStereo=v; return *this; }		///< Set stereographic active
	
	StereoMode mode() const { return mMode; }				///< Get stereographic mode
	bool stereo() const { return mStereo; }					///< Get stereographic active
	
	/// This is a handy accessor for the (principal) viewport.
	ViewPort& viewport() { return mViewPort; /*return mViewPorts.front();*/ }
	
	/// selects the appropriate draw method according to stereo setting:
	void draw(void (*draw)(void *), void * userdata);
	
	/// So many different ways to draw :-)
	void drawMono(void (*draw)(void *), void * userdata);
	void drawActive(void (*draw)(void *), void * userdata);
	void drawAnaglyph(void (*draw)(void *), void * userdata);
	void drawDual(void (*draw)(void *), void * userdata);
	void drawLeft(void (*draw)(void *), void * userdata);
	void drawRight(void (*draw)(void *), void * userdata);

protected:
	Vec3d mStereoOffset;					// eye offset vector (right eye; left eye is inverse), usually (1, 0, 0)
	StereoMode mMode;
	AnaglyphMode mAnaglyphMode;
	bool mStereo;
	
	ViewPort mViewPort;			// every context has a viewport
	//std::list<ViewPort> mViewPorts;	// future support for multiple viewports per context
	
//	Notifier<onCreate>
//	Notifier<onDestroy>
};

} // al::

#endif // include guard

