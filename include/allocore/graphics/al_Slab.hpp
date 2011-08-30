#ifndef AL_SLAB_H
#define AL_SLAB_H 1

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
	An image-processing shader

	File author(s):
	Wesley Smith, 2010, wesley.hoke@gmail.com
*/

#include "graphics/al_Shader.hpp"
#include "graphics/al_Texture.hpp"
#include "graphics/al_GPUObject.hpp"

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

#endif // AL_SLAB_H
