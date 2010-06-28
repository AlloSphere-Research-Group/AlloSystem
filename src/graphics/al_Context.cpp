#include "graphics/al_Context.hpp"

using namespace al;

ViewPort :: ViewPort(double eyeSep) 
:	mEyeSep(eyeSep), mCamera(Camera::defaultCamera())
{
	dimensions(1,1,0,0);
	mUserProjectionTransform.set(Matrix4d::Identity());
}

ViewPort :: ~ViewPort() {

}

ViewPort& ViewPort::dimensions(double w, double h){ return dimensions(w,h,mLeft,mBottom); }

ViewPort& ViewPort::dimensions(double w, double h, double x, double y){
	mLeft=x; mBottom=y; mWidth=w; mHeight=h;
	//mAspect = w / h;
	return *this;
}


void ViewPort::view(double aspect, double eyesep) {
	
	Camera& cam		= camera();
	double near		= cam.near();
	double far		= cam.far();
	double focal	= cam.focalLength();
	double fovy		= cam.fovy();
	
	static const double deg2rad = 0.01745329252; // degree-to-radian over /2
	double top = near * tan(fovy * deg2rad * 0.5);	
	double right = aspect * top;
	
	// calculate stereo shift:
	double IOD = eyesep * focal/30.0;
	double shift = (IOD * 0.5)*near/focal;

	//gl.MatrixMode(gl.Projection);
	glMatrixMode(GL_PROJECTION);
	//gl.LoadIdentity();
	glLoadIdentity();
	glFrustum(
		-right + shift, 
		right + shift, 
		-top, 
		top, 
		near, 
		far
	);
//	Matrix4d m = Matrix4d::UnPerspective(
//		-right + shift, 
//		right + shift, 
//		-top, 
//		top, 
//		near, 
//		far);
//	glMultMatrixd(m.elems);
	//glMultMatrixd(userProjectionTransform().elems);
	
	// set perspective; uses cam(focal, vec, uz, uy, ux)
	//gl.MatrixMode(gl.ModelView);
	glMatrixMode(GL_MODELVIEW);
	//gl.LoadIdentity();
	glLoadIdentity();
	
	const Vec3d& vec = cam.vec();
	const Vec3d& ux  = cam.ux();
	const Vec3d& uy  = cam.uy();
	const Vec3d& uz  = cam.uz();

	const Vec3d& eye = vec + ux * IOD;
	const Vec3d& at  = eye + uz * focal;
	
	//gl.lookAt(	eye[0],	eye[1],	eye[2],
	//			at[0],	at[1],	at[2],
	//			uy[0],	uy[1],	uy[2]);
	gluLookAt(	eye[0],	eye[1],	eye[2],
				at[0],	at[1],	at[2],
				uy[0],	uy[1],	uy[2]);	
	

//	// Also, cache the frustum?:
//	const Vec3d& nc = eye - uz * mCamera->near();	// why negative?
//	const Vec3d& fc = eye - uz * mCamera->far();
//
//	mFrustum.ntl = nc + uy * mFrustum.nh - ux * mFrustum.nw;
//	mFrustum.ntr = nc + uy * mFrustum.nh + ux * mFrustum.nw;
//	mFrustum.nbl = nc - uy * mFrustum.nh - ux * mFrustum.nw;
//	mFrustum.nbr = nc - uy * mFrustum.nh + ux * mFrustum.nw;
//
//	mFrustum.ftl = fc + uy * mFrustum.fh - ux * mFrustum.fw;
//	mFrustum.ftr = fc + uy * mFrustum.fh + ux * mFrustum.fw;
//	mFrustum.fbl = fc - uy * mFrustum.fh - ux * mFrustum.fw;
//	mFrustum.fbr = fc - uy * mFrustum.fh + ux * mFrustum.fw;
//
//	mFrustum.pl[Frustumd::TOP].set3Points(	mFrustum.ntr,	mFrustum.ntl,	mFrustum.ftl);
//	mFrustum.pl[Frustumd::BOTTOM].set3Points(	mFrustum.nbl,	mFrustum.nbr,	mFrustum.fbr);
//	mFrustum.pl[Frustumd::LEFT].set3Points(	mFrustum.ntl,	mFrustum.nbl,	mFrustum.fbl);
//	mFrustum.pl[Frustumd::RIGHT].set3Points(	mFrustum.nbr,	mFrustum.ntr,	mFrustum.fbr);
//	mFrustum.pl[Frustumd::NEARP].set3Points(	mFrustum.ntl,	mFrustum.ntr,	mFrustum.nbr);
//	mFrustum.pl[Frustumd::FARP].set3Points(	mFrustum.ftr,	mFrustum.ftl,	mFrustum.fbl);
}


