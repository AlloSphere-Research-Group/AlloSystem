#include "graphics/al_Context.hpp"

using namespace al;

ViewPort :: ViewPort(double eyeSep) 
:	mEyeSep(eyeSep), mAspect(1), mCamera(Camera::defaultCamera())
{
	dimensions(4,3,0,0);
}

ViewPort :: ~ViewPort() {

}

ViewPort& ViewPort::dimensions(double w, double h){ return dimensions(w,h,mLeft,mBottom); }

ViewPort& ViewPort::dimensions(double w, double h, double x, double y){
	mLeft=x; mBottom=y; mWidth=w; mHeight=h;
	mAspect = mWidth / mHeight;
	return *this;
}

//void ViewPort::calcFrustum(){
//	mRatio = mWidth / (mHeight>0 ? mHeight : 1e-10);
//
//	if(stereo() && (mode() == Dual)) 
//		mRatio /= 2;		// assume screen space is twice the width of the display
//
//	double zm = pow(2, -zoom());
//	mNearTop = near() * mTanFOV * zm;
//	mFarTop = far() * mTanFOV * zm;
//	mNearOverFocalLength = near() / (focalLength() * 2.) * zm;
//
//	// Derive the eye offset vector
//	mStereoOffset = mUX * (eyeSep()*0.5);
//
//	// TODO: this is redundant
//	mFrustum.setCamInternals(aperture(), ratio(), near(), far());
//}

//void ViewPort::setLookAt(double tx, double ty, double tz) {
//
//	//gl.matrixMode(gl.ModelView); 
//	glMatrixMode(GL_MODELVIEW);
//	//gl.identity();
//	glLoadIdentity();
//	
//	const Vec3d& vec = mCamera->vec();
//	const Vec3d& uy  = mCamera->uy();
//	const Vec3d& uz  = mCamera->uz();
//	const double focal = mCamera->focalLength();
//	
//	//gl.lookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ)
//	gluLookAt(tx + vec[0], ty + vec[1], tz + vec[2],
//			tx + (vec[0] + uz[0]*focal),
//			ty + (vec[1] + uz[1]*focal),
//			tz + (vec[2] + uz[2]*focal),
//			uy[0], uy[1], uy[2]);
//}


void ViewPort::applyFrustumStereo(double eyesep) {
	
	double aspect	 = mWidth/mHeight;
	double near		 = mCamera->near();
	double far		 = mCamera->far();
	double focal	 = mCamera->focalLength();
	double fovy		 = mCamera->fovy();
	const Vec3d& vec = mCamera->vec();
	const Vec3d& ux  = mCamera->ux();
	const Vec3d& uy  = mCamera->uy();
	const Vec3d& uz  = mCamera->uz();
	
	// typical inter-ocular distance is about 1/30 of the focal length
	double IOD = eyesep * focal/30.0;				// half of inter-ocular distance
	
	static double const tanCoef = 0.01745329252*0.5;	// degree-to-radian over /2
	double tanFOV = tan(fovy * tanCoef);
	mFrustum.nh = near * tanFOV;			// i.e., top.
	mFrustum.nw = mFrustum.nh * mAspect;				// i.e. right
	mFrustum.fh = far  * tanFOV;	
	mFrustum.fw = mFrustum.fh * mAspect;
	
	double top = mFrustum.nh;	
	double right = mFrustum.nw;				
	double shift = (IOD * 0.5)*near/focal;
	const Vec3d eye = vec + (ux * IOD);
	const Vec3d at = eye + (uz * focal);
	
	double l, r, t, b, n, f;
	t = top;
	b = -top;
	l = -right + shift;
	r = right + shift;
	n = near;
	f = far;

	const Vec3d& v = at;
	printf("%f %f %f\n", v[0], v[1], v[2]);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();                                       //reset projection matrix
	glFrustum(l, r, b, t, n, f);
	
}

