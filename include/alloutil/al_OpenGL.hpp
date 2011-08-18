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
	A simple wrapper around OpenGL Textures.
*/
class TextureGL {
public:
	TextureGL()
	:	mID(0),
		mTarget(GL_TEXTURE_2D),
		mFormat(GL_RGBA),
		mType(GL_UNSIGNED_BYTE),
		mLevel(0),
		mBorder(0),
		mAlignment(4),
		mRebuild(true),
		mSubmit(true)
	{
	}
	
	~TextureGL() {}

	void onCreate() {
		glGenTextures(1, &mID);
		glBindTexture(mTarget, id());
		
		// todo: which options?
		glTexParameterf(mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(mTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(mTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(mTarget, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
		submit();
		glBindTexture(mTarget, 0);
		
		mRebuild = false;
		
		GraphicsGL::gl_error("creating texture");
	}
	
	void onDestroy() {
		if (mID) {
			glDeleteTextures(1, &mID);
			mID = 0;
		}
	}

	void bind(int unit = 0) {
		if (mRebuild) {
			onDestroy();
		}
		
		// ensure it is created:
		if (id() == 0) onCreate();
		
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

	GLuint mID;
	GLenum mTarget;	// GL_TEXTURE_1D, GL_TEXTURE_2D, etc.
	GLenum mFormat;	
	GLenum mType;
	GLint mLevel;
	GLint mBorder;
	GLint mAlignment;
	
	Array mArray;
	
	bool mRebuild, mSubmit;
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