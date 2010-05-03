#ifndef INCLUDE_AL_GRAPHICS_TEXTURE_HPP
#define INCLUDE_AL_GRAPHICS_TEXTURE_HPP

#include "graphics/al_Common.hpp"
#include "graphics/al_GPUObject.hpp"

namespace al {
namespace gfx{

/// Base texture class

/// 
///
class TextureBase : public GPUObject{
public:
	TextureBase(Format::t format, DataType::t type, WrapMode::t wrap);

	virtual ~TextureBase();

	void * buffer() const { return mBuffer; }
	
	template <class T>
	T * buffer() const { return (T *)mBuffer; }

	const TextureBase& begin() const;						///< Bind self to current context
	const TextureBase& bind() const;						///< Bind self to current context (alias of begin())
	void end() const;										///< Binds default texture
	TextureBase& format(Format::t v);						///< Set the color format
	TextureBase& ipolMode(IpolMode::t v);					///< Set interpolation mode
	TextureBase& dataType(DataType::t v);					///< Set the color data type
	TextureBase& wrapMode(WrapMode::t v);					///< Set wrapping mode
	
	const TextureBase& send() const;						///< Send pointed to pixels to GPU

	Format::t format() const { return mFormat; }
	IpolMode::t ipolMode() const { return mIpol; }
	DataType::t dataType() const { return mType; }
	WrapMode::t wrapMode() const { return mWrap; }

protected:
	void * mPixels;		// pointer to the client-side pixel data (0 if none)
	void * mBuffer;		// internally allocated pixel buffer
	Format::t mFormat;	// format of the pixel data:
						//   GL_COLOR_INDEX, GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA, 
						//   GL_RGB, GL_BGR, GL_RGBA, GL_BGRA, GL_LUMINANCE, and GL_LUMINANCE_ALPHA
	IpolMode::t mIpol;	// interpolation mode
	WrapMode::t mWrap;
	DataType::t mType;			// type of the pixel data:
						//   GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT
						
	void freeMem();
	void allocMem();
	virtual int size() const = 0;	// total number of texels
	virtual int target() const = 0;
	virtual void texImage() const = 0;
	virtual void texSubImage() const = 0;
	virtual void texWrap() const = 0;
	
	virtual void onCreate();
	virtual void onDestroy();
};


/// 2-D texture
class Texture2 : public TextureBase{
public:

	/// This constructor will allocate an internal pixel buffer
	Texture2(	int width, int height, 
				Format::t format=gfx::RGB, 
				DataType::t type=gfx::UByte,
				WrapMode::t wrap=gfx::Repeat);

	int width() const { return w; }		///< Get width
	int height() const { return h; }	///< Get height

	Texture2& draw(											///< Draw texture to rectangular quad
		float ql, float qt, float qr, float qb,
		float tl=0, float tt=1, float tr=1, float tb=0
	);

	Texture2& load(int w, int h, void * pixels=0);	///< Resizes texture on graphics card

	int index1D(int i, int j){ return j*width()+i; }

private:
	int w, h;
	virtual int size() const { return w*h; }
	virtual int target() const;
	virtual void texImage() const;
	virtual void texSubImage() const;
	virtual void texWrap() const;
};



/// 3-D texture
class Texture3 : public TextureBase{
public:

	/// This constructor will allocate an internal pixel buffer
	Texture3(	int width, int height, int depth,
				Format::t format=gfx::RGB, 
				DataType::t type=gfx::UByte,
				WrapMode::t wrap=gfx::Repeat);

	int width() const { return w; }		///< Get width
	int height() const { return h; }	///< Get height
	int depth() const { return d; }		///< Get depth

	Texture3& load(int w, int h, int d, void * pixels=0);	///< Resizes texture on graphics card

	int index1D(int i, int j, int k){ return width()*(k*height() + j) + i; }

private:
	int w, h, d;
	virtual int size() const { return w*h*d; }
	virtual int target() const;
	virtual void texImage() const;
	virtual void texSubImage() const;
	virtual void texWrap() const;
};

} // ::al::gfx
} // ::al

#endif
