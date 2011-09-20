#include "allocore/graphics/al_Stereographic.hpp"
#include "allocore/graphics/al_Graphics.hpp"	/* << need the OpenGL headers */

namespace al{

void Stereographic :: draw(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw) {
	if(mStereo){
		switch(mMode){
			case Anaglyph:	drawAnaglyph(gl, cam, pose, vp, draw); return;
			case Active:	drawActive	(gl, cam, pose, vp, draw); return;
			case Dual:		drawDual	(gl, cam, pose, vp, draw); return;
			case LeftEye:	drawLeft	(gl, cam, pose, vp, draw); return;
			case RightEye:	drawRight	(gl, cam, pose, vp, draw); return;
			default:;
		}
	} else {
		drawMono(gl, cam, pose, vp, draw);
	}
}

void Stereographic :: drawMono(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw) 
{
	double near = cam.near();
	double far = cam.far();
	const Vec3d& pos = pose.pos();
	
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	gl.clearColor(mClearColor);
	
	glDrawBuffer(GL_BACK);
	gl.viewport(vp.l, vp.b, vp.w, vp.h);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	
	if (omni()) {
		int wx = vp.l;
		double fovx = mOmniFov;
		for (unsigned i=0; i<mSlices; i++) {
			// angle at center of slice:
			double angle = fovx * (0.5-((i+0.5)/(double)(mSlices)));
			
			int wx1 = vp.l + vp.w * (i+1)/(double)mSlices;
			Viewport vp1(wx, vp.b, wx1-wx, vp.h);
			double aspect = vp1.aspect(); 
			double fovy = Camera::getFovyForFovX(fovx * (vp1.w)/(double)vp.w, aspect); 
			
			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mProjection = Matrix4d::perspective(fovy, aspect, near, far);
			mModelView = Matrix4d::lookAt(ux, uy, uz, pos);
			
			gl.viewport(vp1.l, vp1.b, vp1.w, vp1.h);
			gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
			
			// apply camera transform:
			gl.pushMatrix(gl.PROJECTION);
			gl.loadMatrix(mProjection);
			gl.pushMatrix(gl.MODELVIEW);
			gl.loadMatrix(mModelView);
			
			draw.onDraw(gl);

			gl.popMatrix(gl.PROJECTION);
			gl.popMatrix(gl.MODELVIEW);
			
			wx = wx1;
		}
		
	} else {
		
		double fovy = cam.fovy();
		double aspect = vp.aspect();
		Vec3d ux, uy, uz; pose.unitVectors(ux, uy, uz);
		mProjection = Matrix4d::perspective(fovy, aspect, near, far);
		mModelView = Matrix4d::lookAt(ux, uy, uz, pos);

		// apply camera transform:
		gl.pushMatrix(gl.PROJECTION);
		gl.loadMatrix(mProjection);
		gl.pushMatrix(gl.MODELVIEW);
		gl.loadMatrix(mModelView);
		
		draw.onDraw(gl);

		gl.popMatrix(gl.PROJECTION);
		gl.popMatrix(gl.MODELVIEW);

	}
	
	gl.scissor(false);
	glPopAttrib();
}

void Stereographic :: drawAnaglyph(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw) 
{	
	double near = cam.near();
	double far = cam.far();
	double focal = cam.focalLength();
	double iod = cam.eyeSep();
	const Vec3d& pos = pose.pos();

	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	gl.clearColor(mClearColor);
	
	glDrawBuffer(GL_BACK);
	gl.viewport(vp.l, vp.b, vp.w, vp.h);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	
	switch(mAnaglyphMode){
		case RedBlue:
		case RedGreen:
		case RedCyan:	glColorMask(GL_TRUE, GL_FALSE,GL_FALSE,GL_TRUE); break;
		case BlueRed:	glColorMask(GL_FALSE,GL_FALSE,GL_TRUE, GL_TRUE); break;
		case GreenRed:	glColorMask(GL_FALSE,GL_TRUE, GL_FALSE,GL_TRUE); break;
		case CyanRed:	glColorMask(GL_FALSE,GL_TRUE, GL_TRUE, GL_TRUE); break;
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
			double aspect = vp1.aspect(); 
			double fovy = Camera::getFovyForFovX(fovx * (vp1.w)/(double)vp.w, aspect); 
			
			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
			mModelView = Matrix4d::lookAtRight(ux, uy, uz, pos, iod);
		
			gl.viewport(vp1.l, vp1.b, vp1.w, vp1.h);
			gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
			
			// apply camera transform:
			gl.pushMatrix(gl.PROJECTION);
			gl.loadMatrix(mProjection);
			gl.pushMatrix(gl.MODELVIEW);
			gl.loadMatrix(mModelView);
			
			draw.onDraw(gl);

			gl.popMatrix(gl.PROJECTION);
			gl.popMatrix(gl.MODELVIEW);
			
			wx = wx1;
		}
	} else {
		double fovy = cam.fovy();
		double aspect = vp.aspect();		
		Vec3d ux, uy, uz; 
		pose.unitVectors(ux, uy, uz);

		gl.clear(gl.DEPTH_BUFFER_BIT);
		
		// apply camera transform:
		mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
		mModelView = Matrix4d::lookAtRight(ux, uy, uz, pos, iod);
		gl.pushMatrix(gl.PROJECTION);
		gl.loadMatrix(mProjection);
		gl.pushMatrix(gl.MODELVIEW);
		gl.loadMatrix(mModelView);
		
		draw.onDraw(gl);

		gl.popMatrix(gl.PROJECTION);
		gl.popMatrix(gl.MODELVIEW);
		
	}
	
