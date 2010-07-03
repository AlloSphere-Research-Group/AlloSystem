#ifndef INCLUDE_AL_GRAPHICS_GPUOBJECT_H
#define INCLUDE_AL_GRAPHICS_GPUOBJECT_H

#include <list>

namespace al {
namespace gfx{

class GPUObject{
public:
	GPUObject(): mID(0){}

	virtual ~GPUObject(){}

	/// Returns the assigned object id
	const uint32_t id() const { return mID; }
	
	bool created(){ return id()!=0; }

	/// Creates object on GPU
	void create(){
		if(created()){ destroy(); }
		onCreate();
	}
	
	/// Destroys object on GPU
	void destroy(){ onDestroy(); mID=0; }

protected:
	uint32_t mID;
	virtual void onCreate() = 0;
	virtual void onDestroy() = 0;
};

/*
	Utility object
*/
class GPUObjectList {
	
	void add(GPUObject & gpuObject) { mObjects.push_back(&gpuObject); }
	void remove(GPUObject & gpuObject) { mObjects.remove(&gpuObject); }
	
	void create() {
		for (std::list<GPUObject *>::iterator it = mObjects.begin(); it != mObjects.end(); it++) {
			(*it)->create();
		}
	}
	
	void destroy() {
		for (std::list<GPUObject *>::iterator it = mObjects.begin(); it != mObjects.end(); it++) {
			(*it)->destroy();
		}
	}
	
protected:
	std::list<GPUObject *> mObjects;
};

} // ::al::gfx
} // ::al

#endif
