#ifndef INC_AL_EASY_FBO_HPP
#define INC_AL_EASY_FBO_HPP

/*  Allocore --  Multimedia / virtual environment application class library

	Description:
	Collates lower-level components into more user-friendly FBO

	Author(s):
	Tim Wood, 2015, fishuyo@gmail.com
	Lance Putnam, 2022
*/

#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_FBO.hpp"
#include "allocore/graphics/al_Texture.hpp"
#include "allocore/spatial/al_Pose.hpp"

namespace al {

/// Encapsulates FBO, depth buffer, and texture
/// @ingroup allocore
///
struct EasyFBO {

	EasyFBO(){}

	/// \param[in] w		Width of buffer
	/// \param[in] h		Height of buffer
	/// \param[in] format	Texel format of color buffer
	/// \param[in] type		Texel data type of color buffer
	EasyFBO(
		unsigned w, unsigned h,
		Graphics::Format format = Graphics::RGBA,
		Graphics::DataType type = Graphics::UBYTE
	):	EasyFBO()
	{
		resize(w,h, format,type);
	}

	EasyFBO(
		unsigned wh,
		Graphics::Format format = Graphics::RGBA,
		Graphics::DataType type = Graphics::UBYTE	
	)
	:	EasyFBO(wh,wh, format,type)
	{}


	/// Get height
	unsigned width() const { return mTexture.width(); }
	/// Get width
	unsigned height() const { return mTexture.height(); }

	/// Get number of color components
	unsigned numComponents() const { return mTexture.numComponents(); }
	/// Get color format
	Graphics::Format format() const { return mTexture.format(); }
	/// Get color type
	Graphics::DataType type() const { return mTexture.type(); }

	/// Resize and configure color buffer
	EasyFBO& resize(
		unsigned w, unsigned h,
		Graphics::Format format,
		Graphics::DataType type = Graphics::UBYTE
	){
		mTexture.format(format).type(type);
		return resize(w,h);
	}

	/// Resize
	EasyFBO& resize(unsigned w, unsigned h){
		mTexture.resize(w,h);
		mNeedsSync = true;
		return *this;
	}

	/// Set multisample count (or 0 to disable multisampling)
	EasyFBO& samples(unsigned n){
		if(samples() != n){
			mColorRBO_MS.samples(n);
			mDepthRBO_MS.samples(n);
			mNeedsSync = true;
		}
		return *this;
	}

	/// Get multisample count
	unsigned samples() const { return mColorRBO_MS.samples(); }

	/// Get modelview matrix
	const Matrix4d& modelView() const { return mMV; }
	Matrix4d& modelView(){ return mMV; }
	/// Set modelview matrix
	EasyFBO& modelView(const Matrix4d& m){ mMV=m; return *this; }

	/// Get projection matrix
	const Matrix4d& projection() const { return mProj; }
	Matrix4d& projection(){ return mProj; }
	/// Set projection matrix
	EasyFBO& projection(const Matrix4d& m){ mProj=m; return *this; }

	/// Set clear color
	EasyFBO& clearColor(const Color& c){ mClearColor=c; return *this; }

	/// Set whether to clear buffer when calling draw()
	EasyFBO& clear(bool v){
		mDoClear = v;
		return *this;
	}

	/// Set whether to use depth buffer
	EasyFBO& depth(bool v){
		if(v != mUseDepth){
			mUseDepth = v;
			mNeedsSync = true;
		}
		return *this;
	}

	/// Get color buffer texture
	const Texture& texture() const { return mTexture; }
	Texture& texture(){ return mTexture; }

	/// Get underlying FBO
	const FBO& fbo() const { return mFBO; }
	FBO& fbo(){ sync(); return mFBO; }

	/// Call draw commands writing into FBO
	template <class DrawFunc>
	void draw(Graphics& g, const DrawFunc& drawFunc){

		sync();

		g.pushMatrix(Graphics::PROJECTION);
		g.pushMatrix(Graphics::MODELVIEW);
		auto oldVP = g.viewport();

		auto drawScene = [&]{
			g.viewport(0, 0, width(), height());
			if(mDoClear){
				g.clearColor(mClearColor);
				auto clearBuffers = Graphics::COLOR_BUFFER_BIT;
				if(mUseDepth) clearBuffers = clearBuffers | Graphics::DEPTH_BUFFER_BIT;
				g.clear(clearBuffers);
			}
			g.projection(mProj);
			g.modelView(mMV);
			drawFunc();
		};

		if(0 == samples()){
			mFBO.scope([&]{ drawScene(); });
		} else {
			mFBO_MS.scope([&]{ drawScene(); });
			mFBO_MS.copyTo(mFBO, width(),height(), Graphics::COLOR_BUFFER_BIT);
		}

		g.popMatrix(Graphics::PROJECTION);
		g.popMatrix(Graphics::MODELVIEW);
		g.viewport(oldVP);
	}

private:

	FBO mFBO;
	Texture mTexture; // for color buffer (read-write)
	RBO mDepthRBO; // for depth buffer (write-only)
	FBO mFBO_MS;
	RBO mDepthRBO_MS;
	RBO mColorRBO_MS;
	Matrix4d mMV{1};
	Matrix4d mProj = Matrix4d::ortho(-1,1, -1,1, -1,1);
	Color mClearColor{0};
	bool mUseDepth = true;
	bool mDoClear = true;
	bool mNeedsSync = false;

	void sync(){
		if(mNeedsSync){
			mNeedsSync = false;

			// Clear all attachments first
			// If we make new attachments and their size does not match existing
			// attachments, it can trigger an incomplete FBO.
			mFBO.detachTexture2D(FBO::COLOR_ATTACHMENT0);
			mFBO.detachRBO(FBO::DEPTH_ATTACHMENT);

			mTexture.submit();
			mFBO.attachTexture2D(mTexture, FBO::COLOR_ATTACHMENT0);

			if(0 == samples()){
				if(mUseDepth){
					mDepthRBO.resize(width(), height());
					mFBO.attachRBO(mDepthRBO, FBO::DEPTH_ATTACHMENT);
				}
			} else {
				mFBO_MS.detachRBO(FBO::COLOR_ATTACHMENT0);
				mFBO_MS.detachRBO(FBO::DEPTH_ATTACHMENT);

				mColorRBO_MS.format(mTexture.format()).resize(width(), height());
				mFBO_MS.attachRBO(mColorRBO_MS, FBO::COLOR_ATTACHMENT0);
				if(mUseDepth){
					mDepthRBO_MS.resize(width(), height());
					mFBO_MS.attachRBO(mDepthRBO_MS, FBO::DEPTH_ATTACHMENT);
				}
			}
			//printf("fbo status %s\n", mFBO.statusString());
			AL_GRAPHICS_ERROR("EasyFBO::sync", -1);
		}
	}
};

} // al::
#endif
