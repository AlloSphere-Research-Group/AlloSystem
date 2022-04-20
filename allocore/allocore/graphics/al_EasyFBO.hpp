#ifndef __EASYFBO_HPP__
#define __EASYFBO_HPP__

/*  Allocore --
  Multimedia / virtual environment application class library

  Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
  Copyright (C) 2012. The Regents of the University of California.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    Neither the name of the University of California nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

  File description:
  Collates lower-level components into more user-friendly FBO

  File author(s):
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
	EasyFBO(int w, int h){ resize(w,h); }

	EasyFBO& resize(int w, int h){
		mTexture.format(Graphics::RGB);
		mTexture.type(Graphics::UBYTE);
		mTexture.resize(w,h);
		mNeedsSync = true;
		return *this;
	}

	int width() const { return mTexture.width(); }
	int height() const { return mTexture.height(); }

	const Matrix4d& modelView() const { return mMV; }
	Matrix4d& modelView(){ return mMV; }

	const Matrix4d& projection() const { return mProj; }
	Matrix4d& projection(){ return mProj; }

	EasyFBO& clearColor(const Color& c){ mClearColor=c; return *this; }

	const Texture& texture() const { return mTexture; }
	Texture& texture(){ return mTexture; }

	template <class DrawFunc>
	void draw(Graphics& g, const DrawFunc& drawFunc){

		sync();

		g.pushMatrix(Graphics::PROJECTION);
		g.pushMatrix(Graphics::MODELVIEW);
		auto oldVP = g.viewport();

		mFBO.begin();
			g.viewport(0, 0, width(), height());
			g.clearColor(mClearColor);
			g.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
			g.projection(mProj);
			g.modelView(mMV);
			drawFunc();
		mFBO.end();

		g.popMatrix(Graphics::PROJECTION);
		g.popMatrix(Graphics::MODELVIEW);
		g.viewport(oldVP);
	}

private:

	FBO mFBO;
	Texture mTexture; // for color buffer (read-write)
	RBO mRBO; // for depth buffer (write-only)
	Matrix4d mMV{1};
	Matrix4d mProj = Matrix4d::ortho(-2, 2, -2, 2, 0.01, 100);
	Color mClearColor = Color(0,0,0,1);
	bool mNeedsSync = false;

	void sync(){
		if(mNeedsSync){
			mNeedsSync = false;
			// both depth and color attachees must be valid on the GPU before use:
			mTexture.validate();
			mRBO.resize(width(), height());
			mFBO.attachRBO(mRBO, FBO::DEPTH_ATTACHMENT);
			mFBO.attachTexture2D(mTexture, FBO::COLOR_ATTACHMENT0);
			//printf("fbo status %s\n", mFBO.statusString());
		}
	}
};

} // al::

#endif
