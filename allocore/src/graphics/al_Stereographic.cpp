#include "allocore/graphics/al_Stereographic.hpp"
#include "allocore/graphics/al_Graphics.hpp"	/* << need the OpenGL headers */

namespace al{


void Stereographic::pushDrawPop(Graphics& gl, Drawable& draw){
	gl.pushMatrix(gl.PROJECTION);
	gl.loadMatrix(projection());
	gl.pushMatrix(gl.MODELVIEW);
	gl.loadMatrix(modelView());
		draw.onDraw(gl);
	gl.popMatrix(gl.PROJECTION);
	gl.popMatrix(gl.MODELVIEW);
}

void Stereographic::sendViewport(Graphics& gl, const Viewport& vp){
	glScissor(vp.l, vp.b, vp.w, vp.h);
	gl.viewport(vp.l, vp.b, vp.w, vp.h);
	mVP = vp;
}

void Stereographic::sendClear(Graphics& gl){
	gl.depthMask(1);
	gl.clearColor(mClearColor);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
}

void Stereographic :: draw(Graphics& gl, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear, double pixelaspect) {
	//printf("%d, %d\n", mStereo, mMode);
	if(mStereo){
		switch(mMode){
			case ANAGLYPH:	drawAnaglyph(gl, lens, pose, vp, draw, clear, pixelaspect); return;
			case ACTIVE:	drawActive	(gl, lens, pose, vp, draw, clear, pixelaspect); return;
			case DUAL:		drawDual	(gl, lens, pose, vp, draw, clear, pixelaspect); return;
			case LEFT_EYE:	drawLeft	(gl, lens, pose, vp, draw, clear, pixelaspect); return;
			case RIGHT_EYE:	drawRight	(gl, lens, pose, vp, draw, clear, pixelaspect); return;
			default:;
		}
	} else {
		drawMono(gl, lens, pose, vp, draw, clear, pixelaspect);
	}
}

void Stereographic :: drawMono(Graphics& gl, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear, double pixelaspect) 
{
	double near = lens.near();
	double far = lens.far();
	const Vec3d& pos = pose.pos();
	
	//glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);

	// We must configure scissoring BEFORE clearing buffers
	glEnable(GL_SCISSOR_TEST);
	sendViewport(gl, vp);

	// glDrawBuffer(GL_BACK);	// << breaks usage under FBO
	if(clear) sendClear(gl);
	
	mEye = pos;
	
	if (omni()) {
		int wx = vp.l;
		double fovx = mOmniFov;
		for (unsigned i=0; i<mSlices; i++) {
			// angle at center of slice:
			double angle = fovx * (0.5-((i+0.5)/(double)(mSlices)));
			
			int wx1 = vp.l + vp.w * (i+1)/(double)mSlices;
			Viewport vp1(wx, vp.b, wx1-wx, vp.h);
			double aspect = vp1.aspect() * pixelaspect; 
			double fovy = Lens::getFovyForFovX(fovx * (vp1.w)/(double)vp.w, aspect); 
			
			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mProjection = Matrix4d::perspective(fovy, aspect, near, far);
			mModelView = Matrix4d::lookAt(ux, uy, uz, mEye);
			
			sendViewport(gl, vp1);
			if(clear) sendClear(gl);
			
			pushDrawPop(gl,draw);
			
			wx = wx1;
		}
		
	} else {
		
		double fovy = lens.fovy();
		double aspect = vp.aspect() * pixelaspect;
		Vec3d ux, uy, uz; pose.unitVectors(ux, uy, uz);
		mProjection = Matrix4d::perspective(fovy, aspect, near, far);
		mModelView = Matrix4d::lookAt(ux, uy, uz, mEye);

		pushDrawPop(gl,draw);
	}

	//glPopAttrib();
	glDisable(GL_SCISSOR_TEST);
}

void Stereographic :: drawAnaglyph(Graphics& gl, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear, double pixelaspect) 
{	
	double near = lens.near();
	double far = lens.far();
	double focal = lens.focalLength();
	double iod = lens.eyeSep();
	const Vec3d& pos = pose.pos();

	//glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);

	// We must configure scissoring BEFORE clearing buffers
	glEnable(GL_SCISSOR_TEST);
	sendViewport(gl, vp);

	// glDrawBuffer(GL_BACK); //<< this breaks usage within FBO.
	if(clear) sendClear(gl);
	
	switch(mAnaglyphMode){
		case RED_BLUE:
		case RED_GREEN:
		case RED_CYAN:	glColorMask(GL_TRUE, GL_FALSE,GL_FALSE,GL_TRUE); break;
		case BLUE_RED:	glColorMask(GL_FALSE,GL_FALSE,GL_TRUE, GL_TRUE); break;
		case GREEN_RED:	glColorMask(GL_FALSE,GL_TRUE, GL_FALSE,GL_TRUE); break;
		case CYAN_RED:	glColorMask(GL_FALSE,GL_TRUE, GL_TRUE, GL_TRUE); break;
		default:		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE ,GL_TRUE);
	} 