	switch(mAnaglyphMode){
		case RedBlue:	glColorMask(GL_FALSE,GL_FALSE,GL_TRUE, GL_TRUE); break;
		case RedGreen:	glColorMask(GL_FALSE,GL_TRUE, GL_FALSE,GL_TRUE); break;
		case RedCyan:	glColorMask(GL_FALSE,GL_TRUE, GL_TRUE, GL_TRUE); break;
		case BlueRed:
		case GreenRed:
		case CyanRed:	glColorMask(GL_TRUE, GL_FALSE,GL_FALSE,GL_TRUE); break;
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
			double aspect = vp1.aspect(); 
			double fovy = Camera::getFovyForFovX(fovx * (vp1.w)/(double)vp.w, aspect); 
			
			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
			mModelView = Matrix4d::lookAtLeft(ux, uy, uz, pos, iod);
		
			gl.viewport(vp1.l, vp1.b, vp1.w, vp1.h);
			gl.clear(gl.DEPTH_BUFFER_BIT);				// Note: depth only this pass
			
			// apply camera transform:
			gl.pushMatrix(gl.PROJECTION);
			gl.loadMatrix(mProjection);
			gl.pushMatrix(gl.MODELVIEW);
			gl.loadMatrix(mModelView);
			
			draw.onDraw(gl);

			gl.popMatrix(gl.PROJECTION);
			gl.popMatrix(gl.MODELVIEW);
			
			wx = wx1;
		}
	} else {
		double fovy = cam.fovy();
		double aspect = vp.aspect();		
		Vec3d ux, uy, uz; 
		pose.unitVectors(ux, uy, uz);
		
		gl.clear(gl.DEPTH_BUFFER_BIT);
		
		// apply camera transform:
		mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
		mModelView = Matrix4d::lookAtLeft(ux, uy, uz, pos, iod);
		gl.pushMatrix(gl.PROJECTION);
		gl.loadMatrix(mProjection);
		gl.pushMatrix(gl.MODELVIEW);
		gl.loadMatrix(mModelView);
		
		draw.onDraw(gl);
		
		gl.popMatrix(gl.PROJECTION);
		gl.popMatrix(gl.MODELVIEW);
		
	}
	
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	gl.scissor(false);
	glPopAttrib();
}

