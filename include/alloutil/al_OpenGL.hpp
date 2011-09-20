#ifndef INC_AL_UTIL_OPENGL_HPP
#define INC_AL_UTIL_OPENGL_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


	File description:
	A collection of utilities that are specific to OpenGL

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include "allocore/graphics/al_Graphics.hpp"
#include "alloutil/al_TextureGL.hpp"

#include <stdio.h>

/*!
	
*/

namespace al {

class DisplayList {
public:
	DisplayList() : mID(0) {}
	~DisplayList() { clear(); }

	void begin() {
		if (mID)
			glDeleteLists(mID, 1);
		mID = glGenLists(1);
		glNewList(mID, GL_COMPILE);
	}
	void end() { glEndList(); }
	void draw() { glCallList(mID); }
	void clear() { glDeleteLists(mID, 1);}

	unsigned long mID;
};


class CubeMapFBO : public CubeMapTexture {
public:
	CubeMapFBO(int resolution=1024) 
	:	CubeMapTexture(resolution),
		mFboId(0),
		mRboId(0),
		mClearColor(0)
	{}
	
	virtual ~CubeMapFBO() {}
	
	virtual void onCreate() {
		CubeMapTexture::onCreate();
		
		// one FBO to rule them all...
		glGenFramebuffersEXT(1, &mFboId);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFboId);
		//Attach one of the faces of the Cubemap texture to this FBO
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, id(), 0);

		
		glGenRenderbuffersEXT(1, &mRboId);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mRboId);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, mResolution, mResolution);
		// Attach depth buffer to FBO
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, mRboId);
		
		// ...and in the darkness bind them:
		for (unsigned face=0; face<6; face++) {
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT+face, GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, id(), 0);
		}
		
		//Does the GPU support current FBO configuration?
		GLenum status;
		status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			printf("GPU does not support required FBO configuration\n");
			exit(0);
		}
		
		// cleanup:
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
	
	virtual void onDestroy() {
		glDeleteRenderbuffersEXT(1, &mRboId);
		glDeleteFramebuffersEXT(1, &mFboId);
		mRboId = mFboId = 0;
		
		CubeMapTexture::onDestroy();
	}
	
	void capture(Graphics& gl, const Camera& cam, const Pose& pose, Drawable& draw) {
		validate();
		
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		
		Vec3d pos = pose.pos();
		Vec3d ux, uy, uz; 
		pose.unitVectors(ux, uy, uz);
		mProjection = Matrix4d::perspective(90, 1, cam.near(), cam.far());
		
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFboId);
		for (int face=0; face<6; face++) {
			glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT+face);
			
			gl.viewport(0, 0, resolution(), resolution());
			gl.clearColor(clearColor());
			gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
			
			switch (face) {
				case 0:
					// GL_TEXTURE_CUBE_MAP_POSITIVE_X   
					mModelView = Matrix4d::lookAt(uz, uy, -ux, pos);
					break;
				case 1:
					// GL_TEXTURE_CUBE_MAP_NEGATIVE_X   
					mModelView = Matrix4d::lookAt(-uz, uy, ux, pos);
					break;
				case 2:
					// GL_TEXTURE_CUBE_MAP_POSITIVE_Y   
					mModelView = Matrix4d::lookAt(ux, -uz, uy, pos);
					break;
				case 3:
					// GL_TEXTURE_CUBE_MAP_NEGATIVE_Y   
					mModelView = Matrix4d::lookAt(ux, uz, -uy, pos);
					break;
				case 4:
					// GL_TEXTURE_CUBE_MAP_POSITIVE_Z   
					mModelView = Matrix4d::lookAt(ux, uy, uz, pos);
					break;
				default:
					// GL_TEXTURE_CUBE_MAP_NEGATIVE_Z   
					mModelView = Matrix4d::lookAt(-ux, uy, -uz, pos);
					break;
			}
			
			// apply camera transform:
			gl.pushMatrix(gl.PROJECTION);
			gl.loadMatrix(mProjection);
			gl.pushMatrix(gl.MODELVIEW);
			gl.loadMatrix(mModelView);
			
			draw.onDraw(gl);
			
			gl.popMatrix(gl.PROJECTION);
			gl.popMatrix(gl.MODELVIEW);
		}
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		
		glPopAttrib();
	}
	
	Color clearColor() const { return mClearColor; }
	Matrix4d projection() const { return mProjection; }
	Matrix4d modelView() const { return mModelView; }
	GLuint fbo() { return mFboId; }
	GLuint rbo() { return mRboId; }

	CubeMapFBO& clearColor(const Color& c) { mClearColor = c; return *this; }
	
