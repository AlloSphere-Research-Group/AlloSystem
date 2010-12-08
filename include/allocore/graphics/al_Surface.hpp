#ifndef INCLUDE_AL_GRAPHICS_SURFACE_HPP
#define INCLUDE_AL_GRAPHICS_SURFACE_HPP

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