	if (omni()) {
		int wx = vp.l;
		double fovx = mOmniFov;
		for (unsigned i=0; i<mSlices; i++) {
			// angle at center of slice:
			double angle = fovx * (0.5-((i+0.5)/(double)(mSlices)));
			
			int wx1 = vp.l + vp.w * (i+1)/(double)mSlices;
			Viewport vp1(wx, vp.b, wx1-wx, vp.h);
			double aspect = vp1.aspect() * pixelaspect; 
			double fovy = Lens::getFovyForFovX(fovx * (vp1.w)/(double)vp.w, aspect); 
			
			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mEye = pos + (ux * -iod);	// right
			mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
			mModelView = Matrix4d::lookAt(ux, uy, uz, mEye);
		
			sendViewport(gl, vp1);
			if(clear) sendClear(gl);

			pushDrawPop(gl,draw);
			
			wx = wx1;
		}
	} else {
		double fovy = lens.fovy();
		double aspect = vp.aspect() * pixelaspect;		
		Vec3d ux, uy, uz; 
		pose.unitVectors(ux, uy, uz);
		
		// apply camera transform:
		mEye = pos + (ux * -iod);	// right
		mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
		mModelView = Matrix4d::lookAt(ux, uy, uz, mEye);
		
		pushDrawPop(gl,draw);
	}
	
	switch(mAnaglyphMode){
		case RED_BLUE:	glColorMask(GL_FALSE,GL_FALSE,GL_TRUE, GL_TRUE); break;
		case RED_GREEN:	glColorMask(GL_FALSE,GL_TRUE, GL_FALSE,GL_TRUE); break;
		case RED_CYAN:	glColorMask(GL_FALSE,GL_TRUE, GL_TRUE, GL_TRUE); break;
		case BLUE_RED:
		case GREEN_RED:
		case CYAN_RED:	glColorMask(GL_TRUE, GL_FALSE,GL_FALSE,GL_TRUE); break;
		default:		glColorMask(GL_TRUE, GL_TRUE ,GL_TRUE, GL_TRUE);
	} 
	