void Context :: draw(void (*draw)(void *), int width, int height, void * userdata) 
{
	if (mStereo) {
		switch (mMode) {
			case Anaglyph:
				drawAnaglyph(draw, width, height, userdata);
				break;
			case Active:
				drawActive(draw, width, height, userdata);
				break;
			case Dual:
				drawDual(draw, width, height, userdata);
				break;
			case LeftEye:
				drawLeft(draw, width, height, userdata);
				break;
			case RightEye:
				drawRight(draw, width, height, userdata);
				break;
			default:
				break;
		}
	} else {
		drawMono(draw, width, height, userdata);
	}
}

void Context :: drawMono(void (*draw)(void *), int width, int height, void * userdata) 
{
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	
	// TODO: for each viewport
	ViewPort& vp = viewport();
	{
		//enable(ScissorTest);
		glEnable(GL_SCISSOR_TEST);
		//scissor(vp.left(),vp.bottom(), width * vp.width(), height * vp.height());
		glScissor(vp.left(), vp.bottom(), width * vp.width(), height * vp.height());
		//viewport(vp.left(),vp.bottom(), width * vp.width(), height * vp.height());
		glViewport(vp.left(), vp.bottom(), width * vp.width(), height * vp.height());
		double aspect = (width * vp.width())/(height * vp.height());
		
		//drawBuffer(Back);
		glDrawBuffer(GL_BACK);
		//clear(ColorBufferBit | DepthBufferBit);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		vp.view(aspect, 0);
		draw(userdata);
		
		//disable(ScissorTest);
		glDisable(GL_SCISSOR_TEST);
	}
	
	//popAttrib()
	glPopAttrib();
}

void Context :: drawActive(void (*draw)(void *), int width, int height, void * userdata) 
{
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	
	// TODO: for each viewport
	ViewPort& vp = viewport();
	{
		//enable(ScissorTest);
		glEnable(GL_SCISSOR_TEST);
		//scissor(vp.left(),vp.bottom(), width * vp.width(), height * vp.height());
		glScissor(vp.left(), vp.bottom(), width * vp.width(), height * vp.height());
		//viewport(vp.left(),vp.bottom(), width * vp.width(), height * vp.height());
		glViewport(vp.left(), vp.bottom(), width * vp.width(), height * vp.height());
		double aspect = (width * vp.width())/(height * vp.height());
		
		//drawBuffer(BackLeft);
		glDrawBuffer(GL_BACK_LEFT);
		//clear(ColorBufferBit | DepthBufferBit);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		vp.view(aspect, -1);
		draw(userdata);
		
		
		//drawBuffer(BackRight);
		glDrawBuffer(GL_BACK_RIGHT);
		//clear(ColorBufferBit | DepthBufferBit);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		vp.view(aspect, 1);
		draw(userdata);
		
		//disable(ScissorTest);
		glDisable(GL_SCISSOR_TEST);
	}
	
	//popAttrib()
	glPopAttrib();
}

void Context :: drawAnaglyph(void (*draw)(void *), int width, int height, void * userdata) 
{
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	
	// TODO: for each viewport
	ViewPort& vp = viewport();
	{
		//enable(ScissorTest);
		glEnable(GL_SCISSOR_TEST);
		//scissor(vp.left(),vp.bottom(), width * vp.width(), height * vp.height());
		glScissor(vp.left(), vp.bottom(), width * vp.width(), height * vp.height());
		//viewport(vp.left(),vp.bottom(), width * vp.width(), height * vp.height());
		glViewport(vp.left(), vp.bottom(), width * vp.width(), height * vp.height());
		double aspect = (width * vp.width())/(height * vp.height());
		
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
		
		vp.view(aspect, -1);
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
		
		vp.view(aspect, 1);
		draw(userdata);
		
		
		//disable(ScissorTest);
		glDisable(GL_SCISSOR_TEST);
	}
	
	//colorMask(1,1,1,1);
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	
	//popAttrib()
	glPopAttrib();
}

