#include "graphics/al_Stereographic.hpp"
#include "graphics/al_Common.hpp"	/* << need the OpenGL headers */

namespace al {
namespace gfx {

void Stereographic :: draw(Graphics& gl, Camera& cam, void (*draw)(void *), double width, double height, void * userdata) 
{
	if (mStereo) {
		switch (mMode) {
			case Anaglyph:
				drawAnaglyph(gl, cam, draw, width, height, userdata);
				break;
			case Active:
				drawActive(gl, cam, draw, width, height, userdata);
				break;
			case Dual:
				drawDual(gl, cam, draw, width, height, userdata);
				break;
			case LeftEye:
				drawLeft(gl, cam, draw, width, height, userdata);
				break;
			case RightEye:
				drawRight(gl, cam, draw, width, height, userdata);
				break;
			default:
				printf("unknown stereo mode\n");
				break;
		}
	} else {
		drawMono(gl, cam, draw, width, height, userdata);
	}
}

void Stereographic :: drawMono(Graphics& gl, Camera& cam, void (*draw)(void *), double width, double height, void * userdata) 
{
	double aspect = width/height;
	const Vec3d& pos = cam.pos();
	const Vec3d& ux  = cam.ux();
	const Vec3d& uy  = cam.uy();
	const Vec3d& uz  = cam.uz();
	double fovy = cam.fovy();
	double near = cam.near();
	double far = cam.far();
		
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	//glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	
	//gl.drawBuffer(Back);
	glDrawBuffer(GL_BACK);
	
	// GraphicsBackedOpenGL should also apply matching glScissor() 
//	//enable(ScissorTest);
//	glEnable(GL_SCISSOR_TEST);
//	//scissor(vp.left(),vp.bottom(), width * vp.width(), height * vp.height());
//	glScissor(0, 0, width, height);
	gl.viewport(0,0,width,height);
	
	gl.clear(gfx::COLOR_BUFFER_BIT | gfx::DEPTH_BUFFER_BIT);
	
	// apply camera transform:
	gl.matrixMode(gfx::PROJECTION);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::perspective(fovy, aspect, near, far));

	gl.matrixMode(gfx::MODELVIEW);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::lookAt(ux, uy, uz, pos));
	
	draw(userdata);
	
	gl.matrixMode(gfx::PROJECTION);
	gl.popMatrix();
	gl.matrixMode(gfx::MODELVIEW);
	gl.popMatrix();
	
	//disable(ScissorTest);
	//glDisable(GL_SCISSOR_TEST);
	//popAttrib()
	//glPopAttrib();
}

void Stereographic :: drawAnaglyph(Graphics& gl, Camera& cam, void (*draw)(void *), double width, double height, void * userdata) 
{
	double aspect = width/height;
	const Vec3d& pos = cam.pos();
	const Vec3d& ux  = cam.ux();
	const Vec3d& uy  = cam.uy();
	const Vec3d& uz  = cam.uz();
	double fovy = cam.fovy();
	double near = cam.near();
	double far = cam.far();
	double focal = cam.focalLength();
	double iod = cam.IOD();
	
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	//glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);

	//drawBuffer(BackLeft);
	glDrawBuffer(GL_BACK);
	
	// GraphicsBackedOpenGL should also apply matching glScissor() 
//	//enable(ScissorTest);
//	glEnable(GL_SCISSOR_TEST);
//	//scissor(vp.left(),vp.bottom(), width * vp.width(), height * vp.height());
//	glScissor(0, 0, width, height);
	gl.viewport(0,0,width,height);
	
	gl.clear(gfx::COLOR_BUFFER_BIT);
	
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
	gl.clear(gfx::DEPTH_BUFFER_BIT);
	
	// apply camera transform:
	gl.matrixMode(gfx::PROJECTION);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal)); //T fovy, T aspect, T near, T far, T eyeSep, T focal

	gl.matrixMode(gfx::MODELVIEW);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::lookAtLeft(ux, uy, uz, pos, iod));
	
	draw(userdata);
	
	gl.matrixMode(gfx::PROJECTION);
	gl.popMatrix();
	gl.matrixMode(gfx::MODELVIEW);
	gl.popMatrix();
	
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
	gl.clear(gfx::DEPTH_BUFFER_BIT);
	
	// apply camera transform:
	gl.matrixMode(gfx::PROJECTION);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal)); //T fovy, T aspect, T near, T far, T eyeSep, T focal

	gl.matrixMode(gfx::MODELVIEW);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::lookAtRight(ux, uy, uz, pos, iod));
	
	draw(userdata);
	
	gl.matrixMode(gfx::PROJECTION);
	gl.popMatrix();
	gl.matrixMode(gfx::MODELVIEW);
	gl.popMatrix();
	
	//colorMask(1,1,1,1);
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	
//	//disable(ScissorTest);
//	glDisable(GL_SCISSOR_TEST);