void Stereographic :: drawActive(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw) 
{
	double near = cam.near();
	double far = cam.far();
	double focal = cam.focalLength();
	double iod = cam.eyeSep();
	const Vec3d& pos = pose.pos();

	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	gl.clearColor(mClearColor);
	gl.viewport(vp.l, vp.b, vp.w, vp.h);
	
	glDrawBuffer(GL_BACK_RIGHT);
	gl.viewport(vp.l, vp.b, vp.w, vp.h);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	
	if (omni()) {
	
		int wx = vp.l;
		double fovx = mOmniFov;
		for (unsigned i=0; i<mSlices; i++) {
			// angle at center of slice:
			double angle = fovx * (0.5-((i+0.5)/(double)(mSlices)));
			
			int wx1 = vp.l + vp.w * (i+1)/(double)mSlices;
			Viewport vp1(wx, vp.b, wx1-wx, vp.h);
			double aspect = vp1.aspect(); 
			double fovy = Camera::getFovyForFovX(fovx * (vp1.w)/(double)vp.w, aspect); 
			
			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
			mModelView = Matrix4d::lookAtRight(ux, uy, uz, pos, iod);
			
			gl.viewport(vp1.l, vp1.b, vp1.w, vp1.h);
			gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
			
			// apply camera transform:
			gl.pushMatrix(gl.PROJECTION);
			gl.loadMatrix(mProjection);
			gl.pushMatrix(gl.MODELVIEW);
			gl.loadMatrix(mModelView);
			
			draw.onDraw(gl);

			gl.popMatrix(gl.PROJECTION);
			gl.popMatrix(gl.MODELVIEW);
			
			wx = wx1;
		}
		
	} else {
		
		double fovy = cam.fovy();
		double aspect = vp.aspect();
		Vec3d ux, uy, uz; pose.unitVectors(ux, uy, uz);
		mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
		mModelView = Matrix4d::lookAtRight(ux, uy, uz, pos, iod);

		// apply camera transform:
		gl.pushMatrix(gl.PROJECTION);
		gl.loadMatrix(mProjection);
		gl.pushMatrix(gl.MODELVIEW);
		gl.loadMatrix(mModelView);
		
		draw.onDraw(gl);

		gl.popMatrix(gl.PROJECTION);
		gl.popMatrix(gl.MODELVIEW);

	}
	
//	// apply camera transform:
//	mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
//	mModelView = Matrix4d::lookAtRight(ux, uy, uz, pos, iod);
//	gl.pushMatrix(gl.PROJECTION);
//	gl.loadMatrix(mProjection);
//	gl.pushMatrix(gl.MODELVIEW);
//	gl.loadMatrix(mModelView);
//	
//	draw.onDraw(gl);
//	
//	gl.popMatrix(gl.PROJECTION);
//	gl.popMatrix(gl.MODELVIEW);

	
	glDrawBuffer(GL_BACK_LEFT);
	gl.viewport(vp.l, vp.b, vp.w, vp.h);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	
	if (omni()) {
	
		int wx = vp.l;
		double fovx = mOmniFov;
		for (unsigned i=0; i<mSlices; i++) {
			// angle at center of slice:
			double angle = fovx * (0.5-((i+0.5)/(double)(mSlices)));
			
			int wx1 = vp.l + vp.w * (i+1)/(double)mSlices;
			Viewport vp1(wx, vp.b, wx1-wx, vp.h);
			double aspect = vp1.aspect(); 
			double fovy = Camera::getFovyForFovX(fovx * (vp1.w)/(double)vp.w, aspect); 
			
			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
			mModelView = Matrix4d::lookAtLeft(ux, uy, uz, pos, iod);
	
			gl.viewport(vp1.l, vp1.b, vp1.w, vp1.h);
			gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
			
			// apply camera transform:
			gl.pushMatrix(gl.PROJECTION);
			gl.loadMatrix(mProjection);
			gl.pushMatrix(gl.MODELVIEW);
			gl.loadMatrix(mModelView);
			
			draw.onDraw(gl);

			gl.popMatrix(gl.PROJECTION);
			gl.popMatrix(gl.MODELVIEW);
			
			wx = wx1;
		}
		
	} else {
		
		double fovy = cam.fovy();
		double aspect = vp.aspect();
		Vec3d ux, uy, uz; pose.unitVectors(ux, uy, uz);
		mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
		mModelView = Matrix4d::lookAtLeft(ux, uy, uz, pos, iod);
	
		// apply camera transform:
		gl.pushMatrix(gl.PROJECTION);
		gl.loadMatrix(mProjection);
		gl.pushMatrix(gl.MODELVIEW);
		gl.loadMatrix(mModelView);
		
		draw.onDraw(gl);

		gl.popMatrix(gl.PROJECTION);
		gl.popMatrix(gl.MODELVIEW);

	}
	
//	// apply camera transform:
//	mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
//	mModelView = Matrix4d::lookAtLeft(ux, uy, uz, pos, iod);
//	gl.pushMatrix(gl.PROJECTION);
//	gl.loadMatrix(mProjection);
//	gl.pushMatrix(gl.MODELVIEW);
//	gl.loadMatrix(mModelView);
//	
//	draw.onDraw(gl);
//	
//	gl.popMatrix(gl.PROJECTION);
//	gl.popMatrix(gl.MODELVIEW);
	
	gl.scissor(false);
	glPopAttrib();
}

