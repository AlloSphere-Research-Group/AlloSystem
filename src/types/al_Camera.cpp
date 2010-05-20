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

#include "types/al_Camera.hpp"


namespace al{

void Nav :: toAED(const Vec3d & to, double azimuth, double elevation, double distance) {
	
	Vec3d rel = to - mPos;
	distance = rel.mag();
	
	if (distance > QUAT_EPSILON*2) 
	{
		rel.normalize();
		
		// dot product of A & B vectors is the similarity or cosine:
		double xness = rel.dot(mUX); 
		double yness = rel.dot(mUY);
		double zness = rel.dot(mUZ);
		
		azimuth = -atan2(xness, zness);
		elevation = asin(yness);
	} else {
		// near origin; might as well assume 0 to avoid denormals
		// do not set az/el; they may already have more meaningful values
		distance = 0.0;
	}

}

void Nav :: updateUnitVectors() {
	mQuat.toVectorX(mUX);
	mQuat.toVectorY(mUY);
	mQuat.toVectorZ(mUZ);
}

void Nav :: step() {
	// accumulate orientation:
	mQuat *= mVel.mQuat;
	updateUnitVectors();
	
	// accumulate position:
	for (int i=0; i<3; i++) {
		mPos[i] += mVel.mPos[0] * mUX[i] + mVel.mPos[1] * mUY[i] + mVel.mPos[2] * mUZ[i];
	}

}

void Nav :: step(double dt) {
	// accumulate orientation:
	Quatd q2(mQuat);
	q2 *= mVel.mQuat;
	mQuat.slerp(q2, dt);
	updateUnitVectors();
	
	// accumulate position:
	for (int i=0; i<3; i++) {
		mPos[i] += dt * (mVel.mPos[0] * mUX[i] + mVel.mPos[1] * mUY[i] + mVel.mPos[2] * mUZ[i]);
	}
}






Camera :: Camera(double aper, double nearClip, double farClip, double focalLen, double eyeSep)
:	mFocalLength(focalLen), mEyeSep(eyeSep), mNear(nearClip), mFar(farClip),
	mStereo(false), mMode(Anaglyph), 
	mZoom(0)
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
	mFrustumG.setCamInternals(aperture(), ratio(), near(), far());
}


Camera& Camera::aperture(double v){
	mAperture=v;
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

Camera::Frustum Camera::frustum(double sep) const{
	Frustum f;
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
	Frustum f = frustum(sep);

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
	gluLookAt(tx + mPos[0], ty + mPos[1], tz + mPos[2],
			tx + (mPos[0] + mUZ[0]*mFocalLength),
			ty + (mPos[1] + mUZ[1]*mFocalLength),
			tz + (mPos[2] + mUZ[2]*mFocalLength),
			mUY[0], mUY[1], mUY[2]);
}




FrustumG& Camera::computeTestFrustum(){
	// TODO: this should take the eye vector into account
	Vec3d vl = pos()+vf();
	mFrustumG.setCamDef(pos(), vl, vu());
	return mFrustumG;
}



void FrustumG::setCamInternals(float angle, float ratio, float nearD, float farD){
	mRatio = ratio;
	mAngle = angle;
	mNear = nearD;
	mFar = farD;

	static double const tanCoef = 0.01745329252*0.5;	// degree-to-radian over /2
	mTanFOV = (float)tan(mAngle * tanCoef);
	nh = mNear * mTanFOV;
	nw = nh * mRatio; 
	fh = mFar  * mTanFOV;
	fw = fh * mRatio;
}

void FrustumG::setCamDef(const Vec3d& p, const Vec3d& l, const Vec3d& u){

	Vec3d Z = (p-l).normalize();
	Vec3d X = cross(u,Z).normalize();
	Vec3d Y = cross(Z,X);

	Vec3d nc = p - Z * mNear;
	Vec3d fc = p - Z * mFar;

	ntl = nc + Y * nh - X * nw;
	ntr = nc + Y * nh + X * nw;
	nbl = nc - Y * nh - X * nw;
	nbr = nc - Y * nh + X * nw;

	ftl = fc + Y * fh - X * fw;
	ftr = fc + Y * fh + X * fw;
	fbl = fc - Y * fh - X * fw;
	fbr = fc - Y * fh + X * fw;

	pl[TOP].set3Points(ntr,ntl,ftl);
	pl[BOTTOM].set3Points(nbl,nbr,fbr);
	pl[LEFT].set3Points(ntl,nbl,fbl);
	pl[RIGHT].set3Points(nbr,ntr,fbr);
	pl[NEARP].set3Points(ntl,ntr,nbr);
	pl[FARP].set3Points(ftr,ftl,fbl);
}

int FrustumG::pointInFrustum(const Vec3d& p) const {
	int result = INSIDE;
	for(int i=0; i<6; ++i){
		if(pl[i].distance(p) < 0)
			return OUTSIDE;
	}
	return result;
}

int FrustumG::sphereInFrustum(const Vec3d& p, float raio) const {
	int result = INSIDE;
	for(int i=0; i<6; ++i){
		float distance = pl[i].distance(p);
		if(distance < -raio)
			return OUTSIDE;
		else if(distance < raio)
			result = INTERSECT;
	}
	return result;
}

//int FrustumG::boxInFrustum(AABox &b) const {
//	int result = INSIDE;
//	for(int i=0; i < 6; i++){
//		if(pl[i].distance(b.getVertexP(pl[i].normal)) < 0)
//			return OUTSIDE;
//		else if(pl[i].distance(b.getVertexN(pl[i].normal)) < 0)
//			result =  INTERSECT;
//	}
//	return result;
//}


} // al::