//	//popAttrib()
//	glPopAttrib();
}

void Stereographic :: drawActive(Graphics& gl, Camera& cam, void (*draw)(void *), double width, double height, void * userdata) 
{
	double aspect = width/height;
	const Vec3d& pos = cam.pos();
	const Vec3d& ux  = cam.ux();
	const Vec3d& uy  = cam.uy();
	const Vec3d& uz  = cam.uz();
	double fovy = cam.fovy();
	double near = cam.near();
	double far = cam.far();
	double focal = cam.focalLength();
	double iod = cam.IOD();
	
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	//glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	
	//drawBuffer(BackLeft);
	glDrawBuffer(GL_BACK_LEFT);
	
	gl.clear(gfx::COLOR_BUFFER_BIT | gfx::DEPTH_BUFFER_BIT);
	
	// GraphicsBackedOpenGL should also apply matching glScissor() 
//	//enable(ScissorTest);
//	glEnable(GL_SCISSOR_TEST);
//	//scissor(vp.left(),vp.bottom(), width * vp.width(), height * vp.height());
//	glScissor(0, 0, width, height);
	gl.viewport(0,0,width,height);

	// apply camera transform:
	gl.matrixMode(gfx::PROJECTION);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal)); //T fovy, T aspect, T near, T far, T eyeSep, T focal

	gl.matrixMode(gfx::MODELVIEW);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::lookAtLeft(ux, uy, uz, pos, iod));
	
	draw(userdata);
	
	gl.matrixMode(gfx::PROJECTION);
	gl.popMatrix();
	gl.matrixMode(gfx::MODELVIEW);
	gl.popMatrix();
	
	//drawBuffer(BackRight);
	glDrawBuffer(GL_BACK_RIGHT);
	gl.clear(gfx::COLOR_BUFFER_BIT | gfx::DEPTH_BUFFER_BIT);
	
	// GraphicsBackedOpenGL should also apply matching glScissor() 
//	//enable(ScissorTest);
//	glEnable(GL_SCISSOR_TEST);
//	//scissor(vp.left(),vp.bottom(), width * vp.width(), height * vp.height());
//	glScissor(0, 0, width, height);
	gl.viewport(0,0,width,height);

	// apply camera transform:
	gl.matrixMode(gfx::PROJECTION);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal)); //T fovy, T aspect, T near, T far, T eyeSep, T focal

	gl.matrixMode(gfx::MODELVIEW);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::lookAtRight(ux, uy, uz, pos, iod));
	
	draw(userdata);
	
	gl.matrixMode(gfx::PROJECTION);
	gl.popMatrix();
	gl.matrixMode(gfx::MODELVIEW);
	gl.popMatrix();
	
	//disable(ScissorTest);
	//glDisable(GL_SCISSOR_TEST);
	//popAttrib()
	//glPopAttrib();
}

void Stereographic :: drawDual(Graphics& gl, Camera& cam, void (*draw)(void *), double width, double height, void * userdata) 
{
	double aspect = width/height;
	const Vec3d& pos = cam.pos();
	const Vec3d& ux  = cam.ux();
	const Vec3d& uy  = cam.uy();
	const Vec3d& uz  = cam.uz();
	double fovy = cam.fovy();
	double near = cam.near();
	double far = cam.far();
	double focal = cam.focalLength();
	double iod = cam.IOD();
	
	aspect *= 0.5;	// for split view
	
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	//glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	
	//drawBuffer(BackLeft);
	glDrawBuffer(GL_BACK);
	
	// GraphicsBackedOpenGL should also apply matching glScissor() 
//	glEnable(GL_SCISSOR_TEST);
//	glScissor(0, 0, width*0.5, height);
	gl.viewport(0,0,width*0.5,height);
	
	gl.clear(gfx::COLOR_BUFFER_BIT | gfx::DEPTH_BUFFER_BIT);
	
	// apply camera transform:
	gl.matrixMode(gfx::PROJECTION);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal)); //T fovy, T aspect, T near, T far, T eyeSep, T focal

	gl.matrixMode(gfx::MODELVIEW);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::lookAtLeft(ux, uy, uz, pos, iod));
	
	draw(userdata);
	
	gl.matrixMode(gfx::PROJECTION);
	gl.popMatrix();
	gl.matrixMode(gfx::MODELVIEW);
	gl.popMatrix();
	
	// GraphicsBackedOpenGL should also apply matching glScissor() 
