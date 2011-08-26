#ifndef INC_AL_UTIL_OPENGL_HPP
#define INC_AL_UTIL_OPENGL_HPP

#include "allocore/graphics/al_GraphicsOpenGL.hpp"

/*!
	A collection of utilities that are (currently) specific to OpenGL
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

/*!
	A simple wrapper around OpenGL Textures, 
	using al::Array as a CPU-side interface for configuring & submitting
	
	TODO: lift out common features of TextureGL and CubeMapTexture into a 
	generic superclass ?
*/
class TextureGL : public GPUObject {
public:
	TextureGL()
	:	GPUObject(),
		mTarget(GL_TEXTURE_2D),
		mFormat(GL_RGBA),
		mType(GL_UNSIGNED_BYTE),
		mLevel(0),
		mBorder(0),
		mAlignment(4),
		mRebuild(true),
		mSubmit(true)
	{}
	
	virtual ~TextureGL() {}

	virtual void onCreate() {
		if (mID == 0) {
			glGenTextures(1, (GLuint *)&mID);
			glBindTexture(mTarget, id());
			
			// TODO: which options?
			glTexParameterf(mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf(mTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(mTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(mTarget, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
			submit();
			glBindTexture(mTarget, 0);
			GraphicsGL::gl_error("creating texture");
		}
		mRebuild = false;
	}
	
	virtual void onDestroy() {
		if (mID) {
			glDeleteTextures(1, (GLuint *)&mID);
			mID = 0;
		}
	}

	void bind(int unit = 0) {
		if (mRebuild) {
			onDestroy();
		}
		
		// ensure it is created:
		validate();
		
		// multitexturing:
		glActiveTextureARB( GL_TEXTURE0_ARB+unit );
		
		// bind:
		glEnable(mTarget);
		glBindTexture(mTarget, id());
		
		GraphicsGL::gl_error("binding texture");
		
		// re-submit if necessary:
		if(mSubmit) submit();
		
		
	}
	
	void unbind(int unit = 0) {
		
		// multitexturing:
		glActiveTextureARB( GL_TEXTURE0_ARB+unit );
		
		glBindTexture(mTarget, 0);
		glDisable(mTarget);
	}
	
	GLuint id() const { return mID; }
	
	uint32_t width() const { return mArray.width(); }
	uint32_t height() const { return mArray.height(); }
	uint32_t depth() const { return mArray.depth(); }
	
	// retrieve internal array:
	al::Array& array() { return mArray; }
	// raw access:
	char * data() { return mArray.data.ptr; }
	
	// set texture data from array.
	void fromArray(const al::Array& src) {
		int sz = src.size();
		if(sz <= 0) {
			printf("ignoring attempt to store empty array to texture\n");
			return;	// ignore empty arrays
		}
		
		// format change?
		if (!mArray.isFormat(src.header)) {
			printf("texture format change\n");
			configureFromArray(src);
			// trigger re-create:
			mRebuild = true;
		}
		
		// copy data & change format:
		mArray = src;
		
		// trigger resubmit
		mSubmit = true;
	}
	
	// manually trigger format/size change:
	void rebuild() { 
		configureFromArray(mArray);
		mRebuild = true; 
	}
	// manually trigger data change:
	void resubmit() { mSubmit = true; }
	
protected:

	GLenum formatFromArray(const Array& src) {
		switch(src.header.components) {
			case 1:	return GL_LUMINANCE;
			case 2:	return GL_LUMINANCE_ALPHA;
			case 3:	return GL_RGB;
			case 4:	return GL_RGBA;
			default:
				printf("warning: unknown type\n");
				return GL_RGBA;
		}
	}
	
	GLenum typeFromArray(const Array& src) {
		switch(src.header.type) {
			case AlloSInt8Ty:		return GL_BYTE;
			case AlloUInt8Ty:		return GL_UNSIGNED_BYTE;
			case AlloSInt16Ty:		return GL_SHORT;
			case AlloUInt16Ty:		return GL_UNSIGNED_SHORT;
			case AlloSInt32Ty:		return GL_INT;
			case AlloUInt32Ty:		return GL_UNSIGNED_INT;
			case AlloFloat32Ty:		return GL_FLOAT;
			default:
				printf("warning: unknown type\n");
				return GL_UNSIGNED_BYTE;
		}
	}
	
	GLenum targetFromArray(const Array& src) {
		switch(src.header.dimcount) {
			case 1:		return GL_TEXTURE_1D;
			case 2:		return GL_TEXTURE_2D;
			case 3:		return GL_TEXTURE_3D;
			default:
				printf("warning: unknown dimcount\n");
				return GL_TEXTURE_2D;
		}
	}

	void configureFromArray(const Array& src) {
		// derive parameters from array type:
		mFormat = formatFromArray(src);
		mType = typeFromArray(src);
		mTarget = targetFromArray(src);
		mAlignment = getRowStride() % 4;
		if(mAlignment == 0) mAlignment = 4;
		printf("reconfigure from array %x %x %x %x \n", mFormat, mType, mTarget, mAlignment);
	}
	
	int getRowStride() const {
		return mArray.header.stride[1];
	}
	
	void submit() {
		GLvoid * ptr = (GLvoid *)data();
		glPixelStorei(GL_UNPACK_ALIGNMENT, mAlignment);
		
		printf("texture submit id %i target %X level %i components %i dim %ix%i format %X type %X ptr %p alignment %i\n", id(), mTarget, mLevel, mArray.header.components, width(), height(), mFormat, mType, ptr, mAlignment);
		printf("glTexImage2D target %X level %i components %i dim %ix%i format %X type %X ptr %p alignment %i\n", GL_TEXTURE_2D, 0, 3, 600, 600, GL_RGB, GL_UNSIGNED_BYTE, ptr, 4);

		switch(mTarget) {
			case GL_TEXTURE_1D:
				glTexImage1D(
					mTarget, mLevel, mArray.header.components, 
					width(), mBorder, 
					mFormat, mType, ptr
				);
				break;
			case GL_TEXTURE_2D:
			//case GL_TEXTURE_RECTANGLE_ARB:
//				glTexSubImage2D (
//					mTarget, mLevel, 
//					0, 0, width(), height(), 
//					mFormat, mType, ptr);
				glTexImage2D(
					mTarget, mLevel, mArray.header.components, 
					width(), height(), mBorder, 
					mFormat, mType, ptr
				);
				break;
			case GL_TEXTURE_3D:
				glTexImage3D(
					mTarget, mLevel, mArray.header.components, 
					width(), height(), depth(), mBorder, 
					mFormat, mType, ptr
				);
				break;

			default:
				printf("target not yet handled\n");
				break;
		}
		mSubmit = false;	
		
		GraphicsGL::gl_error("submitting texture");
	}

	GLenum mTarget;	// GL_TEXTURE_1D, GL_TEXTURE_2D, etc.
	GLenum mFormat;	
	GLenum mType;
	GLint mLevel;
	GLint mBorder;
	GLint mAlignment;
	
	Array mArray;
	
	bool mRebuild, mSubmit;
};

class CubeMapTexture : public GPUObject {
public:
	// same order as OpenGL
	enum Faces { 
		POSITIVE_X, NEGATIVE_X, 
		POSITIVE_Y, NEGATIVE_Y, 
		POSITIVE_Z, NEGATIVE_Z 
	};

	CubeMapTexture(int resolution=1024) 
	:	GPUObject(),
		mResolution(resolution),
		mTarget(GL_TEXTURE_CUBE_MAP)
	{
		// create mesh for drawing map:
		mMapMesh.primitive(Graphics::QUADS);
		mMapMesh.color(1,1,1,1);
		int mapSteps =100;
		double step = 1./mapSteps;
		for (double x=0; x<=1.; x+=step) {
			for (double y=0; y<=1.; y+=step) {
				drawMapVertex(x,		y);
				drawMapVertex(x,		y+step);
				drawMapVertex(x+step,	y+step);
				drawMapVertex(x+step,	y);
			}
		}
	}
	
	virtual ~CubeMapTexture() {}
	
	virtual void onCreate() {
		if (mID == 0) {
		
			// create cubemap texture:
			glGenTextures(1, (GLuint *)&mID);
			glBindTexture(mTarget, mID);
			// each cube face should clamp at texture edges:
			glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(mTarget, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			// normal filtering
			glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			// no mipmapping:
			//glTexParameteri(mTarget, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
			//glTexParameterf(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			
			// Domagoj also has:
			glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
			glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
			glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
			float X[4] = { 1,0,0,0 };
			float Y[4] = { 0,1,0,0 };
			float Z[4] = { 0,0,1,0 };
			glTexGenfv( GL_S, GL_OBJECT_PLANE, X );
			glTexGenfv( GL_T, GL_OBJECT_PLANE, Y );
			glTexGenfv( GL_R, GL_OBJECT_PLANE, Z );
		
			submit();
			
			// clean up:
			glBindTexture(mTarget, 0);
			GraphicsGL::gl_error("creating cubemap texture");
		}
		//printf("created CubeMapTexture %dx%d\n", mResolution, mResolution);
	}
	
	virtual void onDestroy() {
		if (mID) {
			glDeleteTextures(1, (GLuint *)&mID);
			mID=0;
		}
	}
	
	void bind(int unit = 0) {
		// ensure it is created:
		validate();
		
		// multitexturing:
		glActiveTextureARB( GL_TEXTURE0_ARB+unit );
		
		glEnable(mTarget);
		glBindTexture(mTarget, id());
	}
	
	void unbind(int unit = 0) {
		// multitexturing:
		glActiveTextureARB( GL_TEXTURE0_ARB+unit );
		
		glBindTexture(mTarget, 0);
		glDisable(mTarget);
	}	
	
	unsigned resolution() const { return mResolution; }
	
	// useful for debugging
	// draws full cubemap in a cylindrical projection
	void drawMap(Graphics& gl, double x0=0., double y0=0., double x1=1., double y1=1.) {
		bind();
		gl.color(1, 1, 1, 1);
		gl.pushMatrix();
		gl.translate(x0, y0, 0);
		gl.scale((x1-x0), (y1-y0), 1.);
		gl.draw(mMapMesh);
		gl.popMatrix();
		unbind();
	}

protected:
	inline void drawMapVertex(double x, double y) {
		// x runs 0..1, convert to angle -PI..PI:
		double az = M_PI * (x*2.-1.);
		// y runs 0..1, convert to angle -PI_2..PI_2:
		double el = M_PI * 0.5 * (y*2.-1.);
		// convert polar to normal:
		double x1 = sin(az);
		double y1 = sin(el);
		double z1 = cos(az);
		Vec3d v(x1, y1, z1);
		v.normalize();
		mMapMesh.texCoord	( v );
		mMapMesh.vertex	( x, y, 0);
	}
	
	void submit() {
		// RGBA8 Cubemap texture, 24 bit depth texture, mResolution x mResolution
		// NULL means reserve texture memory, but texels are undefined
		for (int i=0; i<6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA8, mResolution, mResolution, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		}
	}
	
	unsigned mResolution;
	Mesh mMapMesh;
	
	GLenum mTarget;
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
				glLightfv(id, GL_SPOT_DIRECTION, spotDirection.elems);
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