#ifndef INC_AL_UTIL_OPENGL_HPP
#define INC_AL_UTIL_OPENGL_HPP

#include "allocore/graphics/al_GraphicsOpenGL.hpp"

/*!
	A collection of utilities that are (currently) specific to OpenGL
*/

namespace al {

/*!
	A lot of OpenGL features follow a common pattern:
		gen/delete (usually tied to window/context creation/deletion)
		bind/unbind (around any process that utilizes the resource)
*/



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
		mAlignment(4)
	{
	}
	
	~TextureGL() {}

	void onCreate() {
		glGenTextures(1, &mID);
		
		printf("texture create %i\n", id());
		
		glBindTexture(mTarget, id());
		
		// todo: which options?
		glTexParameterf(mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(mTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(mTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(mTarget, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
		
		submit();
		
		unbind();
	}
	
	void onDestroy() {
		if (mID) {
			printf("texture destroy %i\n", id());
			glDeleteTextures(1, &mID);
			mID = 0;
		}
	}

	void bind(int unit = 0) {
		// rebuild?
		if (mRebuild) {
			printf("texture rebuild %i\n", id());
			onDestroy();
			onCreate();
			mRebuild = false;
			mUpdate = false;	// submitted in onCreate()
		}
		
		// multitexturing
		//glActiveTextureARB( GL_TEXTURE0_ARB+unit );
		glEnable(mTarget);
		glBindTexture(mTarget, id());
		
		//printf("texture bound %i\n", id());
		
		if(mUpdate) submit();
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
		mUpdate = true;
	}
	
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
			case AlloFloat64Ty:		return GL_DOUBLE;
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
		printf("texture submit %i %ix%i\n", id(), width(), height());
		GLvoid * ptr = (GLvoid *)data();
		glPixelStorei(GL_UNPACK_ALIGNMENT, mAlignment);
		switch(mTarget) {
			case GL_TEXTURE_1D:
				glTexSubImage1D(
					mTarget, 
					mLevel, 
					0, 
					width(), 
					mFormat, 
					mTarget, 
					ptr
				);
				break;
			case GL_TEXTURE_2D:
			case GL_TEXTURE_RECTANGLE_ARB:
				glTexSubImage2D(
					mTarget, 
					mLevel, 
					0, 0, 
					width(), height(), 
					mFormat,
					mType,
					ptr
				);
				break;
			case GL_TEXTURE_3D:
				glTexSubImage3D(
					mTarget, 
					mLevel, 
					0, 0, 0, 
					width(), height(), depth(), 
					mFormat, 
					mType, 
					ptr
				);
				break;

			default:
				printf("target not yet handled\n");
				break;
		}
		
	}

	GLuint mID;
	GLenum mTarget;	// GL_TEXTURE_1D, GL_TEXTURE_2D, etc.
	GLenum mFormat;	
	GLenum mType;
	GLint mLevel;
	GLint mAlignment;
	
	Array mArray;
	
	bool mRebuild, mUpdate;
};


} // al::

#endif