	if (omni()) {
		int wx = vp.l;
		double fovx = mOmniFov;
		for (unsigned i=0; i<mSlices; i++) {
			// angle at center of slice:
			double angle = fovx * (0.5-((i+0.5)/(double)(mSlices)));
			
			int wx1 = vp.l + vp.w * (i+1)/(double)mSlices;
			Viewport vp1(wx, vp.b, wx1-wx, vp.h);
			double aspect = vp1.aspect() * pixelaspect; 
			double fovy = Lens::getFovyForFovX(fovx * (vp1.w)/(double)vp.w, aspect); 
			
			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mEye = pos + (ux * iod);	// left
			mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
			mModelView = Matrix4d::lookAt(ux, uy, uz, mEye);
		
			sendViewport(gl, vp1);
			if (clear){
				gl.depthMask(1);
				gl.clear(gl.DEPTH_BUFFER_BIT);				// Note: depth only this pass
			}			

			pushDrawPop(gl,draw);
			
			wx = wx1;
		}
	} else {
		double fovy = lens.fovy();
		double aspect = vp.aspect() * pixelaspect;		
		Vec3d ux, uy, uz; 
		pose.unitVectors(ux, uy, uz);
		
		// Only clear depth buffer for this eye
		if (clear){
			gl.depthMask(1);
			gl.clear(gl.DEPTH_BUFFER_BIT);				// Note: depth only this pass
		}
	
		// apply camera transform:
		mEye = pos + (ux * iod);	// left
		mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
		mModelView = Matrix4d::lookAt(ux, uy, uz, mEye);

		pushDrawPop(gl,draw);
	}
	
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	glDisable(GL_SCISSOR_TEST);
	//glPopAttrib();
}



void Stereographic :: drawActive(Graphics& gl, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear, double pixelaspect) 
{
	double near = lens.near();
	double far = lens.far();
	double focal = lens.focalLength();
	double iod = lens.eyeSep();
	const Vec3d& pos = pose.pos();

	//glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);

	// We must configure scissoring BEFORE clearing buffers
	glEnable(GL_SCISSOR_TEST);
	sendViewport(gl, vp);

	glDrawBuffer(GL_BACK_RIGHT);
	if(clear) sendClear(gl);
	
	if (omni()) {
	
		int wx = vp.l;
		double fovx = mOmniFov;
		for (unsigned i=0; i<mSlices; i++) {
			// angle at center of slice:
			double angle = fovx * (0.5-((i+0.5)/(double)(mSlices)));
			
			int wx1 = vp.l + vp.w * (i+1)/(double)mSlices;
			Viewport vp1(wx, vp.b, wx1-wx, vp.h);
			double aspect = vp1.aspect() * pixelaspect; 
			double fovy = Lens::getFovyForFovX(fovx * (vp1.w)/(double)vp.w, aspect); 

			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mEye = pos + (ux * -iod);	// right
			mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
			mModelView = Matrix4d::lookAt(ux, uy, uz, mEye);
			
			sendViewport(gl, vp1);
			if(clear) sendClear(gl);
			
			pushDrawPop(gl,draw);
			
			wx = wx1;
		}
		
	} else {
		double fovy = lens.fovy();
		double aspect = vp.aspect() * pixelaspect;
		Vec3d ux, uy, uz; pose.unitVectors(ux, uy, uz);
		mEye = pos + (ux * -iod);	// right
		mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
		mModelView = Matrix4d::lookAtRight(ux, uy, uz, mEye, iod);

		pushDrawPop(gl,draw);
	}
	
	
	glDrawBuffer(GL_BACK_LEFT);
	if(clear) sendClear(gl);
	
	if (omni()) {
	
		int wx = vp.l;
		double fovx = mOmniFov;
		for (unsigned i=0; i<mSlices; i++) {
			// angle at center of slice:
			double angle = fovx * (0.5-((i+0.5)/(double)(mSlices)));
			
			int wx1 = vp.l + vp.w * (i+1)/(double)mSlices;
			Viewport vp1(wx, vp.b, wx1-wx, vp.h);
			double aspect = vp1.aspect() * pixelaspect; 
			double fovy = Lens::getFovyForFovX(fovx * (vp1.w)/(double)vp.w, aspect); 
			
			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mEye = pos + (ux * iod);	// left
			mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
			mModelView = Matrix4d::lookAt(ux, uy, uz, mEye);
	
			sendViewport(gl, vp1);
			if(clear) sendClear(gl);
			
			pushDrawPop(gl,draw);
			
			wx = wx1;
		}
		
	} else {
		double fovy = lens.fovy();
		double aspect = vp.aspect() * pixelaspect;
		Vec3d ux, uy, uz; pose.unitVectors(ux, uy, uz);
		mEye = pos + (ux * iod);	// left
		mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
		mModelView = Matrix4d::lookAtLeft(ux, uy, uz, mEye, iod);

		pushDrawPop(gl,draw);
	}
	
	glDrawBuffer(GL_BACK);

	//glPopAttrib();
	glDisable(GL_SCISSOR_TEST);
}