Frustumd ViewPort::monoFrustum() {

	static double const tanCoef = 0.01745329252*0.5;	// degree-to-radian over /2
	double fovy		 = mCamera->fovy();
	double tanFOV = tan(fovy * tanCoef);
	double aspect	 = mWidth/mHeight;
	double near		 = mCamera->near();
	double far		 = mCamera->far();
	double focal	 = mCamera->focalLength();
	double IOD		 = mEyeSep * focal/30.0;		// half of inter-ocular distance
	
	const Vec3d& pos = mCamera->vec();
	const Vec3d& ux  = mCamera->ux();
	const Vec3d& uy  = mCamera->uy();
	const Vec3d& uz  = mCamera->uz();
	
	const Vec3d eye  = pos + (ux * IOD * 0.5);
	const Vec3d at   = eye + (uz * focal);
				
	// Also, update the frustum:
	const Vec3d& nc = eye - uz * mCamera->near();	// why negative?
	const Vec3d& fc = eye - uz * mCamera->far();

	Frustumd f;
	
	f.nh = near * tanFOV;			
	f.nw = f.nh * mAspect;			
	f.fh = far  * tanFOV;	
	f.fw = f.fh * mAspect;
	
	f.ntl = nc + uy * f.nh - ux * f.nw;
	f.ntr = nc + uy * f.nh + ux * f.nw;
	f.nbl = nc - uy * f.nh - ux * f.nw;
	f.nbr = nc - uy * f.nh + ux * f.nw;

	f.ftl = fc + uy * f.fh - ux * f.fw;
	f.ftr = fc + uy * f.fh + ux * f.fw;
	f.fbl = fc - uy * f.fh - ux * f.fw;
	f.fbr = fc - uy * f.fh + ux * f.fw;
	
	f.pl[Frustumd::TOP].set3Points(	f.ntr,	f.ntl,	f.ftl);
	f.pl[Frustumd::BOTTOM].set3Points(f.nbl,	f.nbr,	f.fbr);
	f.pl[Frustumd::LEFT].set3Points(	f.ntl,	f.nbl,	f.fbl);
	f.pl[Frustumd::RIGHT].set3Points(	f.nbr,	f.ntr,	f.fbr);
	f.pl[Frustumd::NEARP].set3Points(	f.ntl,	f.ntr,	f.nbr);
	f.pl[Frustumd::FARP].set3Points(	f.ftr,	f.ftl,	f.fbl);
	
	return f;
}
	


void ViewPort::view(double eyesep) {
	double focal	 = mCamera->focalLength();
	
	// typical inter-ocular distance is about 1/30 of the focal length
	double IOD = eyesep * eyeSep() * focal/30.0;		// half of inter-ocular distance
	
	const Vec3d& pos = mCamera->vec();
	const Vec3d& ux  = mCamera->ux();
	const Vec3d& uy  = mCamera->uy();
	const Vec3d& uz  = mCamera->uz();
	
	const Vec3d eye  = pos + (ux * IOD * 0.5);
	const Vec3d at   = eye + (uz * focal);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(	eye[0],	eye[1],	eye[2],
				at[0],	at[1],	at[2],
				uy[0],	uy[1],	uy[2]);	
				
	// Also, update the frustum:
	const Vec3d& nc = eye - uz * mCamera->near();	// why negative?
	const Vec3d& fc = eye - uz * mCamera->far();

	mFrustum.ntl = nc + uy * mFrustum.nh - ux * mFrustum.nw;
	mFrustum.ntr = nc + uy * mFrustum.nh + ux * mFrustum.nw;
	mFrustum.nbl = nc - uy * mFrustum.nh - ux * mFrustum.nw;
	mFrustum.nbr = nc - uy * mFrustum.nh + ux * mFrustum.nw;

	mFrustum.ftl = fc + uy * mFrustum.fh - ux * mFrustum.fw;
	mFrustum.ftr = fc + uy * mFrustum.fh + ux * mFrustum.fw;
	mFrustum.fbl = fc - uy * mFrustum.fh - ux * mFrustum.fw;
	mFrustum.fbr = fc - uy * mFrustum.fh + ux * mFrustum.fw;

	mFrustum.pl[Frustumd::TOP].set3Points(	mFrustum.ntr,	mFrustum.ntl,	mFrustum.ftl);
	mFrustum.pl[Frustumd::BOTTOM].set3Points(	mFrustum.nbl,	mFrustum.nbr,	mFrustum.fbr);
	mFrustum.pl[Frustumd::LEFT].set3Points(	mFrustum.ntl,	mFrustum.nbl,	mFrustum.fbl);
	mFrustum.pl[Frustumd::RIGHT].set3Points(	mFrustum.nbr,	mFrustum.ntr,	mFrustum.fbr);
	mFrustum.pl[Frustumd::NEARP].set3Points(	mFrustum.ntl,	mFrustum.ntr,	mFrustum.nbr);
	mFrustum.pl[Frustumd::FARP].set3Points(	mFrustum.ftr,	mFrustum.ftl,	mFrustum.fbl);
}


