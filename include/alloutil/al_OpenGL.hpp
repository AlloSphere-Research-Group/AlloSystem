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
		if(sz <= 0) return;	// ignore empty arrays
		
		// format change?
		if (!mArray.isFormat(src.header)) {
			configureFromArray(src);
			// trigger re-create:
			mRebuild = true;
		}
		
		// copy data & change format:
		mArray = src;
		
//		// double-check:
//		for (int x=0; x<width(); x++) {
//		for (int y=0; y<height(); y++) {
//			uint8_t * cell = mArray.cell<uint8_t>(x, y);
//			printf("%i %i %u %u %u\n", x, y, cell[0], cell[1], cell[2]);
//		}}
		
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
		
		//printf("texture submit id %i target %X level %i components %i dim %ix%i format %X type %X ptr %p alignment %i\n", id(), mTarget, mLevel, mArray.header.components, width(), height(), mFormat, mType, ptr, mAlignment);
//		printf("glTexImage2D target %X level %i components %i dim %ix%i format %X type %X ptr %p alignment %i\n", GL_TEXTURE_2D, 0, 3, 600, 600, GL_RGB, GL_UNSIGNED_BYTE, ptr, 4);

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


} // al::

#endif