void Stereographic :: drawDual(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw) 
{
	double fovy = cam.fovy();
	double near = cam.near();
	double far = cam.far();
	double focal = cam.focalLength();
	double iod = cam.eyeSep();
	double aspect = vp.aspect();
	const Vec3d& pos = pose.pos();
	Vec3d ux, uy, uz; pose.unitVectors(ux, uy, uz);
	
	aspect *= 0.5;	// for split view
	
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	gl.clearColor(mClearColor);
	
	glDrawBuffer(GL_BACK);
	gl.viewport(vp.l, vp.b, vp.w, vp.h);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	
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
			double aspect = vp1.aspect(); 
			double fovy = Camera::getFovyForFovX(fovx * (vp1.w)/(double)vpleft.w, aspect); 
			
			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
			mModelView = Matrix4d::lookAtLeft(ux, uy, uz, pos, iod);
			
			gl.viewport(vp1.l, vp1.b, vp1.w, vp1.h);
			gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
			
			// apply camera transform:
			gl.pushMatrix(gl.PROJECTION);
			gl.loadMatrix(mProjection);
			gl.pushMatrix(gl.MODELVIEW);
			gl.loadMatrix(mModelView);
			
			draw.onDraw(gl);

			gl.popMatrix(gl.PROJECTION);
			gl.popMatrix(gl.MODELVIEW);
			
			wx = wx1;
		}
		
		wx = vpright.l;
		for (unsigned i=0; i<mSlices; i++) {
			// angle at center of slice:
			double angle = fovx * (0.5-((i+0.5)/(double)(mSlices)));
			
			int wx1 = vpright.l + vpright.w * (i+1)/(double)mSlices;
			Viewport vp1(wx, vpright.b, wx1-wx, vpright.h);
			double aspect = vp1.aspect(); 
			double fovy = Camera::getFovyForFovX(fovx * (vp1.w)/(double)vpright.w, aspect); 
			
			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
			mModelView = Matrix4d::lookAtRight(ux, uy, uz, pos, iod);
			
			gl.viewport(vp1.l, vp1.b, vp1.w, vp1.h);
			gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
			
			// apply camera transform:
			gl.pushMatrix(gl.PROJECTION);
			gl.loadMatrix(mProjection);
			gl.pushMatrix(gl.MODELVIEW);
			gl.loadMatrix(mModelView);
			
			draw.onDraw(gl);

			gl.popMatrix(gl.PROJECTION);
			gl.popMatrix(gl.MODELVIEW);
			
			wx = wx1;
		}
	} else {
		gl.viewport(vpleft.l, vpleft.b, vpleft.w, vpleft.h);
		
		// apply camera transform:
		mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
		mModelView = Matrix4d::lookAtLeft(ux, uy, uz, pos, iod);
		gl.pushMatrix(gl.PROJECTION);
		gl.loadMatrix(mProjection);
		gl.pushMatrix(gl.MODELVIEW);
		gl.loadMatrix(mModelView);
		
		draw.onDraw(gl);
		
		gl.popMatrix(gl.PROJECTION);
		gl.popMatrix(gl.MODELVIEW);
		
		gl.viewport(vpright.l, vpright.b, vpright.w, vpright.h);
		
		// apply camera transform:
		mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
		mModelView = Matrix4d::lookAtRight(ux, uy, uz, pos, iod);
		gl.pushMatrix(gl.PROJECTION);
		gl.loadMatrix(mProjection);
		gl.pushMatrix(gl.MODELVIEW);
		gl.loadMatrix(mModelView);
		
		draw.onDraw(gl);
		
		gl.popMatrix(gl.PROJECTION);
		gl.popMatrix(gl.MODELVIEW);
	}	
	
	gl.scissor(false);
	glPopAttrib();
}