void Context :: draw(void (*draw)(void *), void * userdata) 
{
	if (mStereo) {
		switch (mMode) {
			case Anaglyph:
				drawAnaglyph(draw, userdata);
				break;
			case Active:
				drawActive(draw, userdata);
				break;
			case Dual:
				drawDual(draw, userdata);
				break;
			case LeftEye:
				drawLeft(draw, userdata);
				break;
			case RightEye:
				drawRight(draw, userdata);
				break;
			default:
				break;
		}
	} else {
		drawMono(draw, userdata);
	}
}

void Context :: drawMono(void (*draw)(void *), void * userdata) 
{
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	
	// TODO: for each viewport
	ViewPort& vp = viewport();
	{
		//enable(ScissorTest);
		glEnable(GL_SCISSOR_TEST);
		//scissor(vp.left(),vp.bottom(), vp.width(),vp.height());
		glScissor(vp.left(), vp.bottom(), vp.width(), vp.height());
		//viewport(vp.left(),vp.bottom(), vp.width(),vp.height());
		glViewport(vp.left(), vp.bottom(), vp.width(), vp.height());
		
		//drawBuffer(Back);
		glDrawBuffer(GL_BACK);
		//clear(ColorBufferBit | DepthBufferBit);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		//setFrustum(0);
		//setLookAt(0);
		draw(userdata);
		
		//disable(ScissorTest);
		glDisable(GL_SCISSOR_TEST);
	}
	
	//popAttrib()
	glPopAttrib();
}

void Context :: drawActive(void (*draw)(void *), void * userdata) 
{
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	
	// TODO: for each viewport
	ViewPort& vp = viewport();
	{
		//enable(ScissorTest);
		glEnable(GL_SCISSOR_TEST);
		//scissor(vp.left(),vp.bottom(), vp.width(),vp.height());
		glScissor(vp.left(), vp.bottom(), vp.width(), vp.height());
		//viewport(vp.left(),vp.bottom(), vp.width(),vp.height());
		glViewport(vp.left(), vp.bottom(), vp.width(), vp.height());
		
		//drawBuffer(BackLeft);
		glDrawBuffer(GL_BACK_LEFT);
		//clear(ColorBufferBit | DepthBufferBit);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		//setFrustum(LEFT);
		//setLookAt(LEFT);
		draw(userdata);
		
		
		//drawBuffer(BackRight);
		glDrawBuffer(GL_BACK_RIGHT);
		//clear(ColorBufferBit | DepthBufferBit);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//setFrustum(RIGHT);
		//setLookAt(RIGHT);
		draw(userdata);
		
		//disable(ScissorTest);
		glDisable(GL_SCISSOR_TEST);
	}
	
	//popAttrib()
	glPopAttrib();
}

void Context :: drawAnaglyph(void (*draw)(void *), void * userdata) 
{
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	
	// TODO: for each viewport
	ViewPort& vp = viewport();
	{
		//enable(ScissorTest);
		glEnable(GL_SCISSOR_TEST);
		//scissor(vp.left(),vp.bottom(), vp.width(),vp.height());
		glScissor(vp.left(), vp.bottom(), vp.width(), vp.height());
		//viewport(vp.left(),vp.bottom(), vp.width(),vp.height());
		glViewport(vp.left(), vp.bottom(), vp.width(), vp.height());
		
		//drawBuffer(BackLeft);
		glDrawBuffer(GL_BACK);
		//clear(ColorBufferBit);
		glClear(GL_COLOR_BUFFER_BIT);
		
		
		switch (mAnaglyphMode) {
			case RedBlue:
			case RedGreen:
			case RedCyan:
				//colorMask(1, 0, 0, 1);
				glColorMask(GL_TRUE,GL_FALSE,GL_FALSE,GL_TRUE);
			break;
			case BlueRed:
				//colorMask(0, 0, 1, 1);
				glColorMask(GL_FALSE,GL_FALSE,GL_TRUE,GL_TRUE);
			break;
			case GreenRed:
				//colorMask(0, 1, 0, 1);
				glColorMask(GL_FALSE,GL_TRUE,GL_FALSE,GL_TRUE);
			break;
			case CyanRed:
				//colorMask(0, 1, 1, 1);
				glColorMask(GL_FALSE,GL_TRUE,GL_TRUE,GL_TRUE);
			break;
			default:
				//colorMask(1, 1, 1, 1);
				glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
		} 
		//clear(DepthBufferBit);
		glClear(GL_DEPTH_BUFFER_BIT);
		
		//setFrustum(LEFT);
		//setLookAt(LEFT);
		draw(userdata);
		
		switch (mAnaglyphMode) {
			case RedBlue:
				//colorMask(0, 1, 0, 1);
				glColorMask(GL_FALSE,GL_FALSE,GL_TRUE,GL_TRUE);
			break;
			case RedGreen:
				//colorMask(0, 1, 0, 1);
				glColorMask(GL_FALSE,GL_TRUE,GL_FALSE,GL_TRUE);
			break;
			case RedCyan:
				//colorMask(0, 1, 1, 1);
				glColorMask(GL_FALSE,GL_TRUE,GL_TRUE,GL_TRUE);
			break;
			case BlueRed:
			case GreenRed:
			case CyanRed:
				//colorMask(1, 0, 0, 1);
				glColorMask(GL_TRUE,GL_FALSE,GL_FALSE,GL_TRUE);
			break;
			default:
				//colorMask(1, 1, 1, 1);
				glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
		} 
		//clear(DepthBufferBit);
		glClear(GL_DEPTH_BUFFER_BIT);
		
		//setFrustum(RIGHT);
		//setLookAt(RIGHT);
		draw(userdata);
		
		
		//disable(ScissorTest);
		glDisable(GL_SCISSOR_TEST);
	}
	
	//colorMask(1,1,1,1);
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	
	//popAttrib()
	glPopAttrib();
}

