/* temporary. eventually replace with al_Graphics: */
#if defined (__APPLE__) || defined (OSX)
	#include <OpenGL/OpenGL.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>
	#include <OpenGL/glu.h>	
#elif defined(__linux__)
	#include <GL/glew.h>
	#include <GL/gl.h>
	#include <GL/glext.h>
	#include <GL/glu.h>
	#include <time.h>	
#elif defined(WIN32)
	#include <windows.h>
	#include <gl/gl.h>
	#include <gl/glu.h>
	#pragma comment( lib, "winmm.lib")
	#pragma comment( lib, "opengl32.lib" )
	#pragma comment( lib, "glu32.lib" )	
#endif

#include "spatial/al_Camera.hpp"


namespace al{

Camera :: Camera(double aper, double nearClip, double farClip, double focalLen, double eyeSep)
:	mFocalLength(focalLen), mZoom(0), mEyeSep(eyeSep), mNear(nearClip), mFar(farClip),
	mMode(Anaglyph), mStereo(false)
{
	aperture(aper);
	dimensions(4,3,0,0);
	calcFrustum();
}


void Camera::calcFrustum(){
	mRatio = mWidth / (mHeight>0 ? mHeight : 1e-10);

	if(stereo() && (mode() == Dual)) 
		mRatio /= 2;		// assume screen space is twice the width of the display

	double zm = pow(2, -zoom());
	mNearTop = near() * mTanFOV * zm;
	mFarTop = far() * mTanFOV * zm;
	mNearOverFocalLength = near() / (focalLength() * 2.) * zm;

	// Derive the eye offset vector
	mStereoOffset = mUX * (eyeSep()*0.5);

	// TODO: this is redundant
	//mFrustum.setCamInternals(aperture(), ratio(), near(), far());
}


Camera& Camera::aperture(double v){
	mFOVY=v;
	static double const tanCoef = 0.01745329252*0.5;	// degree-to-radian over /2
	mTanFOV = tan(tanCoef * aperture());
	return *this;
}

Camera& Camera::dimensions(double w, double h){ return dimensions(w,h,mLeft,mBottom); }

Camera& Camera::dimensions(double w, double h, double x, double y){
	mLeft=x; mBottom=y; mWidth=w; mHeight=h;
	return *this;
}
	
double Camera::nearTop() const { return mNearTop; }
double Camera::nearRight() const { return nearTop() * ratio(); }
double Camera::farTop() const { return mFarTop; }
double Camera::farRight() const { return farTop() * ratio(); }


void Camera::begin(double w, double h, double x, double y){
	dimensions(w,h,x,y);
	calcFrustum();
	
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	//push(Projection);
	//push(ModelView);
	glMatrixMode(GL_PROJECTION); glPushMatrix(); //pop(Projection);
	glMatrixMode(GL_MODELVIEW); glPushMatrix(); //pop(ModelView);
	
	//enable(ScissorTest);
	glEnable(GL_SCISSOR_TEST);
	//scissor(mLeft,mBottom, mWidth,mHeight);
	glScissor(mLeft, mBottom, mWidth, mHeight);
	
	if(mode() == Dual){
		//drawBuffer(Back);
		glDrawBuffer(GL_BACK);
		//clear(ColorBufferBit | DepthBufferBit);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void Camera::end(){
	if(mode() == Anaglyph) glColorMask(1, 1, 1, 1); //colorMask(1,1,1,1);
	//disable(ScissorTest);
	glDisable(GL_SCISSOR_TEST);
	//popAttrib();
	glPopAttrib();
	glMatrixMode(GL_PROJECTION); glPopMatrix(); //pop(Projection);
	glMatrixMode(GL_MODELVIEW); glPopMatrix(); //pop(ModelView);
}

int Camera::eyeStart() const {
	if(mode() == RightEye) return 1;
	else return 0;
}

int Camera::eyeEnd() const {
	if(mode() == LeftEye) return 1;
	else return 2;
}

Camera::FrustumGL Camera::frustum(double sep) const{
	FrustumGL f;
	double sep_ndfl = sep*mNearOverFocalLength;
	
	f.left	=-nearRight() + sep_ndfl;
	f.right	= nearRight() + sep_ndfl;
	f.bottom=-nearTop();
	f.top	= nearTop();
	f.near	= near();
	f.far	= far();

	return f;
}

void Camera::setEye(int i){ stereo() ? (i ? right() : left()) : mid(); }

void Camera::setFrustum(double sep){
	//matrixMode(Projection); identity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	FrustumGL f = frustum(sep);

	//frustum(l,r,b,t near, far)
	//glw::frustum(f.left, f.right, f.bottom, f.top, f.near, f.far);
	glFrustum(f.left, f.right, f.bottom, f.top, f.near, f.far);
}



void Camera::mid(){
	//clear(ColorBufferBit | DepthBufferBit);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//viewport(mLeft,mBottom, mWidth,mHeight);
	glViewport(mLeft, mBottom, mWidth, mHeight);
	setFrustum(0);
	setLookAt(0,0,0);
}

void Camera::left(){
	
	setFrustum(eyeSep());
	
	switch(mMode){
		case Active:
			//drawBuffer(BackLeft);
			glDrawBuffer(GL_BACK_LEFT);
			//clear(ColorBufferBit | DepthBufferBit);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//viewport(mLeft,mBottom, mWidth,mHeight);
			glViewport(mLeft, mBottom, mWidth, mHeight);
			break;

		case Anaglyph:
			//drawBuffer(Back);
			glDrawBuffer(GL_BACK);
			//clear(DepthBufferBit);
			glClear(GL_DEPTH_BUFFER_BIT);
			glColorMask(1, 0, 0, 0);
			//colorMask(1, 0, 0, 0);
			//viewport(mLeft,mBottom, mWidth,mHeight);
			glViewport(mLeft, mBottom, mWidth, mHeight);
			break;

		case Dual:
			//viewport(mLeft, mBottom, mWidth/2, mHeight);
			glViewport(mLeft, mBottom, mWidth/2, mHeight);
			break;
			
		default:
			//drawBuffer(Back);
			glDrawBuffer(GL_BACK);
			//clear(ColorBufferBit | DepthBufferBit);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//viewport(mLeft,mBottom, mWidth,mHeight);
			glViewport(mLeft, mBottom, mWidth, mHeight);
	};
	
	setLookAt(-mStereoOffset[0], -mStereoOffset[1], -mStereoOffset[2]);
}


void Camera::right(){
	
	setFrustum(-eyeSep());

	switch(mMode){
		case Active:
			//drawBuffer(BackRight);
			glDrawBuffer(GL_BACK_RIGHT);
			//clear(ColorBufferBit | DepthBufferBit);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//viewport(mLeft,mBottom, mWidth,mHeight);
			glViewport(mLeft, mBottom, mWidth, mHeight);
			break;

		case Anaglyph:
			//drawBuffer(Back);
			glDrawBuffer(GL_BACK);
			//clear(DepthBufferBit);
			glClear(GL_DEPTH_BUFFER_BIT);
			glColorMask(0, 1, 1, 0);
			//colorMask(0,1,1,0);
			//viewport(mLeft,mBottom, mWidth,mHeight);
			glViewport(mLeft, mBottom, mWidth, mHeight);
			break;

		case Dual:
			//viewport(mLeft + mWidth/2, mBottom, mWidth/2, mHeight);
			glViewport(mLeft + mWidth/2, mBottom, mWidth/2, mHeight);
			break;
			
		default:
			//drawBuffer(Back);
			glDrawBuffer(GL_BACK);
			//clear(ColorBufferBit | DepthBufferBit);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//viewport(mLeft,mBottom, mWidth,mHeight);
			glViewport(mLeft, mBottom, mWidth, mHeight);
	};

	setLookAt(mStereoOffset[0], mStereoOffset[1], mStereoOffset[2]);
}

void Camera::setLookAt(double tx, double ty, double tz){

	//gl.matrixMode(gl.ModelView); 
	glMatrixMode(GL_MODELVIEW);
	//gl.identity();
	glLoadIdentity();
	
	//gl.lookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ)
	gluLookAt(tx + vec()[0], ty + vec()[1], tz + vec()[2],
			tx + (vec()[0] + uz()[0]*mFocalLength),
			ty + (vec()[1] + uz()[1]*mFocalLength),
			tz + (vec()[2] + uz()[2]*mFocalLength),
			uy()[0], uy()[1], uy()[2]);
}




//Frustumd& Camera::computeTestFrustum(){
//	// TODO: this should take the eye vector into account
//	Vec3d vl = pos()+vf();
//	mFrustum.setCamDef(pos(), vl, vu());
//	return mFrustum;
//}






} // al::