//	glScissor(width*0.5, 0, width*0.5, height);
	gl.viewport(width*0.5,0,width*0.5,height);
	
	gl.clear(gfx::COLOR_BUFFER_BIT | gfx::DEPTH_BUFFER_BIT);
	
	// apply camera transform:
	gl.matrixMode(gfx::PROJECTION);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal)); //T fovy, T aspect, T near, T far, T eyeSep, T focal

	gl.matrixMode(gfx::MODELVIEW);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::lookAtRight(ux, uy, uz, pos, iod));
	
	draw(userdata);
	
	gl.matrixMode(gfx::PROJECTION);
	gl.popMatrix();
	gl.matrixMode(gfx::MODELVIEW);
	gl.popMatrix();
	
	//disable(ScissorTest);
	//glDisable(GL_SCISSOR_TEST);
	//popAttrib()
	//glPopAttrib();
}



void Stereographic :: drawLeft(Graphics& gl, Camera& cam, void (*draw)(void *), double width, double height, void * userdata) 
{
	double aspect = width/height;
	const Vec3d& pos = cam.pos();
	const Vec3d& ux  = cam.ux();
	const Vec3d& uy  = cam.uy();
	const Vec3d& uz  = cam.uz();
	double fovy = cam.fovy();
	double near = cam.near();
	double far = cam.far();
	double focal = cam.focalLength();
	double iod = cam.IOD();
	
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	//glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	
	glDrawBuffer(GL_BACK);
	
	// GraphicsBackedOpenGL should also apply matching glScissor() 
//	glEnable(GL_SCISSOR_TEST);
//	glScissor(0, 0, width, height);
	gl.viewport(0,0,width,height);
	
	gl.clear(gfx::COLOR_BUFFER_BIT | gfx::DEPTH_BUFFER_BIT);
	
	// apply camera transform:
	gl.matrixMode(gfx::PROJECTION);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::perspectiveLeft(fovy, aspect, near, far, iod, focal)); //T fovy, T aspect, T near, T far, T eyeSep, T focal

	gl.matrixMode(gfx::MODELVIEW);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::lookAtLeft(ux, uy, uz, pos, iod));

	draw(userdata);
	
	gl.matrixMode(gfx::PROJECTION);
	gl.popMatrix();
	gl.matrixMode(gfx::MODELVIEW);
	gl.popMatrix();
	
	//disable(ScissorTest);
	//glDisable(GL_SCISSOR_TEST);
	//popAttrib()
	//glPopAttrib();
}

void Stereographic :: drawRight(Graphics& gl, Camera& cam, void (*draw)(void *), double width, double height, void * userdata) 
{
	double aspect = width/height;
	const Vec3d& pos = cam.pos();
	const Vec3d& ux  = cam.ux();
	const Vec3d& uy  = cam.uy();
	const Vec3d& uz  = cam.uz();
	double fovy = cam.fovy();
	double near = cam.near();
	double far = cam.far();
	double focal = cam.focalLength();
	double iod = cam.IOD();
	
	//pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	//glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	
	glDrawBuffer(GL_BACK);
	
	// GraphicsBackedOpenGL should also apply matching glScissor() 
//	glEnable(GL_SCISSOR_TEST);
//	glScissor(0, 0, width, height);
	gl.viewport(0,0,width,height);
	
	gl.clear(gfx::COLOR_BUFFER_BIT | gfx::DEPTH_BUFFER_BIT);
	
	// apply camera transform:
	gl.matrixMode(gfx::PROJECTION);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::perspectiveRight(fovy, aspect, near, far, iod, focal)); //T fovy, T aspect, T near, T far, T eyeSep, T focal

	gl.matrixMode(gfx::MODELVIEW);
	gl.pushMatrix();
	gl.loadMatrix(Matrix4d::lookAtRight(ux, uy, uz, pos, iod));

	draw(userdata);
	
	gl.matrixMode(gfx::PROJECTION);
	gl.popMatrix();
	gl.matrixMode(gfx::MODELVIEW);
	gl.popMatrix();
	
	//disable(ScissorTest);
	//glDisable(GL_SCISSOR_TEST);
	//popAttrib()
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

}

} // gfx::
} // al::