void Context :: drawDual(void (*draw)(void *), void * userdata) 
{
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	
	//drawBuffer(BackLeft);
	glDrawBuffer(GL_BACK);
	
	// TODO: for each viewport
	ViewPort& vp = viewport();
	{
		
		//enable(ScissorTest);
		glEnable(GL_SCISSOR_TEST);
		//scissor(vp.left(),vp.bottom(), vp.width()/2,vp.height());
		glScissor(vp.left(), vp.bottom(), vp.width()/2, vp.height());
		//viewport(vp.left(),vp.bottom(), vp.width()/2,vp.height());
		glViewport(vp.left(), vp.bottom(), vp.width()/2, vp.height());
		
		//clear(ColorBufferBit | DepthBufferBit);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//setFrustum(LEFT);
		//setLookAt(LEFT);
		draw(userdata);
		
		
		//scissor(vp.left()+vp.width()/2,vp.bottom(), vp.width()/2,vp.height());
		glScissor(vp.left()+vp.width()/2, vp.bottom(), vp.width()/2, vp.height());
		//viewport(vp.left()+vp.width()/2,vp.bottom(), vp.width()/2,vp.height());
		glViewport(vp.left()+vp.width()/2, vp.bottom(), vp.width()/2, vp.height());
		
		
		//clear(ColorBufferBit | DepthBufferBit);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//setFrustum(RIGHT);
		//setLookAt(RIGHT);
		draw(userdata);
		
		
		//disable(ScissorTest);
		glDisable(GL_SCISSOR_TEST);
	}
	
	//popAttrib()
	glPopAttrib();
}



void Context :: drawLeft(void (*draw)(void *), void * userdata) 
{
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	
	// TODO: for each viewport
	ViewPort& vp = viewport();
	{
		//enable(ScissorTest);
		glEnable(GL_SCISSOR_TEST);
		//scissor(vp.left(),vp.bottom(), vp.width(),vp.height());
		glScissor(vp.left(), vp.bottom(), vp.width(), vp.height());
		//viewport(vp.left(),vp.bottom(), vp.width(),vp.height());
		glViewport(vp.left(), vp.bottom(), vp.width(), vp.height());
		
		//drawBuffer(Back);
		glDrawBuffer(GL_BACK);
		//clear(ColorBufferBit | DepthBufferBit);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		//setFrustum(LEFT);
		//setLookAt(LEFT);
		draw(userdata);
		
		//disable(ScissorTest);
		glDisable(GL_SCISSOR_TEST);
	}
	
	//popAttrib()
	glPopAttrib();
}

void Context :: drawRight(void (*draw)(void *), void * userdata) 
{
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	
	// TODO: for each viewport
	ViewPort& vp = viewport();
	{
		//enable(ScissorTest);
		glEnable(GL_SCISSOR_TEST);
		//scissor(vp.left(),vp.bottom(), vp.width(),vp.height());
		glScissor(vp.left(), vp.bottom(), vp.width(), vp.height());
		//viewport(vp.left(),vp.bottom(), vp.width(),vp.height());
		glViewport(vp.left(), vp.bottom(), vp.width(), vp.height());
		
		//drawBuffer(Back);
		glDrawBuffer(GL_BACK);
		//clear(ColorBufferBit | DepthBufferBit);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		//setFrustum(RIGHT);
		//setLookAt(RIGHT);
		draw(userdata);
		
		//disable(ScissorTest);
		glDisable(GL_SCISSOR_TEST);
	}
	
	//popAttrib()
	glPopAttrib();
}