void Context :: drawDual(void (*draw)(void *), int width, int height, void * userdata) 
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
		//scissor(vp.left(),vp.bottom(), width * vp.width()/2, height * vp.height());
		glScissor(vp.left(), vp.bottom(), width * vp.width()/2, height * vp.height());
		//viewport(vp.left(),vp.bottom(), width * vp.width()/2, height * vp.height());
		glViewport(vp.left(), vp.bottom(), width * vp.width()/2, height * vp.height());
		double aspect = (width * vp.width()/2)/(height * vp.height());
		
		//clear(ColorBufferBit | DepthBufferBit);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		vp.view(aspect, -1);
		draw(userdata);
		
		
		//scissor(vp.left()+width * vp.width()/2, vp.bottom(), width * vp.width()/2, height * vp.height());
		glScissor(vp.left()+width * vp.width()/2, vp.bottom(), width * vp.width()/2, height * vp.height());
		//viewport(vp.left()+width * vp.width()/2, vp.bottom(), width * vp.width()/2, height * vp.height());
		glViewport(vp.left()+width * vp.width()/2, vp.bottom(), width * vp.width()/2, height * vp.height());
		
		
		//clear(ColorBufferBit | DepthBufferBit);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		vp.view(aspect, 1);
		draw(userdata);
		
		
		//disable(ScissorTest);
		glDisable(GL_SCISSOR_TEST);
	}
	
	//popAttrib()
	glPopAttrib();
}



void Context :: drawLeft(void (*draw)(void *), int width, int height, void * userdata) 
{
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	
	// TODO: for each viewport
	ViewPort& vp = viewport();
	{
		//enable(ScissorTest);
		glEnable(GL_SCISSOR_TEST);
		//scissor(vp.left(),vp.bottom(), width * vp.width(), height * vp.height());
		glScissor(vp.left(), vp.bottom(), width * vp.width(), height * vp.height());
		//viewport(vp.left(),vp.bottom(), width * vp.width(), height * vp.height());
		glViewport(vp.left(), vp.bottom(), width * vp.width(), height * vp.height());
		double aspect = (width * vp.width())/(height * vp.height());
		
		//drawBuffer(Back);
		glDrawBuffer(GL_BACK);
		//clear(ColorBufferBit | DepthBufferBit);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		vp.view(aspect, -1);
		draw(userdata);
		
		//disable(ScissorTest);
		glDisable(GL_SCISSOR_TEST);
	}
	
	//popAttrib()
	glPopAttrib();
}

void Context :: drawRight(void (*draw)(void *), int width, int height, void * userdata) 
{
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	
	// TODO: for each viewport
	ViewPort& vp = viewport();
	{
		//enable(ScissorTest);
		glEnable(GL_SCISSOR_TEST);
		//scissor(vp.left(),vp.bottom(), width * vp.width(), height * vp.height());
		glScissor(vp.left(), vp.bottom(), width * vp.width(), height * vp.height());
		//viewport(vp.left(),vp.bottom(), width * vp.width(), height * vp.height());
		glViewport(vp.left(), vp.bottom(), width * vp.width(), height * vp.height());
		double aspect = (width * vp.width())/(height * vp.height());
		
		//drawBuffer(Back);
		glDrawBuffer(GL_BACK);
		//clear(ColorBufferBit | DepthBufferBit);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		vp.view(aspect, 1);
		draw(userdata);
		
		//disable(ScissorTest);
		glDisable(GL_SCISSOR_TEST);
	}
	
	//popAttrib()
	glPopAttrib();
}