void Stereographic :: drawDual(Graphics& gl, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear, double pixelaspect) 
{
	double fovy = lens.fovy();
	double near = lens.near();
	double far = lens.far();
	double focal = lens.focalLength();
	double iod = lens.eyeSep();
	double aspect = vp.aspect() * pixelaspect;
	const Vec3d& pos = pose.pos();
	Vec3d ux, uy, uz; pose.unitVectors(ux, uy, uz);
	
	aspect *= 0.5;	// for split view

	//glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);

	glEnable(GL_SCISSOR_TEST);
	sendViewport(gl, vp);

	// glDrawBuffer(GL_BACK); //<< this breaks usage within FBO.
	if(clear) sendClear(gl);
	
	Viewport vpleft(vp.l, vp.b, vp.w*0.5, vp.h);
	Viewport vpright(vp.l + vp.w*0.5, vp.b, vp.w*0.5, vp.h);
	
	if (omni()) {
		int wx = vpleft.l;
		double fovx = mOmniFov;
		for (unsigned i=0; i<mSlices; i++) {
			// angle at center of slice:
			double angle = fovx * (0.5-((i+0.5)/(double)(mSlices)));
			
			int wx1 = vpleft.l + vpleft.w * (i+1)/(double)mSlices;
			Viewport vp1(wx, vpleft.b, wx1-wx, vpleft.h);
			double aspect = vp1.aspect() * pixelaspect; 
			double fovy = Lens::getFovyForFovX(fovx * (vp1.w)/(double)vpleft.w, aspect); 
			
			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mEye = pos + (ux * iod);	// left
			mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
			mModelView = Matrix4d::lookAt(ux, uy, uz, mEye);
			
			sendViewport(gl, vp1);
			if(clear) sendClear(gl);
			
			pushDrawPop(gl,draw);
			
			wx = wx1;
		}
		
		wx = vpright.l;
		for (unsigned i=0; i<mSlices; i++) {
			// angle at center of slice:
			double angle = fovx * (0.5-((i+0.5)/(double)(mSlices)));
			
			int wx1 = vpright.l + vpright.w * (i+1)/(double)mSlices;
			Viewport vp1(wx, vpright.b, wx1-wx, vpright.h);
			double aspect = vp1.aspect() * pixelaspect; 
			double fovy = Lens::getFovyForFovX(fovx * (vp1.w)/(double)vpright.w, aspect); 
			
			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mEye = pos + (ux * -iod);	// right
			mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
			mModelView = Matrix4d::lookAt(ux, uy, uz, mEye);
			
			sendViewport(gl, vp1);
			if(clear) sendClear(gl);
			
			pushDrawPop(gl,draw);
			
			wx = wx1;
		}
	} else {
		sendViewport(gl, vpleft);
		
		// apply camera transform:
		mEye = pos + (ux * iod);	// left
		mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
		mModelView = Matrix4d::lookAtLeft(ux, uy, uz, mEye, iod);
		
		pushDrawPop(gl,draw);
		
		sendViewport(gl, vpright);
		
		// apply camera transform:
		mEye = pos + (ux * -iod);	// right
		mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
		mModelView = Matrix4d::lookAtRight(ux, uy, uz, mEye, iod);
		
		pushDrawPop(gl,draw);
	}	

	glDisable(GL_SCISSOR_TEST);
	//glPopAttrib();
}



