#ifndef INCLUDE_AL_GRAPHICS_SURFACE_HPP
#define INCLUDE_AL_GRAPHICS_SURFACE_HPP

#include "allocore/graphics/al_Texture.hpp"
#include "allocore/graphics/al_Common.hpp"
#include "allocore/graphics/al_GPUObject.hpp"
#include "allocore/types/al_types.hpp"
#include "allocore/types/al_Color.hpp"

namespace al {
namespace gfx{

class GraphicsBackend;

///
class Surface : public GPUObject {
public:

	Surface(GraphicsBackend *backend, void *surface_data);
	virtual ~Surface();
	
	void * surfaceData();
	
	void attach(Texture *tex);
	Texture * texture();
	
	void enter();
	void leave();
	
	Color& clearColor();
	
	float clearDepth();
	void clearDepth(float v);
	
	bool autoClear();
	void autoClear(bool v);
	
	int width();
	int height();
	
protected:

	virtual void onCreate();
	virtual void onDestroy();
	
	
	GraphicsBackend	* mBackend;
	void *			mSurfaceData;
	Texture *		mTexture;
	Color			mClearColor;
	float			mClearDepth;
	bool			mAutoclear;
};

} // ::al::gfx
} // ::al

#endif
