#ifndef AL_SLAB_H
#define AL_SLAB_H 1

/*	Allocore --
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
	An image-processing shader

	File author(s):
	Wesley Smith, 2010, wesley.hoke@gmail.com
*/
/*
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/graphics/al_Texture.hpp"
#include "allocore/graphics/al_GPUObject.hpp"

#define SLAB_MAX_RENDER_TARGETS	8

using al::Shader;

namespace al{

class Slab : public GPUObject {
public:
	Slab(Graphics *backend);
	virtual ~Slab();
	
	void draw(int argc, Texture **argv);
	void draw(Texture *tex);
	void draw(Texture *tex1, Texture *tex2);
	void draw(Slab *slab);
	void draw(Slab *slab1, Slab *slab2);
	

	Texture * get_texture(int i=0) {return mTex[i];}
	Surface * get_surface(int i=0) {return mSurface[i];}
	
	void get_dim(int width, int height);

	int get_width() {return mWidth;}
	int get_height() {return mHeight;}
	
	Shader * get_shader() {return mShader;}

protected:
	void onDestroy();
	void onCreate() {
		// dummy id for proxy destruction of internal objects
		id(1);
	}

protected:
	Graphics *				mBackend;				///< The backend
	Shader *				mShader;				///< The shader	
	int						mNumActiveTextures;		///< number of active render target textures
	Texture					*mTex[SLAB_MAX_RENDER_TARGETS];		///< Output texture
	Surface					*mSurface[SLAB_MAX_RENDER_TARGETS];	///< Output surface
	int						mWidth;					///< Output width of Slab
	int						mHeight;				///< Output height of Slab
};

} // al::
*/
#endif // AL_SLAB_H