/// blue line sync for active stereo
/// @see http://local.wasp.uwa.edu.au/~pbourke/miscellaneous/stereographics/stereorender/GLUTStereo/glutStereo.cpp
void Context :: drawBlueLine(int window_width, int window_height) 
{
	GLint i;
	unsigned long buffer;
	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	for(i = 0; i < 6; i++) glDisable(GL_CLIP_PLANE0 + i);
	glDisable(GL_COLOR_LOGIC_OP);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DITHER);
	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_LINE_STIPPLE);
	glDisable(GL_SCISSOR_TEST);
	//glDisable(GL_SHARED_TEXTURE_PALETTE_EXT); /* not in 10.5 sdk */
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_3D);
	glDisable(GL_TEXTURE_CUBE_MAP);
	glDisable(GL_TEXTURE_RECTANGLE_EXT);
	glDisable(GL_VERTEX_PROGRAM_ARB);
		
	for(buffer = GL_BACK_LEFT; buffer <= GL_BACK_RIGHT; buffer++) {
		GLint matrixMode;
		GLint vp[4];
		
		glDrawBuffer(buffer);
		
		glGetIntegerv(GL_VIEWPORT, vp);
		glViewport(0, 0, window_width, window_height);
		
		glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glScalef(2.0f / window_width, -2.0f / window_height, 1.0f);
		glTranslatef(-window_width / 2.0f, -window_height / 2.0f, 0.0f);
	
		// draw sync lines
		glColor3d(0.0f, 0.0f, 0.0f);
		glBegin(GL_LINES); // Draw a background line
			glVertex3f(0.0f, window_height - 0.5f, 0.0f);
			glVertex3f(window_width, window_height - 0.5f, 0.0f);
		glEnd();
		glColor3d(0.0f, 0.0f, 1.0f);
		glBegin(GL_LINES); // Draw a line of the correct length (the cross over is about 40% across the screen from the left
			glVertex3f(0.0f, window_height - 0.5f, 0.0f);
			if(buffer == GL_BACK_LEFT)
				glVertex3f(window_width * 0.30f, window_height - 0.5f, 0.0f);
			else
				glVertex3f(window_width * 0.80f, window_height - 0.5f, 0.0f);
		glEnd();
	
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(matrixMode);
		
		glViewport(vp[0], vp[1], vp[2], vp[3]);
	}	
	glPopAttrib();

}






Frustumd ViewPort::monoFrustum() {

//	static double const tanCoef = 0.01745329252*0.5;	// degree-to-radian over /2
//	double fovy		 = mCamera->fovy();
//	double tanFOV = tan(fovy * tanCoef);
//	double aspect	 = mWidth/mHeight;
//	double near		 = mCamera->near();
//	double far		 = mCamera->far();
//	double focal	 = mCamera->focalLength();
//	double IOD		 = mEyeSep * focal/30.0;		// half of inter-ocular distance
//	
//	const Vec3d& pos = mCamera->vec();
//	const Vec3d& ux  = mCamera->ux();
//	const Vec3d& uy  = mCamera->uy();
//	const Vec3d& uz  = mCamera->uz();
//	
//	const Vec3d eye  = pos + (ux * IOD * 0.5);
//	const Vec3d at   = eye + (uz * focal);
//				
//	// Also, update the frustum:
//	const Vec3d& nc = eye - uz * mCamera->near();	// why negative?
//	const Vec3d& fc = eye - uz * mCamera->far();
//
	Frustumd f;
//	
//	f.nh = near * tanFOV;			
//	f.nw = f.nh * mAspect;			
//	f.fh = far  * tanFOV;	
//	f.fw = f.fh * mAspect;
//	
//	f.ntl = nc + uy * f.nh - ux * f.nw;
//	f.ntr = nc + uy * f.nh + ux * f.nw;
//	f.nbl = nc - uy * f.nh - ux * f.nw;
//	f.nbr = nc - uy * f.nh + ux * f.nw;
//
//	f.ftl = fc + uy * f.fh - ux * f.fw;
//	f.ftr = fc + uy * f.fh + ux * f.fw;
//	f.fbl = fc - uy * f.fh - ux * f.fw;
//	f.fbr = fc - uy * f.fh + ux * f.fw;
//	
//	f.pl[Frustumd::TOP].set3Points(	f.ntr,	f.ntl,	f.ftl);
//	f.pl[Frustumd::BOTTOM].set3Points(f.nbl,	f.nbr,	f.fbr);
//	f.pl[Frustumd::LEFT].set3Points(	f.ntl,	f.nbl,	f.fbl);
//	f.pl[Frustumd::RIGHT].set3Points(	f.nbr,	f.ntr,	f.fbr);
//	f.pl[Frustumd::NEARP].set3Points(	f.ntl,	f.ntr,	f.nbr);
//	f.pl[Frustumd::FARP].set3Points(	f.ftr,	f.ftl,	f.fbl);
	
	return f;
}
	






