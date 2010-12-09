#ifndef INCLUDE_AL_GRAPHICS_GPUOBJECT_H
#define INCLUDE_AL_GRAPHICS_GPUOBJECT_H

#include <string>
#include <list>

namespace al{

class GPUContext {
public:	
	GPUContext();
	virtual ~GPUContext() {};
	
	// triggers destroy handler for each GPUObject registered in a given context
	void contextDestroy();
	
	int contextID() { return mContextID; }
	
protected:
	int mContextID;
};

/// Base class for allocated resources on the GPU
class GPUObject{
public:
	
	GPUObject(int ctx=0): mID(0) { contextRegister(ctx); }
	GPUObject(GPUContext& ctx): mID(0) { contextRegister(ctx.contextID()); }

	virtual ~GPUObject(){ contextUnregister(); }
	
	/// ensure that the GPUObject is ready to use
	/// typically placed before any rendering implementation
	void validate() {
		if (!created()) create();
	}
	
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
	
	/// Returns the assigned object id
	const uint32_t id() const { return mID; }
	void id(uint32_t v) {mID = v;}

protected:
	// register with a context (note: should only register with one context!)
	void contextRegister(int ctx=0);

	// remove from the context:
	void contextUnregister();
	
	uint32_t mID;
	virtual void onCreate() = 0;
	virtual void onDestroy() = 0;
};

} // ::al

#endif
