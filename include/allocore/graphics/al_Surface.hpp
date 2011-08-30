#ifndef INCLUDE_AL_GRAPHICS_SURFACE_HPP
#define INCLUDE_AL_GRAPHICS_SURFACE_HPP

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
	A GPU object into which rendering can occur

	File author(s):
	Wesley Smith, 2010, wesley.hoke@gmail.com
*/

#include "allocore/graphics/al_Texture.hpp"
#include "allocore/graphics/al_GPUObject.hpp"
#include "allocore/types/al_Array.hpp"
#include "allocore/types/al_Color.hpp"

namespace al{

class Graphics;

///
class Surface : public GPUObject {
public:

	Surface(Graphics *backend, void *surface_data);
	virtual ~Surface();
	
	void * surfaceData();
	
	void copy(Texture * tex);
	
	void attach(Texture *tex);
	Texture * texture();
	
	void enter();
	void leave();
	
	void clearColor(Color &c);
	Color& clearColor();
	
	float clearDepth();
	void clearDepth(float v);
	
	bool autoClear();
	void autoClear(bool v);
	
	int width();
	int height();
	
	void clear();
	
	bool creating() {return mCreating;}
	
protected:

	virtual void onCreate();
	virtual void onDestroy();
	
	
	Graphics *		mBackend;
	void *			mSurfaceData;
	Texture *		mTexture;
	Color			mClearColor;
	float			mClearDepth;
	bool			mAutoclear;
	bool			mCreating;
};

} // ::al

#endif