protected:
	GLuint mFboId, mRboId;
	Color mClearColor;
	Matrix4d mProjection;
	Matrix4d mModelView;
};


class Lighting {
public:
	
	static void globalAmbient(Color c) {
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, c.components);
	}
	
	static void shadeModel(bool b) {
		glShadeModel(b ? GL_SMOOTH : GL_FLAT);
	}
	
	static void wireFrame(bool b=true) {
		glPolygonMode(GL_FRONT_AND_BACK, b ? GL_LINE : GL_FILL);
	}
	
	static void cullFace(bool b=true) {
		if (b) {
			glEnable(GL_CULL_FACE);
		} else {
			glDisable(GL_CULL_FACE);
		}
	}
	
	class Material {
	public:
		
		Material()
		:	ccw(true),
		useGLColor(true),
		twoSided(true),
		wireFrame(false),
		shininess(0.5),
		ambient(0.5),
		diffuse(ambient),
		specular(1),
		emission(0)
		{}
		
		void begin() {
			glFrontFace(ccw ? GL_CCW : GL_CW);
			GLenum face = twoSided ? GL_FRONT_AND_BACK : GL_FRONT;
			
			glPolygonMode(face, wireFrame ? GL_LINE : GL_FILL);
			
			glMaterialfv(face, GL_SPECULAR, specular.components);
			glMaterialfv(face, GL_EMISSION, emission.components);
			glMateriali(face, GL_SHININESS, 128.f*shininess);
			
			if (useGLColor) {
				glEnable(GL_COLOR_MATERIAL);
				glColorMaterial(face, GL_AMBIENT_AND_DIFFUSE);
			} else {
				glDisable(GL_COLOR_MATERIAL);
				glMaterialfv(face, GL_AMBIENT, ambient.components);
				glMaterialfv(face, GL_DIFFUSE, diffuse.components);
			}
		}
		
		void end() {
			
		}
		
		void ambientAndDiffuse(Color c) {
			ambient = c;
			diffuse = c;
		}
		
		// set winding (which way is front)
		void ccwWinding(bool b=true) {
			ccw = b;
		}
		
		bool ccw;
		bool useGLColor;
		bool twoSided;
		bool wireFrame;
		float shininess;
		Color ambient, diffuse, specular, emission;
		
	};
	
	class Light {
	public:
		
		Light()
		:	id(0),
		diffuse(1),
		specular(1),
		ambient(0),
		position(Vec3f(0, 0, 1)),
		positionMode(1.f),
		spotDirection(Vec3f(0, 0, -1)),
		spotExponent(0),
		spotCutoff(180),
		attenuationConstant(1.f),
		attenuationLinear(0.f),
		attenuationQuadratic(0.f)
		{}
		
		void color(const Color& c) {
			diffuse = c;
			specular = c;
		}
		
		void direction(Vec3f d) {
			directional();
			position.set(d);
		}
		
		void directional() { positionMode=0.f; }
		void positional() { positionMode=1.f; }
		
		void begin(int i=0) {
			id = GL_LIGHT0 + (i&7);
			glEnable(id);
			glLightfv(id, GL_AMBIENT, ambient.components);
			glLightfv(id, GL_DIFFUSE, diffuse.components);
			glLightfv(id, GL_SPECULAR, specular.components);
			float p[] = {position[0], position[1], position[2], positionMode};
			glLightfv(id, GL_POSITION, p);
			
			glLightf (id, GL_SPOT_CUTOFF, spotCutoff);
			glLightf (id, GL_SPOT_EXPONENT, spotExponent);
			if (spotCutoff != 180.f) {
				glLightfv(id, GL_SPOT_DIRECTION, spotDirection.elems());
			}
			
			if (positionMode != 0.f) {
				glLightf(id, GL_CONSTANT_ATTENUATION, attenuationConstant);
				glLightf(id, GL_LINEAR_ATTENUATION, attenuationLinear);
				glLightf(id, GL_QUADRATIC_ATTENUATION, attenuationQuadratic);
			}
		}
		
		void end() {
			glDisable(id);
		}
		
		int id;
		Color diffuse, specular, ambient;
		Vec3f position;
		float positionMode;	// 0==directional, 1==positional
		Vec3f spotDirection;// valid only when spotCutoff != 180
		int spotExponent;	// 0..128, increases focussing of light in spotDirection
		float spotCutoff;	// 0..90 (cone) or 180 (omni)
		float attenuationConstant, attenuationLinear, attenuationQuadratic;
		
	};
};

} // al::

#endif