void Stereographic :: drawLeft(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw) 
{
	double near = cam.near();
	double far = cam.far();
	double focal = cam.focalLength();
	double iod = cam.eyeSep();
	const Vec3d& pos = pose.pos();
	
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	gl.clearColor(mClearColor);
	
	glDrawBuffer(GL_BACK);
	gl.viewport(vp.l, vp.b, vp.w, vp.h);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	
	if (omni()) {
	
		int wx = vp.l;
		double fovx = mOmniFov;
		for (unsigned i=0; i<mSlices; i++) {
			// angle at center of slice:
			double angle = fovx * (0.5-((i+0.5)/(double)(mSlices)));
			
			int wx1 = vp.l + vp.w * (i+1)/(double)mSlices;
			Viewport vp1(wx, vp.b, wx1-wx, vp.h);
			double aspect = vp1.aspect(); 
			double fovy = Camera::getFovyForFovX(fovx * (vp1.w)/(double)vp.w, aspect); 
			
			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
			mModelView = Matrix4d::lookAtLeft(ux, uy, uz, pos, iod);
	
			gl.viewport(vp1.l, vp1.b, vp1.w, vp1.h);
			gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
			
			// apply camera transform:
			gl.pushMatrix(gl.PROJECTION);
			gl.loadMatrix(mProjection);
			gl.pushMatrix(gl.MODELVIEW);
			gl.loadMatrix(mModelView);
			
			draw.onDraw(gl);

			gl.popMatrix(gl.PROJECTION);
			gl.popMatrix(gl.MODELVIEW);
			
			wx = wx1;
		}
		
	} else {
		
		double fovy = cam.fovy();
		double aspect = vp.aspect();
		Vec3d ux, uy, uz; pose.unitVectors(ux, uy, uz);
		mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
		mModelView = Matrix4d::lookAtLeft(ux, uy, uz, pos, iod);
	
		// apply camera transform:
		gl.pushMatrix(gl.PROJECTION);
		gl.loadMatrix(mProjection);
		gl.pushMatrix(gl.MODELVIEW);
		gl.loadMatrix(mModelView);
		
		draw.onDraw(gl);

		gl.popMatrix(gl.PROJECTION);
		gl.popMatrix(gl.MODELVIEW);

	}
	
//	// apply camera transform:
//	mProjection = Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal);
//	mModelView = Matrix4d::lookAtLeft(ux, uy, uz, pos, iod);
//	gl.pushMatrix(gl.PROJECTION);
//	gl.loadMatrix(mProjection);
//	gl.pushMatrix(gl.MODELVIEW);
//	gl.loadMatrix(mModelView);
//
//	draw.onDraw(gl);
//	
//	gl.popMatrix(gl.PROJECTION);
//	gl.popMatrix(gl.MODELVIEW);
	
	gl.scissor(false);
	glPopAttrib();
}

void Stereographic :: drawRight(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw) 
{
	double near = cam.near();
	double far = cam.far();
	double focal = cam.focalLength();
	double iod = cam.eyeSep();
	const Vec3d& pos = pose.pos();
	
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	gl.clearColor(mClearColor);

	glDrawBuffer(GL_BACK);
	gl.viewport(vp.l, vp.b, vp.w, vp.h);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	
	if (omni()) {
		int wx = vp.l;
		double fovx = mOmniFov;
		for (unsigned i=0; i<mSlices; i++) {
			// angle at center of slice:
			double angle = fovx * (0.5-((i+0.5)/(double)(mSlices)));
			
			int wx1 = vp.l + vp.w * (i+1)/(double)mSlices;
			Viewport vp1(wx, vp.b, wx1-wx, vp.h);
			double aspect = vp1.aspect(); 
			double fovy = Camera::getFovyForFovX(fovx * (vp1.w)/(double)vp.w, aspect); 
			
			Quatd q = pose.quat() * Quatd().fromAxisAngle(M_DEG2RAD * angle, 0, 1, 0);
			Vec3d ux = q.toVectorX();
			Vec3d uy = q.toVectorY();
			Vec3d uz = q.toVectorZ();
			
			mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
			mModelView = Matrix4d::lookAtRight(ux, uy, uz, pos, iod);
		
			gl.viewport(vp1.l, vp1.b, vp1.w, vp1.h);
			gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
			
			// apply camera transform:
			gl.pushMatrix(gl.PROJECTION);
			gl.loadMatrix(mProjection);
			gl.pushMatrix(gl.MODELVIEW);
			gl.loadMatrix(mModelView);
			
			draw.onDraw(gl);

			gl.popMatrix(gl.PROJECTION);
			gl.popMatrix(gl.MODELVIEW);
			
			wx = wx1;
		}
	} else {
		double fovy = cam.fovy();
		double aspect = vp.aspect();		
		Vec3d ux, uy, uz; 
		pose.unitVectors(ux, uy, uz);
		
		// apply camera transform:
		mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
		mModelView = Matrix4d::lookAtRight(ux, uy, uz, pos, iod);
		gl.pushMatrix(gl.PROJECTION);
		gl.loadMatrix(mProjection);
		gl.pushMatrix(gl.MODELVIEW);
		gl.loadMatrix(mModelView);
		
		draw.onDraw(gl);

		gl.popMatrix(gl.PROJECTION);
		gl.popMatrix(gl.MODELVIEW);
		
	}

//	// apply camera transform:
//	mProjection = Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal);
//	mModelView = Matrix4d::lookAtRight(ux, uy, uz, pos, iod);
//	gl.pushMatrix(gl.PROJECTION);
//	gl.loadMatrix(mProjection);
//	gl.pushMatrix(gl.MODELVIEW);
//	gl.loadMatrix(mModelView);
//
//	draw.onDraw(gl);
//	
//	gl.popMatrix(gl.PROJECTION);
//	gl.popMatrix(gl.MODELVIEW);
	
	gl.scissor(false);
	glPopAttrib();
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
}

} // al::
