#ifndef INCLUDE_AL_GRAPHICS_GPUOBJECT_H
#define INCLUDE_AL_GRAPHICS_GPUOBJECT_H

#include <list>

namespace al{

/// Base class for allocated resources on the GPU
class GPUObject{
public:
	
	// triggers destroy handler for each GPUObject registered in a given context
	static void contextRefresh(std::string ctx);


	GPUObject(): mID(0){}
	virtual ~GPUObject(){ contextUnregister(); }
	
	// register with a context (note: should only register with one context!)
	void contextRegister(std::string ctx);
	
	// remove from the context:
	void contextUnregister();

	/// Returns the assigned object id
	const uint32_t id() const { return mID; }
	void id(uint32_t v) {mID = v;}
	
	bool created(){ return id()!=0; }

	/// Creates object on GPU
	void create(){
		if(created()){ destroy(); }
		onCreate();
	}
	
	/// Destroys object on GPU
	void destroy(){ 
		if(created()) {
			onDestroy(); mID=0; 
		}
	}
	
	/// ensure that the GPUObject is ready to use
	/// typically placed before any rendering implementation
	void validate() {
		if (!created()) create();
	}

protected:
	uint32_t mID;
	virtual void onCreate() = 0;
	virtual void onDestroy() = 0;
};

} // ::al

#endif