void Stereographic :: drawLeft(Graphics& gl, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear, double pixelaspect) 
{
	double near = lens.near();
	double far = lens.far();
	double focal = lens.focalLength();
	double iod = lens.eyeSep();
	const Vec3d& pos = pose.pos();

	//glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);

	glEnable(GL_SCISSOR_TEST);
	sendViewport(gl, vp);
	
	// glDrawBuffer(GL_BACK); //<< this breaks usage within FBO.
	if(clear) sendClear(gl);
	
	if (omni()) {
		int wx = vp.l;
		double fovx = mOmniFov;
		for (unsigned i=0; i<mSlices; i++) {
			// angle at center of slice:
			double angle = fovx * (0.5-((i+0.5)/(double)(mSlices)));
			
			int wx1 = vp.l + vp.w * (i+1)/(double)mSlices;
			Viewport vp1(wx, vp.b, wx1-wx, vp.h);
			double aspect = vp1.aspect() * pixelaspect; 
			double fovy = Lens::getFovyForFovX(fovx * (vp1.w)/(double)vp.w, aspect); 
			
			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mEye = pos + (ux * iod);	// left
			mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
			mModelView = Matrix4d::lookAt(ux, uy, uz, mEye);
	
			sendViewport(gl, vp1);
			if(clear) sendClear(gl);
			
			pushDrawPop(gl,draw);
			
			wx = wx1;
		}
		
	} else {
		
		double fovy = lens.fovy();
		double aspect = vp.aspect() * pixelaspect;
		Vec3d ux, uy, uz; pose.unitVectors(ux, uy, uz);
		mEye = pos + (ux * iod);	// left
		mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
		mModelView = Matrix4d::lookAtLeft(ux, uy, uz, mEye, iod);
	
		pushDrawPop(gl,draw);
	}

	glDisable(GL_SCISSOR_TEST);
	//glPopAttrib();
}

void Stereographic :: drawRight(Graphics& gl, const Lens& lens, const Pose& pose, const Viewport& vp, Drawable& draw, bool clear, double pixelaspect) 
{
	double near = lens.near();
	double far = lens.far();
	double focal = lens.focalLength();
	double iod = lens.eyeSep();
	const Vec3d& pos = pose.pos();
	
	//glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);

	glEnable(GL_SCISSOR_TEST);
	sendViewport(gl, vp);

	// glDrawBuffer(GL_BACK); //<< this breaks usage within FBO.
	if(clear) sendClear(gl);
	
	if (omni()) {
		int wx = vp.l;
		double fovx = mOmniFov;
		for (unsigned i=0; i<mSlices; i++) {
			// angle at center of slice:
			double angle = fovx * (0.5-((i+0.5)/(double)(mSlices)));
			
			int wx1 = vp.l + vp.w * (i+1)/(double)mSlices;
			Viewport vp1(wx, vp.b, wx1-wx, vp.h);
			double aspect = vp1.aspect() * pixelaspect; 
			double fovy = Lens::getFovyForFovX(fovx * (vp1.w)/(double)vp.w, aspect); 
			
			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mEye = pos + (ux * -iod);	// right
			mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
			mModelView = Matrix4d::lookAt(ux, uy, uz, mEye);
		
			sendViewport(gl, vp1);
			if(clear) sendClear(gl);
			
			pushDrawPop(gl,draw);
			
			wx = wx1;
		}
	} else {
		double fovy = lens.fovy();
		double aspect = vp.aspect() * pixelaspect;		
		Vec3d ux, uy, uz; 
		pose.unitVectors(ux, uy, uz);
		
		// apply camera transform:
		mEye = pos + (ux * -iod);	// right
		mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
		mModelView = Matrix4d::lookAtRight(ux, uy, uz, mEye, iod);
		
		pushDrawPop(gl,draw);		
	}

	glDisable(GL_SCISSOR_TEST);
	//glPopAttrib();
}

/// blue line sync for active stereo
/// @see http://local.wasp.uwa.edu.au/~pbourke/miscellaneous/stereographics/stereorender/GLUTStereo/glutStereo.cpp
void Stereographic :: drawBlueLine(double window_width, double window_height) 
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
	
	glDrawBuffer(GL_BACK);
}

} // al::
